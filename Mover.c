/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de la función
	Mover
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"
#include "Bitboards_inline.h"
#include "Funciones.h"
#include "nnue.h"
#include <string.h>

static inline void Captura(TPosicion* pPosNueva, UINT32 u32PiezaEnDestino, UINT32 u32Hasta, UINT64 BBMASK_HASTA)
{
	// Actualizar contadores de tablas
	pPosNueva->u8Cincuenta = 0;
	pPosNueva->pPunteroRepeticion = pPosNueva;

	// Según pieza capturada
	switch (u32PiezaEnDestino)
	{
		case PB:
			pPosNueva->u64PeonesB ^= BBMASK_HASTA;
			pPosNueva->u8NumPeonesB--;
			pPosNueva->u64HashSignature ^= au64PeonB_Random[u32Hasta];
			pPosNueva->s32EvalMaterial -= VAL_PEON;
			pPosNueva->u64TodasB ^= BBMASK_HASTA;
			return;
		case CB:
			pPosNueva->u64CaballosB ^= BBMASK_HASTA;
			pPosNueva->u8NumPiezasB--;
			pPosNueva->u64HashSignature ^= au64CaballoB_Random[u32Hasta];
			pPosNueva->s32EvalMaterial -= VAL_CABALLO;
			pPosNueva->u64TodasB ^= BBMASK_HASTA;
			return;
		case AB:
			pPosNueva->u64AlfilesB ^= BBMASK_HASTA;
			pPosNueva->u8NumPiezasB--;
			pPosNueva->u64HashSignature ^= au64AlfilB_Random[u32Hasta];
			pPosNueva->s32EvalMaterial -= VAL_ALFIL;
			pPosNueva->u64TodasB ^= BBMASK_HASTA;
			return;
		case TB:
			pPosNueva->u64TorresB ^= BBMASK_HASTA;
			pPosNueva->u8NumPiezasB--;
			pPosNueva->u64HashSignature ^= au64TorreB_Random[u32Hasta];
			pPosNueva->s32EvalMaterial -= VAL_TORRE;
			pPosNueva->u64TodasB ^= BBMASK_HASTA;
			return;
		case DB:
			pPosNueva->u64DamasB ^= BBMASK_HASTA;
			pPosNueva->u8NumPiezasB--;
			pPosNueva->u64HashSignature ^= au64DamaB_Random[u32Hasta];
			pPosNueva->s32EvalMaterial -= VAL_DAMA;
			pPosNueva->u64TodasB ^= BBMASK_HASTA;
			return;
		case PN:
			pPosNueva->u64PeonesN ^= BBMASK_HASTA;
			pPosNueva->u8NumPeonesN--;
			pPosNueva->u64HashSignature ^= au64PeonN_Random[u32Hasta];
			pPosNueva->s32EvalMaterial += VAL_PEON;
			pPosNueva->u64TodasN ^= BBMASK_HASTA;
			return;
		case CN:
			pPosNueva->u64CaballosN ^= BBMASK_HASTA;
			pPosNueva->u8NumPiezasN--;
			pPosNueva->u64HashSignature ^= au64CaballoN_Random[u32Hasta];
			pPosNueva->s32EvalMaterial += VAL_CABALLO;
			pPosNueva->u64TodasN ^= BBMASK_HASTA;
			return;
		case AN:
			pPosNueva->u64AlfilesN ^= BBMASK_HASTA;
			pPosNueva->u8NumPiezasN--;
			pPosNueva->u64HashSignature ^= au64AlfilN_Random[u32Hasta];
			pPosNueva->s32EvalMaterial += VAL_ALFIL;
			pPosNueva->u64TodasN ^= BBMASK_HASTA;
			return;
		case TN:
			pPosNueva->u64TorresN ^= BBMASK_HASTA;
			pPosNueva->u8NumPiezasN--;
			pPosNueva->u64HashSignature ^= au64TorreN_Random[u32Hasta];
			pPosNueva->s32EvalMaterial += VAL_TORRE;
			pPosNueva->u64TodasN ^= BBMASK_HASTA;
			return;
		case DN:
			pPosNueva->u64DamasN ^= BBMASK_HASTA;
			pPosNueva->u8NumPiezasN--;
			pPosNueva->u64HashSignature ^= au64DamaN_Random[u32Hasta];
			pPosNueva->s32EvalMaterial += VAL_DAMA;
			pPosNueva->u64TodasN ^= BBMASK_HASTA;
			return;
		default:
			__assume(0);
	}
}

BOOL Mover(TPosicion * pPos,TJugada * pJug)
{
	TPosicion* pPosNueva;
	const UINT32 u32Desde = Jug_GetDesde(*pJug);
	const UINT32 u32Hasta = Jug_GetHasta(*pJug);
	const UINT32 u32PiezaEnOrigen = PiezaEnCasilla(pPos, u32Desde);
	const UINT32 u32PiezaEnDestino = PiezaEnCasilla(pPos, u32Hasta);
	const UINT64 BBMASK_DESDE = BB_Mask(u32Desde);
	const UINT64 BBMASK_HASTA = BB_Mask(u32Hasta);

	pPosNueva = pPos + 1;
	*pPosNueva = *pPos;

	// Copias comunes a todos los movimientos
	pPosNueva->u64HashSignature ^= au64AlPaso_Random[pPos->u8AlPaso];
	pPosNueva->u64HashSignature ^= au64AlPaso_Random[TAB_ALPASOIMPOSIBLE];
	pPosNueva->u8AlPaso = TAB_ALPASOIMPOSIBLE;
	pPosNueva->pListaJug = pJug;

	assert(u32PiezaEnOrigen > 0);
	assert(u32PiezaEnOrigen != PB || Tab_GetFila(u32Desde) != 1 || Jug_GetPromo(*pJug));
	assert(u32PiezaEnOrigen != PN || Tab_GetFila(u32Desde) != 6 || Jug_GetPromo(*pJug));

	switch(u32PiezaEnOrigen)
	{
		case PB:
			/*
				------------------------------------
							PEON BLANCO
				------------------------------------
			*/
			// Quitar PB de bitboards (peones + todas)
			pPosNueva->u64PeonesB ^= BBMASK_DESDE;
			pPosNueva->u64TodasB ^= BBMASK_DESDE;
			// Siempre aparece "algo" en hasta
			pPosNueva->u64TodasB |= BBMASK_HASTA;
			// Quitar PB de hash
			pPosNueva->u64HashSignature ^= au64PeonB_Random[u32Desde];
			// Actualizar contadores de tablas
			pPosNueva->u8Cincuenta = 0;
			pPosNueva->pPunteroRepeticion = pPosNueva;

			// *** AL PASO ***
			if ((UINT8)u32Hasta == pPos->u8AlPaso)
			{
				// Tareas particulares de una captura al paso
				pPosNueva->u64PeonesN ^= BB_Mask(u32Hasta + 8);
				pPosNueva->u64HashSignature ^= au64PeonN_Random[u32Hasta + 8];
				pPosNueva->u8NumPeonesN--;
				pPosNueva->u64TodasN ^= BB_Mask(u32Hasta + 8);
				pPosNueva->s32EvalMaterial += VAL_PEON;
			}

			// *** PROMOCIÓN ***
			if (Jug_GetPromo(*pJug) == VACIO)
			{
				pPosNueva->u64PeonesB |= BBMASK_HASTA;
				pPosNueva->u64HashSignature ^= au64PeonB_Random[u32Hasta];
			}
			else
			{
				// Tareas particulares para promociones
				pPosNueva->u8NumPeonesB--;
				pPosNueva->u8NumPiezasB++;
				pPosNueva->s32EvalMaterial -= VAL_PEON;
				switch(Jug_GetPromo(*pJug))
				{
					case DB:
						pPosNueva->u64DamasB |= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64DamaB_Random[u32Hasta];
						pPosNueva->s32EvalMaterial += VAL_DAMA;
						break;
					case CB:
						pPosNueva->u64CaballosB |= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64CaballoB_Random[u32Hasta];
						pPosNueva->s32EvalMaterial += VAL_CABALLO;
						break;
					case TB:
						pPosNueva->u64TorresB |= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64TorreB_Random[u32Hasta];
						pPosNueva->s32EvalMaterial += VAL_TORRE;
						break;
					case AB:
						pPosNueva->u64AlfilesB |= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64AlfilB_Random[u32Hasta];
						pPosNueva->s32EvalMaterial += VAL_ALFIL;
						break;
					default:
						__assume(0);
				}
			}

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
			{
				// Sólo aquí se puede dar "crear casilla al paso"
				if ((u32Hasta == u32Desde - 16) && (au64AdyacenteH[u32Hasta] & pPos->u64PeonesN))
				{
					pPosNueva->u8AlPaso = (UINT8)(u32Hasta + 8);
					pPosNueva->u64HashSignature ^= au64AlPaso_Random[TAB_ALPASOIMPOSIBLE];
					pPosNueva->u64HashSignature ^= au64AlPaso_Random[u32Hasta + 8];
				}
			}
			else
			{
				BB_InvertirBit(&pPosNueva->u64TodasN,u32Hasta);
				switch (u32PiezaEnDestino)
				{
					case PN:
						pPosNueva->u64PeonesN ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64PeonN_Random[u32Hasta];
						pPosNueva->u8NumPeonesN--;
						pPosNueva->s32EvalMaterial += VAL_PEON;
						break;
					case CN:
						pPosNueva->u64CaballosN ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64CaballoN_Random[u32Hasta];
						pPosNueva->u8NumPiezasN--;
						pPosNueva->s32EvalMaterial += VAL_CABALLO;
						break;
					case AN:
						pPosNueva->u64AlfilesN ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64AlfilN_Random[u32Hasta];
						pPosNueva->u8NumPiezasN--;
						pPosNueva->s32EvalMaterial += VAL_ALFIL;
						break;
					case TN:
						pPosNueva->u64TorresN ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64TorreN_Random[u32Hasta];
						pPosNueva->u8NumPiezasN--;
						pPosNueva->s32EvalMaterial += VAL_TORRE;
						break;
					case DN:
						pPosNueva->u64DamasN ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64DamaN_Random[u32Hasta];
						pPosNueva->u8NumPiezasN--;
						pPosNueva->s32EvalMaterial += VAL_DAMA;
						break;
					default:
						__assume(0);
				}
			}
			break;
		case CB:
			/*
				------------------------------------
							CABALLO BLANCO
				------------------------------------
			*/
			// CB cambia, y hash
			pPosNueva->u64CaballosB = (pPos->u64CaballosB ^ BBMASK_DESDE) | BBMASK_HASTA;
			pPosNueva->u64HashSignature ^= au64CaballoB_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64CaballoB_Random[u32Hasta];
			// Cambiar el CB en todas B
			pPosNueva->u64TodasB = (pPos->u64TodasB ^ BBMASK_DESDE) | BBMASK_HASTA;

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)				
				pPosNueva->u8Cincuenta++;	// No hay jugada irreversible
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);
			break;
		case AB:
			/*
				------------------------------------
							ALFIL BLANCO
				------------------------------------
			*/
			// AB cambia, y hash
			pPosNueva->u64AlfilesB = (pPos->u64AlfilesB ^ BBMASK_DESDE) | BBMASK_HASTA;
			pPosNueva->u64HashSignature ^= au64AlfilB_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64AlfilB_Random[u32Hasta];
			// Cambiar el AB en todas B
			pPosNueva->u64TodasB = (pPos->u64TodasB ^ BBMASK_DESDE) | BBMASK_HASTA;

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
				pPosNueva->u8Cincuenta++;	// No hay jugada irreversible
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);
			break;
		case TB:
			/*
				------------------------------------
							TORRE BLANCA
				------------------------------------
			*/
			// TB cambia, y hash
			pPosNueva->u64TorresB = (pPos->u64TorresB ^ BBMASK_DESDE) | BBMASK_HASTA;
			pPosNueva->u64HashSignature ^= au64TorreB_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64TorreB_Random[u32Hasta];
			// Cambiar la TB en todas B
			pPosNueva->u64TodasB = (pPos->u64TodasB ^ BBMASK_DESDE) | BBMASK_HASTA;

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
				pPosNueva->u8Cincuenta++;	// No hay jugada irreversible
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);

			// Puede que haya que quitar derechos de enroque
			if (u32Desde == TAB_H1)
			{
				Pos_SetNoEnroqueCortoB(pPosNueva);
				if (pPos->u8Enroques != pPosNueva->u8Enroques)
				{
					pPosNueva->u64HashSignature ^= au64Enroques_Random[pPos->u8Enroques];
					pPosNueva->u64HashSignature ^= au64Enroques_Random[pPosNueva->u8Enroques];
					pPosNueva->pPunteroRepeticion = pPosNueva;
				}
			}
			else if (u32Desde == TAB_A1)
			{
				Pos_SetNoEnroqueLargoB(pPosNueva);
				if (pPos->u8Enroques != pPosNueva->u8Enroques)
				{
					pPosNueva->u64HashSignature ^= au64Enroques_Random[pPos->u8Enroques];
					pPosNueva->u64HashSignature ^= au64Enroques_Random[pPosNueva->u8Enroques];
					pPosNueva->pPunteroRepeticion = pPosNueva;
				}
			}
			break;
		case DB:
			/*
				------------------------------------
							DAMA BLANCA
				------------------------------------
			*/
			// DB cambia, y hash
			pPosNueva->u64DamasB = (pPos->u64DamasB ^ BBMASK_DESDE) | BBMASK_HASTA;
			pPosNueva->u64HashSignature ^= au64DamaB_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64DamaB_Random[u32Hasta];
			// Cambiar la DB en todas B
			pPosNueva->u64TodasB = (pPos->u64TodasB ^ BBMASK_DESDE) | BBMASK_HASTA;

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
				pPosNueva->u8Cincuenta++;	// No hay jugada irreversible
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);
			break;
		case RB:
			/*
				------------------------------------
							REY BLANCO
				------------------------------------
			*/
			// RB cambia, y hash
			pPosNueva->u8PosReyB = (UINT8)u32Hasta;
			pPosNueva->u64HashSignature ^= au64ReyB_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64ReyB_Random[u32Hasta];
			// Cambiar el RB en todas B
			pPosNueva->u64TodasB = (pPos->u64TodasB ^ BBMASK_DESDE) | BBMASK_HASTA;

			// Anular enroques blancos
			Pos_SetNoEnroquesB(pPosNueva);
			if (pPos->u8Enroques != pPosNueva->u8Enroques)
			{
				pPosNueva->u64HashSignature ^= au64Enroques_Random[pPos->u8Enroques];
				pPosNueva->u64HashSignature ^= au64Enroques_Random[pPosNueva->u8Enroques];
				pPosNueva->pPunteroRepeticion = pPosNueva;
			}

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
			{
				// Sólo aquí puede haber enroque
				if (u32Desde == TAB_E1 && u32Hasta == TAB_G1)
				{
					// Enroque corto
					pPosNueva->u64TorresB ^= (BB_Mask(TAB_H1) | BB_Mask(TAB_F1));
					pPosNueva->u64TodasB ^= (BB_Mask(TAB_H1) | BB_Mask(TAB_F1));
					pPosNueva->u64HashSignature ^= au64TorreB_Random[TAB_H1];
					pPosNueva->u64HashSignature ^= au64TorreB_Random[TAB_F1];
				}
				else if (u32Desde == TAB_E1 && u32Hasta == TAB_C1)
				{
					// Enroque largo
					pPosNueva->u64TorresB ^= (BB_Mask(TAB_A1) | BB_Mask(TAB_D1));
					pPosNueva->u64TodasB ^= (BB_Mask(TAB_A1) | BB_Mask(TAB_D1));
					pPosNueva->u64HashSignature ^= au64TorreB_Random[TAB_A1];
					pPosNueva->u64HashSignature ^= au64TorreB_Random[TAB_D1];
				}

				// No hay jugada irreversible
				pPosNueva->u8Cincuenta++;
			}
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);
			break;
		case PN:
			/*
				------------------------------------
							PEON NEGRO
				------------------------------------
			*/
			// Quitar PN de bitboards (peones + todas)
			pPosNueva->u64PeonesN ^= BBMASK_DESDE;
			pPosNueva->u64TodasN ^= BBMASK_DESDE;
			// Siempre aparece "algo" en hasta
			pPosNueva->u64TodasN |= BBMASK_HASTA;
			// Quitar PN de hash
			pPosNueva->u64HashSignature ^= au64PeonN_Random[u32Desde];
			// Actualizar contadores de tablas
			pPosNueva->u8Cincuenta = 0;
			pPosNueva->pPunteroRepeticion = pPosNueva;

			// *** AL PASO ***
			if ((UINT8)u32Hasta == pPos->u8AlPaso)
			{
				// Tareas particulares de una captura al paso
				pPosNueva->u64PeonesB ^= BB_Mask(u32Hasta - 8);
				pPosNueva->u64HashSignature ^= au64PeonB_Random[u32Hasta - 8];
				pPosNueva->u8NumPeonesB--;
				pPosNueva->u64TodasB ^= BB_Mask(u32Hasta - 8);
				pPosNueva->s32EvalMaterial -= VAL_PEON;
			}

			// *** PROMOCIÓN ***
			if (Jug_GetPromo(*pJug) == VACIO)
			{
				pPosNueva->u64PeonesN |= BBMASK_HASTA;
				pPosNueva->u64HashSignature ^= au64PeonN_Random[u32Hasta];
			}
			else
			{
				// Tareas particulares para promociones
				pPosNueva->u8NumPeonesN--;
				pPosNueva->u8NumPiezasN++;
				pPosNueva->s32EvalMaterial += VAL_PEON;
				switch(Jug_GetPromo(*pJug))
				{
					case DB:
						pPosNueva->u64DamasN |= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64DamaN_Random[u32Hasta];
						pPosNueva->s32EvalMaterial -= VAL_DAMA;
						break;
					case CB:
						pPosNueva->u64CaballosN |= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64CaballoN_Random[u32Hasta];
						pPosNueva->s32EvalMaterial -= VAL_CABALLO;
						break;
					case TB:
						pPosNueva->u64TorresN |= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64TorreN_Random[u32Hasta];
						pPosNueva->s32EvalMaterial -= VAL_TORRE;
						break;
					case AB:
						pPosNueva->u64AlfilesN |= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64AlfilN_Random[u32Hasta];
						pPosNueva->s32EvalMaterial -= VAL_ALFIL;
						break;
					default:
						__assume(0);
				}
			}

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
			{
				// Sólo aquí se puede dar "al paso"
				if ((u32Hasta == u32Desde + 16) && (au64AdyacenteH[u32Hasta] & pPos->u64PeonesB))
				{
					pPosNueva->u8AlPaso = (UINT8)(u32Hasta - 8);
					pPosNueva->u64HashSignature ^= au64AlPaso_Random[TAB_ALPASOIMPOSIBLE];
					pPosNueva->u64HashSignature ^= au64AlPaso_Random[u32Hasta - 8];
				}
			}
			else
			{
				BB_InvertirBit(&pPosNueva->u64TodasB,u32Hasta);
				switch (u32PiezaEnDestino)
				{
					case PB:
						pPosNueva->u64PeonesB ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64PeonB_Random[u32Hasta];
						pPosNueva->u8NumPeonesB--;
						pPosNueva->s32EvalMaterial -= VAL_PEON;
						break;
					case CB:
						pPosNueva->u64CaballosB ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64CaballoB_Random[u32Hasta];
						pPosNueva->u8NumPiezasB--;
						pPosNueva->s32EvalMaterial -= VAL_CABALLO;
						break;
					case AB:
						pPosNueva->u64AlfilesB ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64AlfilB_Random[u32Hasta];
						pPosNueva->u8NumPiezasB--;
						pPosNueva->s32EvalMaterial -= VAL_ALFIL;
						break;
					case TB:
						pPosNueva->u64TorresB ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64TorreB_Random[u32Hasta];
						pPosNueva->u8NumPiezasB--;
						pPosNueva->s32EvalMaterial -= VAL_TORRE;
						break;
					case DB:
						pPosNueva->u64DamasB ^= BBMASK_HASTA;
						pPosNueva->u64HashSignature ^= au64DamaB_Random[u32Hasta];
						pPosNueva->u8NumPiezasB--;
						pPosNueva->s32EvalMaterial -= VAL_DAMA;
						break;
					default:
						__assume(0);
				}
			}
			break;
		case CN:
			/*
				------------------------------------
							CABALLO NEGRO
				------------------------------------
			*/
			// CN cambia, y hash
			pPosNueva->u64CaballosN = (pPos->u64CaballosN ^ BBMASK_DESDE) | BBMASK_HASTA;
			pPosNueva->u64HashSignature ^= au64CaballoN_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64CaballoN_Random[u32Hasta];
			// Cambiar el CN en todas N
			pPosNueva->u64TodasN = (pPos->u64TodasN ^ BBMASK_DESDE) | BBMASK_HASTA;

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
				pPosNueva->u8Cincuenta++;	// No hay jugada irreversible
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);
			break;
		case AN:
			/*
				------------------------------------
							ALFIL NEGRO
				------------------------------------
			*/
			// AN cambia, y hash
			pPosNueva->u64AlfilesN = (pPos->u64AlfilesN ^ BBMASK_DESDE) | BBMASK_HASTA;
			pPosNueva->u64HashSignature ^= au64AlfilN_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64AlfilN_Random[u32Hasta];
			// Cambiar el AN en todas N
			pPosNueva->u64TodasN = (pPos->u64TodasN ^ BBMASK_DESDE) | BBMASK_HASTA;

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
				pPosNueva->u8Cincuenta++;	// No hay jugada irreversible
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);
			break;
		case TN:
			/*
				------------------------------------
							TORRE NEGRA
				------------------------------------
			*/
			// TN cambia, y hash
			pPosNueva->u64TorresN = (pPos->u64TorresN ^ BBMASK_DESDE) | BBMASK_HASTA;
			pPosNueva->u64HashSignature ^= au64TorreN_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64TorreN_Random[u32Hasta];
			// Cambiar la TN en todas N
			pPosNueva->u64TodasN = (pPos->u64TodasN ^ BBMASK_DESDE) | BBMASK_HASTA;

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
				pPosNueva->u8Cincuenta++;	// No hay jugada irreversible
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);

			// Puede que haya que quitar derechos de enroque
			if (u32Desde == TAB_H8)
			{
				Pos_SetNoEnroqueCortoN(pPosNueva);
				if (pPos->u8Enroques != pPosNueva->u8Enroques)
				{
					pPosNueva->u64HashSignature ^= au64Enroques_Random[pPos->u8Enroques];
					pPosNueva->u64HashSignature ^= au64Enroques_Random[pPosNueva->u8Enroques];
					pPosNueva->pPunteroRepeticion = pPosNueva;
				}
			}
			else if (u32Desde == TAB_A8)
			{
				Pos_SetNoEnroqueLargoN(pPosNueva);
				if (pPos->u8Enroques != pPosNueva->u8Enroques)
				{
					pPosNueva->u64HashSignature ^= au64Enroques_Random[pPos->u8Enroques];
					pPosNueva->u64HashSignature ^= au64Enroques_Random[pPosNueva->u8Enroques];
					pPosNueva->pPunteroRepeticion = pPosNueva;
				}
			}
			break;
		case DN:
			/*
				------------------------------------
							DAMA NEGRA
				------------------------------------
			*/
			// DN cambia, y hash
			pPosNueva->u64DamasN = (pPos->u64DamasN ^ BBMASK_DESDE) | BBMASK_HASTA;
			pPosNueva->u64HashSignature ^= au64DamaN_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64DamaN_Random[u32Hasta];
			// Cambiar la DN en todas N
			pPosNueva->u64TodasN = (pPos->u64TodasN ^ BBMASK_DESDE) | BBMASK_HASTA;

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
				pPosNueva->u8Cincuenta++;	// No hay jugada irreversible
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);
			break;
		case RN:
			/*
				------------------------------------
							REY NEGRO
				------------------------------------
			*/
			// RN cambia, y hash
			pPosNueva->u8PosReyN = (UINT8)u32Hasta;
			pPosNueva->u64HashSignature ^= au64ReyN_Random[u32Desde];
			pPosNueva->u64HashSignature ^= au64ReyN_Random[u32Hasta];
			// Cambiar el RN en todas N
			pPosNueva->u64TodasN = (pPos->u64TodasN ^ BBMASK_DESDE) | BBMASK_HASTA;

			// Anular enroques negros
			Pos_SetNoEnroquesN(pPosNueva);
			if (pPos->u8Enroques != pPosNueva->u8Enroques)
			{
				pPosNueva->u64HashSignature ^= au64Enroques_Random[pPos->u8Enroques];
				pPosNueva->u64HashSignature ^= au64Enroques_Random[pPosNueva->u8Enroques];
				pPosNueva->pPunteroRepeticion = pPosNueva;
			}

			// *** CAPTURA ***
			if (u32PiezaEnDestino == VACIO)
			{
				// Sólo aquí puede haber enroque
				if (u32Desde == TAB_E8 && u32Hasta == TAB_G8)
				{
					// Enroque corto
					pPosNueva->u64TorresN ^= (BB_Mask(TAB_H8) | BB_Mask(TAB_F8));
					pPosNueva->u64TodasN ^= (BB_Mask(TAB_H8) | BB_Mask(TAB_F8));
					pPosNueva->u64HashSignature ^= au64TorreN_Random[TAB_H8];
					pPosNueva->u64HashSignature ^= au64TorreN_Random[TAB_F8];
				}
				else if (u32Desde == TAB_E8 && u32Hasta == TAB_C8)
				{
					// Enroque largo
					pPosNueva->u64TorresN ^= (BB_Mask(TAB_A8) | BB_Mask(TAB_D8));
					pPosNueva->u64TodasN ^= (BB_Mask(TAB_A8) | BB_Mask(TAB_D8));
					pPosNueva->u64HashSignature ^= au64TorreN_Random[TAB_A8];
					pPosNueva->u64HashSignature ^= au64TorreN_Random[TAB_D8];
				}

				// No hay jugada irreversible
				pPosNueva->u8Cincuenta++;
			}
			else
				Captura(pPosNueva, u32PiezaEnDestino, u32Hasta, BBMASK_HASTA);
			break;
		default:
			__assume(0);
	}

	if (Pos_GetTurno(pPos) == NEGRAS)
	{
		// Verificación de legalidad
		if (Atacado(pPosNueva, pPosNueva->u8PosReyN, BLANCAS))
		{
			assert((pPosNueva->u64TodasB & pPosNueva->u64TodasN) == 0);
			assert(pPosNueva->u64TodasB == (pPosNueva->u64PeonesB | pPosNueva->u64CaballosB | pPosNueva->u64AlfilesB | pPosNueva->u64TorresB | pPosNueva->u64DamasB | BB_Mask(pPosNueva->u8PosReyB)));
			assert(pPosNueva->u64TodasN == (pPosNueva->u64PeonesN | pPosNueva->u64CaballosN | pPosNueva->u64AlfilesN | pPosNueva->u64TorresN | pPosNueva->u64DamasN | BB_Mask(pPosNueva->u8PosReyN)));
			assert(pPosNueva->u8NumPeonesB == CuentaUnos(pPosNueva->u64PeonesB));
			assert(pPosNueva->u8NumPeonesN == CuentaUnos(pPosNueva->u64PeonesN));
			return(FALSE);
		}
		// Turno
		Pos_SetTurnoB(pPosNueva);

		// Jaqueado
		if (Atacado(pPosNueva, pPosNueva->u8PosReyB, NEGRAS))
			Pos_SetJaqueado(pPosNueva);
		else
			Pos_SetNoJaqueado(pPosNueva);
	}
	else
	{
		// Verificación de legalidad
		if (Atacado(pPosNueva, pPosNueva->u8PosReyB, NEGRAS))
		{
			assert((pPosNueva->u64TodasB & pPosNueva->u64TodasN) == 0);
			assert(pPosNueva->u64TodasB == (pPosNueva->u64PeonesB | pPosNueva->u64CaballosB | pPosNueva->u64AlfilesB | pPosNueva->u64TorresB | pPosNueva->u64DamasB | BB_Mask(pPosNueva->u8PosReyB)));
			assert(pPosNueva->u64TodasN == (pPosNueva->u64PeonesN | pPosNueva->u64CaballosN | pPosNueva->u64AlfilesN | pPosNueva->u64TorresN | pPosNueva->u64DamasN | BB_Mask(pPosNueva->u8PosReyN)));
			assert(pPosNueva->u8NumPeonesB == CuentaUnos(pPosNueva->u64PeonesB));
			assert(pPosNueva->u8NumPeonesN == CuentaUnos(pPosNueva->u64PeonesN));
			return(FALSE);
		}
		// Turno
		Pos_SetTurnoN(pPosNueva);

		// Jaqueado
		if (Atacado(pPosNueva, pPosNueva->u8PosReyN, BLANCAS))
			Pos_SetJaqueado(pPosNueva);
		else
			Pos_SetNoJaqueado(pPosNueva);
	}
	pPosNueva->pListaJug = pJug + 1;
	Pos_SetIntentarNull(pPosNueva);
	pPosNueva->s32Eval = NO_EVAL;
	dbDatosBusqueda.u32NumNodos++;
	pPosNueva->u32MovExclu = 0;

	//
	// Seguridad
	//
	assert((pPos->u64TodasB & pPos->u64TodasN) == 0);
	assert(pPos->u64TodasB == (pPos->u64PeonesB | pPos->u64CaballosB | pPos->u64AlfilesB | pPos->u64TorresB | pPos->u64DamasB | BB_Mask(pPos->u8PosReyB)));
	assert(pPos->u64TodasN == (pPos->u64PeonesN | pPos->u64CaballosN | pPos->u64AlfilesN | pPos->u64TorresN | pPos->u64DamasN | BB_Mask(pPos->u8PosReyN)));
	assert((pPosNueva->u64TodasB & pPosNueva->u64TodasN) == 0);
	assert(pPosNueva->u64TodasB == (pPosNueva->u64PeonesB | pPosNueva->u64CaballosB | pPosNueva->u64AlfilesB | pPosNueva->u64TorresB | pPosNueva->u64DamasB | BB_Mask(pPosNueva->u8PosReyB)));
	assert(pPosNueva->u64TodasN == (pPosNueva->u64PeonesN | pPosNueva->u64CaballosN | pPosNueva->u64AlfilesN | pPosNueva->u64TorresN | pPosNueva->u64DamasN | BB_Mask(pPosNueva->u8PosReyN)));
	assert(Pos_GetJaqueado(pPosNueva) == (Pos_GetTurno(pPosNueva) == BLANCAS ? Atacado(pPosNueva,pPosNueva->u8PosReyB,NEGRAS) : Atacado(pPosNueva,pPosNueva->u8PosReyN,BLANCAS)));
	assert(Pos_GetTurno(pPos) != Pos_GetTurno(pPosNueva));
	assert(pPosNueva->s32EvalMaterial == EvaluarMaterial(pPosNueva));
	assert(pPosNueva->s32EvalMaterial == pPos->s32EvalMaterial || Pos_GetCaptura(pPosNueva) || (UINT8)u32Hasta == pPos->u8AlPaso || Jug_GetPromo(*pJug));
	assert(pPos->u8NumPeonesB == CuentaUnos(pPos->u64PeonesB));
	assert(pPos->u8NumPeonesN == CuentaUnos(pPos->u64PeonesN));
	assert(pPosNueva->u8NumPeonesB == CuentaUnos(pPosNueva->u64PeonesB));
	assert(pPosNueva->u8NumPeonesN == CuentaUnos(pPosNueva->u64PeonesN));

	pPos->UltMovPieza = u32PiezaEnOrigen;
	pPos->UltMovHasta = u32Hasta;

	return(TRUE);
}