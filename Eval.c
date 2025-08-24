/*
	Anubis

	José Carlos Martínez Galán
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"
#include "Bitboards_inline.h"
#include "Inline.h"
#include "Funciones.h"
#include "nnue.h"
#include <math.h>

/*
	*****************************************************
	*													*
	*	EvaluarMaterial									*
	*													*
	*													*
	*	Recibe: Puntero a la posición actual			*
	*													*
	*	Devuelve: El balance material					*
	*			  ¡OJO! Siempre es desde el punto de	*
	*				 vista de las blancas				*
	*													*
	*****************************************************
*/
SINT32 EvaluarMaterial(TPosicion* pPos)
{
	SINT32 s32Material;

	s32Material = pPos->u8NumPeonesB * VAL_PEON;
	s32Material -= pPos->u8NumPeonesN * VAL_PEON;
	s32Material += CuentaUnos(pPos->u64CaballosB) * VAL_CABALLO;
	s32Material -= CuentaUnos(pPos->u64CaballosN) * VAL_CABALLO;
	s32Material += CuentaUnos(pPos->u64AlfilesB) * VAL_ALFIL;
	s32Material -= CuentaUnos(pPos->u64AlfilesN) * VAL_ALFIL;
	s32Material += CuentaUnos(pPos->u64TorresB) * VAL_TORRE;
	s32Material -= CuentaUnos(pPos->u64TorresN) * VAL_TORRE;
	s32Material += CuentaUnos(pPos->u64DamasB) * VAL_DAMA;
	s32Material -= CuentaUnos(pPos->u64DamasN) * VAL_DAMA;

	return(s32Material);
}

/*
	*****************************************************
	*													*
	*	InsuficienteMaterial							*
	*													*
	*													*
	*	Recibe: Puntero a la posición actual			*
	*													*
	*	Devuelve: TRUE si deben ser tablas por			*
	*			  insuficiente material, FALSE en caso	*
	*			  contrario								*
	*													*
	*	Descripción: Comprueba los casos más básicos de	*
	*				 tablas por falta de material		*
	*													*
	*****************************************************
*/
BOOL InsuficienteMaterial(TPosicion* pPos)
{
	if (pPos->u8NumPeonesB == 0 && pPos->u8NumPeonesN == 0)
	{
		// Rey contra rey -> tablas
		if (pPos->u8NumPiezasB == 0 && pPos->u8NumPiezasN == 0)
			return(TRUE);

		// Si sólo queda una pieza menor es insuficiente material
		if (pPos->u8NumPiezasB == 0 && pPos->u8NumPiezasN == 1)
		{
			if (pPos->u64DamasN == BB_TABLEROVACIO && pPos->u64TorresN == BB_TABLEROVACIO)
				return(TRUE);
		}
		if (pPos->u8NumPiezasB == 1 && pPos->u8NumPiezasN == 0)
		{
			if (pPos->u64DamasB == BB_TABLEROVACIO && pPos->u64TorresB == BB_TABLEROVACIO)
				return(TRUE);
		}
	}

	return(FALSE);
}

/*
	*********************************************************************************
	*																				*
	*	DeterminarAmenaza															*
	*																				*
	*																				*
	*	Recibe: Puntero a la posición actual										*
	*																				*
	*	Devuelve: Un entero que representa una estimación numérica de la amenaza en	*
	*			  segundo grado, es decir, qué me amenaza mi adversario y qué tal	*
	*			  lo tengo defendido												*
	*																				*
	*	Descripción: Se hace un SEE para cada pieza o peón amenazado. Me quedo con	*
	*				 la amenaza más alta, pero si hay otras, añado una pequeña		*
	*				 cantidad														*
	*																				*
	*********************************************************************************
*/
SINT32 DeterminarAmenaza(TPosicion* pPos)
{
	UINT64	u64Temp;
	UINT32	u32Casilla;
	SINT32	s32Amenaza = 0;
	SINT32	s32AmenazaMax = 0;
	SINT32	s32Adicional = 0;

	if (Pos_GetJaqueado(pPos))
		return 0;

	if (Pos_GetTurno(pPos) == BLANCAS)
		u64Temp = (pPos->u64TodasB & ~BB_Mask(pPos->u8PosReyB)) & pPos->u64AtaquesN;
	else
		u64Temp = (pPos->u64TodasN & ~BB_Mask(pPos->u8PosReyN)) & pPos->u64AtaquesB;
	while (u64Temp)
	{
		u32Casilla = BB_GetBitYQuitar(&u64Temp);
		s32Amenaza = SEE_Amenaza(pPos, u32Casilla, VACIO);

		// Lo que viene es la nueva fórmula. Antes, simplemente, iba acumulando amenazas, sumándolas sin más
		if (s32Amenaza > 0)
		{
			// Tenemos, al menos, una amenaza
			if (s32AmenazaMax == 0)
			{
				// Estoy sumando la primera amenaza
				s32AmenazaMax = s32Amenaza;
			}
			else
			{
				// Ya tengo alguna amenaza anterior; me quedo con la mayor y añado una cantidad para reflejar que hay más amenazas
				SINT32 s32Temp = min(s32AmenazaMax, s32Amenaza);
				s32AmenazaMax = max(s32AmenazaMax, s32Amenaza);
				s32Adicional += (s32Temp / 10);
			}
		}
	}

	return(s32AmenazaMax + s32Adicional);
}

SINT32 Evaluar(TPosicion* pPos)
{
	SINT32 s32Eval;

	pPos->u32Fase = Pos_GetFasePartida(pPos);

	if (Pos_GetJaqueado(pPos) || pPos->u32MovExclu)
	{
		ComputarBB(pPos);
		pPos->s32EvalAmenaza = DeterminarAmenaza(pPos);
		return Pos_GetTurno(pPos) == BLANCAS ? -INFINITO : INFINITO;
	}

#if !defined(DEBUG_NNUE)
	if (ConsultarHashEval(pPos))
	{
		// Ajuste por cercanía al contador de tablas
		s32Eval = pPos->s32Eval;
		s32Eval *= (200 - pPos->u8Cincuenta);
		s32Eval /= 200;

		return(s32Eval);
	}
#endif

	ComputarBB(pPos);
	pPos->s32EvalAmenaza = DeterminarAmenaza(pPos);

	const BOOL bHayPeones = pPos->u8NumPeonesB || pPos->u8NumPeonesN;
	const BOOL bHayTorres = pPos->u64TorresB || pPos->u64TorresN;
	const BOOL bHayDamas = pPos->u64DamasB || pPos->u64DamasN;
	const BOOL bHayCaballos = pPos->u64CaballosB || pPos->u64CaballosN;
	const BOOL bHayAlfiles = pPos->u64AlfilesB || pPos->u64AlfilesN;

	if (bHayPeones && !bHayTorres && !bHayDamas && !bHayAlfiles && !bHayCaballos)
		s32Eval = nnue_evaluate(pPos, &nnue_weights_T1);		// T1 = peones
	else if (bHayTorres && !bHayDamas && !bHayAlfiles && !bHayCaballos)
		s32Eval = nnue_evaluate(pPos, &nnue_weights_T2);		// T2 = torres
	else if (!bHayTorres && bHayDamas && !bHayAlfiles && !bHayCaballos)
		s32Eval = nnue_evaluate(pPos, &nnue_weights_T3);		// T3 = damas
	else if (bHayTorres && bHayDamas && !bHayAlfiles && !bHayCaballos)
		s32Eval = nnue_evaluate(pPos, &nnue_weights_T4);		// T4 = mayores
	else if (!bHayTorres && !bHayDamas && !bHayAlfiles && bHayCaballos)
		s32Eval = nnue_evaluate(pPos, &nnue_weights_T5);		// T5 = caballos
	else if (!bHayTorres && !bHayDamas && bHayAlfiles && !bHayCaballos)
		s32Eval = nnue_evaluate(pPos, &nnue_weights_T6);		// T6 = alfiles
	else if (!bHayTorres && !bHayDamas && bHayAlfiles && bHayCaballos)
		s32Eval = nnue_evaluate(pPos, &nnue_weights_T7);		// T7 = menores
	else if (bHayDamas)
		if (pPos->u8NumPiezasB + pPos->u8NumPiezasN >= 10)
			s32Eval = nnue_evaluate(pPos, &nnue_weights_T0);		// T0 = resto con damas (más de 10 piezas)
		else
			s32Eval = nnue_evaluate(pPos, &nnue_weights_T8);		// T8 = resto con damas (menor o igual a 10 piezas)
	else
		s32Eval = nnue_evaluate(pPos, &nnue_weights_T9);		// T9 = resto sin damas

	if (Pos_GetTurno(pPos) == NEGRAS)
		s32Eval = -s32Eval;

	GrabarHashEval(pPos, s32Eval);

	s32Eval *= (200 - pPos->u8Cincuenta);
	s32Eval /= 200;
	return(s32Eval);
}
