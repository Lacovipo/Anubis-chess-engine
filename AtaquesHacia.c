/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de la función
	AtaquesHacia
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Constantes.h"
#include "Variables.h"
#include "Bitboards_inline.h"
#include "Inline.h"

/*
	*************************************************************
	*															*
	*	AtaquesHacia											*
	*															*
	*															*
	*	Recibe: Puntero a la posición actual y casilla cuyos	*
	*			ataques queremos determinar						*
	*															*
	*	Devuelve: Un bitmap conteniendo todas las casillas que	*
	*			  atacan la dada								*
	*															*
	*	Descripción: Comprueba todas las piezas que atacan la	*
	*				 casilla dada, y las graba en un bitmap		*
	*				 que acaba devolviendo						*
	*															*
	*************************************************************
*/
UINT64 AtaquesHacia(TPosicion * pPos, UINT32 u32Casilla)
{
	UINT64 u64Ataques = BB_TABLEROVACIO;
	UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	UINT64 u64Desde;
	UINT32 u32Origen;

	// Caballos
	u64Ataques |= ((pPos->u64CaballosB | pPos->u64CaballosN) & BB_ATAQUES_CABALLO[u32Casilla]);
	// Alfiles o damas
	u64Desde = (pPos->u64AlfilesB | pPos->u64DamasB | pPos->u64AlfilesN | pPos->u64DamasN) & BB_ATAQUES_ALFIL[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Torres o damas
	u64Desde = (pPos->u64TorresB | pPos->u64DamasB | pPos->u64TorresN | pPos->u64DamasN) & au64AtaquesTorre[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Peones blancos
	if (u32Casilla < 48)
	{
		if (pPos->u64PeonesB & ((BB_Mask(u32Casilla) & BB_SINCOLIZDA) >> 7))
			u64Ataques |= BB_Mask(u32Casilla + 7);
		if (pPos->u64PeonesB & ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) >> 9))
			u64Ataques |= BB_Mask(u32Casilla + 9);
	}
	// Peones negros
	if (u32Casilla > 15)
	{
		if (pPos->u64PeonesN & ((BB_Mask(u32Casilla) & BB_SINCOLIZDA) << 9))
			u64Ataques |= BB_Mask(u32Casilla - 9);
		if (pPos->u64PeonesN & ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) << 7))
			u64Ataques |= BB_Mask(u32Casilla - 7);
	}
	// Al paso (negro)
	if (u32Casilla == (UINT32)(pPos->u8AlPaso + 8))
	{
		if (pPos->u64PeonesB & ((BB_Mask(u32Casilla) & BB_SINCOLIZDA) << 1))
			u64Ataques |= BB_Mask(u32Casilla - 1);
		if (pPos->u64PeonesB & ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) >> 1))
			u64Ataques |= BB_Mask(u32Casilla + 1);
	}
	// Al paso (blanco)
	if (u32Casilla == (UINT32)(pPos->u8AlPaso - 8))
	{
		if (pPos->u64PeonesN & ((BB_Mask(u32Casilla) & BB_SINCOLIZDA) << 1))
			u64Ataques |= BB_Mask(u32Casilla - 1);
		if (pPos->u64PeonesN & ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) >> 1))
			u64Ataques |= BB_Mask(u32Casilla + 1);
	}
	// Rey
	u64Ataques |= ((BB_Mask(pPos->u8PosReyB) | BB_Mask(pPos->u8PosReyN))
					& BB_ATAQUES_REY[u32Casilla]);

	return(u64Ataques);
}

/*
	Ataques blancos a una zona negra
*/
UINT64 AtaquesB_ZonaN(TPosicion * pPos,UINT64 u64Zona)
{
	register UINT64 u64Origenes;

	u64Origenes = AtaquesPeonN(u64Zona) & pPos->u64PeonesB;
	u64Origenes |= AtaquesCaballo(u64Zona) & pPos->u64CaballosB;
	u64Origenes |= AtaquesAlfil(u64Zona,~(pPos->u64PeonesB | pPos->u64PeonesN | pPos->u64AlfilesN | pPos->u64CaballosN)) & pPos->u64AlfilesB;
	u64Origenes |= AtaquesAlfil(u64Zona,~(pPos->u64PeonesB | pPos->u64TodasN)) & pPos->u64DamasB;
	u64Origenes |= AtaquesTorre(u64Zona,~(pPos->u64PeonesB | pPos->u64TodasN)) & pPos->u64DamasB;
	u64Origenes |= AtaquesTorre(u64Zona,~(pPos->u64PeonesB | pPos->u64PeonesN | pPos->u64AlfilesN | pPos->u64CaballosN | pPos->u64TorresN)) & pPos->u64TorresB;

	return(u64Origenes);
}

/*
	Ataques negros a una zona blanca
*/
UINT64 AtaquesN_ZonaB(TPosicion * pPos,UINT64 u64Zona)
{
	register UINT64 u64Origenes;

	u64Origenes = AtaquesPeonB(u64Zona) & pPos->u64PeonesN;
	u64Origenes |= AtaquesCaballo(u64Zona) & pPos->u64CaballosN;
	u64Origenes |= AtaquesAlfil(u64Zona,~(pPos->u64PeonesN | pPos->u64PeonesB | pPos->u64AlfilesB | pPos->u64CaballosB)) & pPos->u64AlfilesN;
	u64Origenes |= AtaquesAlfil(u64Zona,~(pPos->u64PeonesN | pPos->u64TodasB)) & pPos->u64DamasN;
	u64Origenes |= AtaquesTorre(u64Zona,~(pPos->u64PeonesN | pPos->u64TodasB)) & pPos->u64DamasN;
	u64Origenes |= AtaquesTorre(u64Zona,~(pPos->u64PeonesN | pPos->u64PeonesB | pPos->u64AlfilesB | pPos->u64CaballosB | pPos->u64TorresB)) & pPos->u64TorresN;

	return(u64Origenes);
}

/*
	Ataques blancos a una casilla
*/
UINT64 AtaquesB_Hacia(TPosicion * pPos, UINT32 u32Casilla)
{
	UINT64 u64Ataques = BB_TABLEROVACIO;
	UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	UINT64 u64Desde;
	UINT32 u32Origen;

	// Caballos
	u64Ataques |= (pPos->u64CaballosB & BB_ATAQUES_CABALLO[u32Casilla]);
	// Alfiles o damas
	u64Desde = (pPos->u64AlfilesB | pPos->u64DamasB) & BB_ATAQUES_ALFIL[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Torres o damas
	u64Desde = (pPos->u64TorresB | pPos->u64DamasB)	& au64AtaquesTorre[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Peones blancos
	if (u32Casilla < 48)
	{
		if (pPos->u64PeonesB & ((BB_Mask(u32Casilla) & BB_SINCOLIZDA) >> 7))
			u64Ataques |= BB_Mask(u32Casilla + 7);
		if (pPos->u64PeonesB & ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) >> 9))
			u64Ataques |= BB_Mask(u32Casilla + 9);
	}
	// Al paso (negro)
	if (u32Casilla == (UINT32)(pPos->u8AlPaso + 8))
	{
		if (pPos->u64PeonesB & ((BB_Mask(u32Casilla) & BB_SINCOLIZDA) << 1))
			u64Ataques |= BB_Mask(u32Casilla - 1);
		if (pPos->u64PeonesB & ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) >> 1))
			u64Ataques |= BB_Mask(u32Casilla + 1);
	}
	// Rey
	u64Ataques |= (BB_Mask(pPos->u8PosReyB) & BB_ATAQUES_REY[u32Casilla]);

	return(u64Ataques);
}

/*
	Ataques negros a una casilla
*/
UINT64 AtaquesN_Hacia(TPosicion * pPos, UINT32 u32Casilla)
{
	UINT64 u64Ataques = BB_TABLEROVACIO;
	UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	UINT64 u64Desde;
	UINT32 u32Origen;

	// Caballos
	u64Ataques |= (pPos->u64CaballosN & BB_ATAQUES_CABALLO[u32Casilla]);
	// Alfiles o damas
	u64Desde = (pPos->u64AlfilesN | pPos->u64DamasN) & BB_ATAQUES_ALFIL[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Torres o damas
	u64Desde = (pPos->u64TorresN | pPos->u64DamasN)	& au64AtaquesTorre[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Peones negros
	if (u32Casilla > 15)
	{
		if (pPos->u64PeonesN & ((BB_Mask(u32Casilla) & BB_SINCOLIZDA) << 9))
			u64Ataques |= BB_Mask(u32Casilla - 9);
		if (pPos->u64PeonesN & ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) << 7))
			u64Ataques |= BB_Mask(u32Casilla - 7);
	}
	// Al paso (blanco)
	if (u32Casilla == (UINT32)(pPos->u8AlPaso - 8))
	{
		if (pPos->u64PeonesN & ((BB_Mask(u32Casilla) & BB_SINCOLIZDA) << 1))
			u64Ataques |= BB_Mask(u32Casilla - 1);
		if (pPos->u64PeonesN & ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) >> 1))
			u64Ataques |= BB_Mask(u32Casilla + 1);
	}
	// Rey
	u64Ataques |= (BB_Mask(pPos->u8PosReyN) & BB_ATAQUES_REY[u32Casilla]);

	return(u64Ataques);
}

/*
	Ataques de piezas deslizantes blancas a una casilla
*/
UINT64 AtaquesDeslizB_Hacia(TPosicion * pPos, UINT32 u32Casilla)
{
	UINT64 u64Ataques = BB_TABLEROVACIO;
	UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	UINT64 u64Desde;
	UINT32 u32Origen;

	// Alfiles o damas
	u64Desde = (pPos->u64AlfilesB | pPos->u64DamasB) & BB_ATAQUES_ALFIL[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Torres o damas
	u64Desde = (pPos->u64TorresB | pPos->u64DamasB)	& au64AtaquesTorre[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	return(u64Ataques);
}

/*
	Ataques de piezas deslizantes negras a una casilla
*/
UINT64 AtaquesDeslizN_Hacia(TPosicion * pPos, UINT32 u32Casilla)
{
	UINT64 u64Ataques = BB_TABLEROVACIO;
	UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	UINT64 u64Desde;
	UINT32 u32Origen;

	// Alfiles o damas
	u64Desde = (pPos->u64AlfilesN | pPos->u64DamasN) & BB_ATAQUES_ALFIL[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Torres o damas
	u64Desde = (pPos->u64TorresN | pPos->u64DamasN)	& au64AtaquesTorre[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	return(u64Ataques);
}

/*
	Movimientos blancos a una casilla. Es != de ataques por los peones
*/
UINT64 MovimientosB_Hacia(TPosicion * pPos, UINT32 u32Casilla)
{
	UINT64 u64Ataques = BB_TABLEROVACIO;
	UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	UINT64 u64Desde;
	UINT32 u32Origen;

	// Comprobación de seguridad
	if (BB_Mask(u32Casilla) & pPos->u64TodasB)
		return(BB_TABLEROVACIO);

	// Caballos
	u64Ataques |= (pPos->u64CaballosB & BB_ATAQUES_CABALLO[u32Casilla]);
	// Alfiles o damas
	u64Desde = (pPos->u64AlfilesB | pPos->u64DamasB) & BB_ATAQUES_ALFIL[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Torres o damas
	u64Desde = (pPos->u64TorresB | pPos->u64DamasB)	& au64AtaquesTorre[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Peones blancos
	if (u32Casilla < 48)
	{
		UINT64 u64Temp = (pPos->u64PeonesB << 8) & ~u64Todas;
		if (u64Temp & BB_Mask(u32Casilla))
			u64Ataques |= BB_Mask(u32Casilla + 8);
		u64Temp = (u64Temp << 8) & BB_FILA4H4 & ~u64Todas;
		if (u64Temp & BB_Mask(u32Casilla))
			u64Ataques |= BB_Mask(u32Casilla + 16);
	}
	// Rey
	u64Ataques |= (BB_Mask(pPos->u8PosReyB) & BB_ATAQUES_REY[u32Casilla]);

	return(u64Ataques);
}

/*
	Movimientos negros a una casilla. Es != de ataques por los peones
*/
UINT64 MovimientosN_Hacia(TPosicion * pPos, UINT32 u32Casilla)
{
	UINT64 u64Ataques = BB_TABLEROVACIO;
	UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	UINT64 u64Desde;
	UINT32 u32Origen;

	// Comprobación de seguridad
	if (BB_Mask(u32Casilla) & pPos->u64TodasN)
		return(BB_TABLEROVACIO);

	// Caballos
	u64Ataques |= (pPos->u64CaballosN & BB_ATAQUES_CABALLO[u32Casilla]);
	// Alfiles o damas
	u64Desde = (pPos->u64AlfilesN | pPos->u64DamasN) & BB_ATAQUES_ALFIL[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Torres o damas
	u64Desde = (pPos->u64TorresN | pPos->u64DamasN)	& au64AtaquesTorre[u32Casilla];
	while (u64Desde)
	{
		u32Origen = BB_GetBitYQuitar(&u64Desde);
		if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
			u64Ataques |= BB_Mask(u32Origen);
	}
	// Peones negros
	if (u32Casilla > 15)
	{
		UINT64 u64Temp = (pPos->u64PeonesN >> 8) & ~u64Todas;
		if (u64Temp & BB_Mask(u32Casilla))
			u64Ataques |= BB_Mask(u32Casilla - 8);
		u64Temp = (u64Temp >> 8) & BB_FILA5H5 & ~u64Todas;
		if (u64Temp & BB_Mask(u32Casilla))
			u64Ataques |= BB_Mask(u32Casilla - 16);
	}
	// Rey
	u64Ataques |= (BB_Mask(pPos->u8PosReyN) & BB_ATAQUES_REY[u32Casilla]);

	return(u64Ataques);
}

