
#include "Preprocesador.h"
#include "Tipos.h"
#include "Constantes.h"
#include "Variables.h"
#include "Bitboards_inline.h"

/*
 *
 * Atacado
 *
 *
 * Recibe: un puntero a la posición actual, una casilla y el color de las piezas atacantes
 *
 * Devuelve: TRUE si la casilla está atacada por el bando indicado, FALSE en caso contrario
 *
 * Descripción: Trata de generar jugadas a partir de la casilla dada para cada tipo de pieza. Luego comprueba si tal pieza está en el destino esperado. Devuelve TRUE tan pronto como encuentra un ataque sobre la casilla
 *
 */
UINT32 Atacado(TPosicion * pPos, UINT32 u32Casilla, UINT32 u32Color)
{
	UINT64 u64Desde;
	UINT64 u64Todas;
	UINT32 u32Origen;

	if (u32Color == BLANCAS)
	{
		//
		// BLANCAS
		//
		//{
		//	UINT64 u64TodosLosAtaques = au64AtaquesPeonN[u32Casilla] | BB_ATAQUES_CABALLO[u32Casilla] | BB_ATAQUES_ALFIL[u32Casilla] | au64AtaquesTorre[u32Casilla] | BB_ATAQUES_REY[u32Casilla];

		//	if (u64TodosLosAtaques & pPos->u64TodasB == BB_TABLEROVACIO)
		//		return(FALSE);
		//}

		// Caballos
		if (pPos->u64CaballosB & BB_ATAQUES_CABALLO[u32Casilla])
			return(TRUE);

		u64Todas = pPos->u64TodasB | pPos->u64TodasN;

		// Alfiles o damas
		u64Desde = (pPos->u64AlfilesB | pPos->u64DamasB) & BB_ATAQUES_ALFIL[u32Casilla];
		while (u64Desde)
		{
			u32Origen = BB_GetBitYQuitar(&u64Desde);
			if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
				return(TRUE);
		}

		// Torres o damas
		u64Desde = (pPos->u64TorresB | pPos->u64DamasB) & au64AtaquesTorre[u32Casilla];
		while (u64Desde)
		{
			u32Origen = BB_GetBitYQuitar(&u64Desde);
			if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
				return(TRUE);
		}

		// Peones
		if (u32Casilla < 48) // En las dos filas de abajo, no me puede atacar un PB
		{
			if (pPos->u64PeonesB & (((BB_Mask(u32Casilla) & BB_SINCOLIZDA) >> 7) | ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) >> 9)))
				return (TRUE);
		}

		// Rey
		if (BB_Mask(pPos->u8PosReyB) & BB_ATAQUES_REY[u32Casilla])
			return(TRUE);
	} // if (u32Color == BLANCAS)
	else
	{
		//
		// NEGRAS
		//

		// Caballos
		if (pPos->u64CaballosN & BB_ATAQUES_CABALLO[u32Casilla])
			return(TRUE);

		u64Todas = pPos->u64TodasB | pPos->u64TodasN;

		// Alfiles o damas
		u64Desde = (pPos->u64AlfilesN | pPos->u64DamasN) & BB_ATAQUES_ALFIL[u32Casilla];
		while (u64Desde)
		{
			u32Origen = BB_GetBitYQuitar(&u64Desde);
			if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
				return(TRUE);
		}

		// Torres o damas
		u64Desde = (pPos->u64TorresN | pPos->u64DamasN) & au64AtaquesTorre[u32Casilla];
		while (u64Desde)
		{
			u32Origen = BB_GetBitYQuitar(&u64Desde);
			if (!(au64Entre[u32Origen][u32Casilla] & u64Todas))
				return(TRUE);
		}

		// Peones
		if (u32Casilla > 15)
		{
			if (pPos->u64PeonesN & (((BB_Mask(u32Casilla) & BB_SINCOLIZDA) << 9) | ((BB_Mask(u32Casilla) & BB_SINCOLDCHA) << 7)))
				return (TRUE);
		}

		// Rey
		if (BB_Mask(pPos->u8PosReyN) & BB_ATAQUES_REY[u32Casilla])
			return(TRUE);
	} // else [if (u32Color == BLANCAS)]

	return(FALSE);
}
