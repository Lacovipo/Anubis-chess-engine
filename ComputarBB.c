/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de la función
	que computa una serie de bitboards últiles
	para evaluación, poda, generación de
	movimientos, etc.
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"
#include "Bitboards_inline.h"
#include "GPT_Magic.h"

/*
	*************************************************************
	*															*
	*	ComputarBB												*
	*															*
	*															*
	*	Recibe: Puntero a la posición actual					*
	*															*
	*	Descripción: Calcula información sobre ataques para		*
	*				 utilizar en diversas partes del programa,	*
	*				 especialmente en evaluación				*
	*															*
	*************************************************************
*/
void ComputarBB(TPosicion * pPos)
{
	UINT64 u64Clavadores;
	UINT64 u64Vacias = ~(pPos->u64TodasB | pPos->u64TodasN);
	UINT32 u32Desde;

	//
	// Peones
	//
	pPos->u64AtaquesPB = AtaquesPeonB(pPos->u64PeonesB);
	pPos->u64AtaquesPN = AtaquesPeonN(pPos->u64PeonesN);
	//
	// Caballos
	//
	if (pPos->u64CaballosB)
		pPos->u64AtaquesCB = AtaquesCaballo(pPos->u64CaballosB);
	else
		pPos->u64AtaquesCB = BB_TABLEROVACIO;
	if (pPos->u64CaballosN)
		pPos->u64AtaquesCN = AtaquesCaballo(pPos->u64CaballosN);
	else
		pPos->u64AtaquesCN = BB_TABLEROVACIO;
	//
	// Alfiles
	//
	if (pPos->u64AlfilesB)
		pPos->u64AtaquesAB = AtaquesAlfil(pPos->u64AlfilesB, u64Vacias);
	else
		pPos->u64AtaquesAB = BB_TABLEROVACIO;
	if (pPos->u64AlfilesN)
		pPos->u64AtaquesAN = AtaquesAlfil(pPos->u64AlfilesN,u64Vacias);
	else
		pPos->u64AtaquesAN = BB_TABLEROVACIO;
	//
	// Torres
	//
	if (pPos->u64TorresB)
		pPos->u64AtaquesTB = AtaquesTorre(pPos->u64TorresB,u64Vacias);
	else
		pPos->u64AtaquesTB = BB_TABLEROVACIO;
	if (pPos->u64TorresN)
		pPos->u64AtaquesTN = AtaquesTorre(pPos->u64TorresN,u64Vacias);
	else
		pPos->u64AtaquesTN = BB_TABLEROVACIO;
	//
	// Damas
	//
	if (pPos->u64DamasB)
		pPos->u64AtaquesDB = AtaquesAlfil(pPos->u64DamasB,u64Vacias) | AtaquesTorre(pPos->u64DamasB,u64Vacias);
	else
		pPos->u64AtaquesDB = BB_TABLEROVACIO;
	if (pPos->u64DamasN)
		pPos->u64AtaquesDN = AtaquesAlfil(pPos->u64DamasN,u64Vacias) | AtaquesTorre(pPos->u64DamasN,u64Vacias);
	else
		pPos->u64AtaquesDN = BB_TABLEROVACIO;
	//
	// Reyes
	//
	pPos->u64AtaquesRB = BB_ATAQUES_REY[pPos->u8PosReyB];
	pPos->u64AtaquesRN = BB_ATAQUES_REY[pPos->u8PosReyN];

	pPos->u64AtaquesB = pPos->u64AtaquesPB | pPos->u64AtaquesCB	| pPos->u64AtaquesAB | pPos->u64AtaquesTB | pPos->u64AtaquesDB | pPos->u64AtaquesRB;
	pPos->u64AtaquesN = pPos->u64AtaquesPN | pPos->u64AtaquesCN | pPos->u64AtaquesAN | pPos->u64AtaquesTN | pPos->u64AtaquesDN | pPos->u64AtaquesRN;

	//
	// 07/11/2003 - Clavadas
	//
	pPos->u64Clavadas = BB_TABLEROVACIO;
	u64Clavadores = BB_ATAQUES_ALFIL[pPos->u8PosReyB] & (pPos->u64AlfilesN | pPos->u64DamasN);
	u64Clavadores |= (au64AtaquesTorre[pPos->u8PosReyB] & (pPos->u64TorresN | pPos->u64DamasN));
	while (u64Clavadores)
	{
		u32Desde = BB_GetBitYQuitar(&u64Clavadores);
		// Si sólo hay una pieza entre el rey y el "clavador"
		if (CuentaUnos(au64Entre[u32Desde][pPos->u8PosReyB] & (pPos->u64TodasB | pPos->u64TodasN)) == 1)
		{
			// Si esa única pieza es blanca
			if (au64Entre[u32Desde][pPos->u8PosReyB] & pPos->u64TodasB)
				pPos->u64Clavadas |= (au64Entre[u32Desde][pPos->u8PosReyB] & pPos->u64TodasB);
		}
	}
	u64Clavadores = BB_ATAQUES_ALFIL[pPos->u8PosReyN] & (pPos->u64AlfilesB | pPos->u64DamasB);
	u64Clavadores |= (au64AtaquesTorre[pPos->u8PosReyN] & (pPos->u64TorresB | pPos->u64DamasB));
	while (u64Clavadores)
	{
		u32Desde = BB_GetBitYQuitar(&u64Clavadores);
		// Si sólo hay una pieza entre el rey y el "clavador"
		if (CuentaUnos(au64Entre[u32Desde][pPos->u8PosReyN] & (pPos->u64TodasB | pPos->u64TodasN)) == 1)
		{
			// Si esa única pieza es negra
			if (au64Entre[u32Desde][pPos->u8PosReyN] & pPos->u64TodasN)
				pPos->u64Clavadas |= (au64Entre[u32Desde][pPos->u8PosReyN] & pPos->u64TodasN);
		}
	}
}