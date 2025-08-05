
#include <windows.h>
#include "Preprocesador.h"
#include "Tipos.h"
#include "Funciones.h"
#include <stdio.h>
#include "Variables.h"
#include "Inline.h"
#include <string.h>
#include <math.h>
#include "Constantes.h"

/*
 *
 * ImprimirParcialPlyConPV
 *
 */
static void ImprimirParcialPlyConPV(TPosicion * pPos, SINT32 s32Prof, SINT32 s32Eval, char * c)
{
	if (s32Prof < 9 && (dbDatosBusqueda.eTipoBusqueda == TBU_PARTIDA || dbDatosBusqueda.eTipoBusqueda == TBU_PONDER))
		return;

	char szPV[1600];
	char szPVTemp[1500];

	ExtraerPVdeArray(pPos, szPVTemp);

	sprintf(szPV, "(%d/%d%s) ", s32Prof, dbDatosBusqueda.s32ProfSel, c);
	strcat(szPV, szPVTemp);


	ImprimirCadenaGUI(s32Prof, s32Eval, szPV);
}

/*
 *
 * ImprimirParcialPlySinPV
 *
 */
static void ImprimirParcialPlySinPV(SINT32 s32Prof, SINT32 s32Eval, char* szJug)
{
	if (s32Prof < 9 && (dbDatosBusqueda.eTipoBusqueda == TBU_PARTIDA || dbDatosBusqueda.eTipoBusqueda == TBU_PONDER))
		return;

	char sz[20];

	sprintf(sz, "(%d/%d) %s!?", s32Prof, dbDatosBusqueda.s32ProfSel, szJug);


	ImprimirCadenaGUI(s32Prof, s32Eval, sz);
}

//
// Aspiracíón
//
// Laser 1.7:	delta = 14 - min(prof / 4, 6) + abs(bestScore) / 25;		// Ejemplo: prof 10 score +0.25 -> delta = 14 - 2 + 25 / 25 = 13
//				actualiza delta por 3/2
//
void Aspiracion(TPosicion* pPos, TJugada* pJugPV, SINT32 s32Prof)
{
	int iAlfa;
	int iBeta;
	int iDelta = AS_DELTA_DIN_BASE;
	int iScore;
	int iProfOrig = s32Prof;
	static int iIncertidumbre = 0;

	//
	//					Base	Ajuste					Incremento
	//					------------------------------------------
	// Alexandria 8:	  12							*= 1.44
	// Senpai 2.0:		  10							*= 2
	// Arcanum 2.4:		  25							*= 2
	// Rodent IV:		   8							*= 2
	// Peacekeeper 3.01:  28							*= 2
	// Berserk 2025		   9							+= 17 * d/64
	// Ruk 4.0.0:		  17							+= 5 + d/4
	// CFish 12.5		  17							+= 5 + d/4
	// Renegade 1.1.9:	  20							+= d / 2
	// ElBurro 1.3:		   8							+= d / 2
	// Cadie 2.1:		  10							+= d / 2
	// Demolito 2025:	  15							+= d / 2
	// Kobra 2.0:		  17							+= d / 3
	// Astra 5.1:		   9							+= d / 3
	// Caissa 1.22:		   6	lasteval / 16			+= d / 3
	// Kookaburra 3.10:	   5	(promedio^2) / 12991	+= d / 3
	// Brainlearn 30:	   5	(promedio^2) / 13797	+= d / 3
	// Omega MPV:		  10	promedio / 20			+= d / 3
	// Clover 8.1:		   7	(lasteval^2) / 10000	+= d * 31/100
	// 
	// Yo				  10	lasteval / 8			*= 2 ó *= 3
	//
	if (s32Prof >= 5)
	{
		iDelta = AS_DELTA_DIN_BASE + abs(dbDatosBusqueda.s32EvalRoot) / AS_DELTA_DIN_DIVISOR;
		iDelta += (iIncertidumbre + dbDatosBusqueda.s32CambiosJugMejor);

		iAlfa = max(dbDatosBusqueda.s32EvalRoot - iDelta, -INFINITO);
		iBeta = min(dbDatosBusqueda.s32EvalRoot + iDelta, INFINITO);
	}
	else
	{
		iAlfa = -INFINITO;
		iBeta = INFINITO;
		iIncertidumbre = 0;
	}

	dbDatosBusqueda.bFailLowEnPV = FALSE;
	dbDatosBusqueda.bAnalizandoPV = TRUE;
	while (TRUE)
	{
		if (iAlfa <= -1500)
			iAlfa = -INFINITO;
		if (iBeta >= 1500)
			iBeta = INFINITO;

		//
		// Búsqueda
		//
		Ordenar(pPos, pJugPV);
		iScore = Raiz(pPos, pJugPV, iAlfa, iBeta, s32Prof);
		if (dbDatosBusqueda.bAbortar)
			break;

		// Salida por valor exacto
		if (iAlfa < iScore && iScore < iBeta)
			break;

		// Fail low
		if (iScore <= iAlfa)
		{
			dbDatosBusqueda.bFailLowEnPV = TRUE;
			dbDatosBusqueda.s32EvalRoot = iAlfa;
			iBeta = (iAlfa + iBeta) / 2;
			iAlfa -= iDelta;
			if (iAlfa < -2000)
				iAlfa = -INFINITO;
			iDelta *= AS_DELTA_FL_MULT;
			s32Prof = iProfOrig;
			iIncertidumbre += 2;
		}
		// Fail high
		else
		{
			dbDatosBusqueda.s32EvalRoot = iBeta;
			iBeta += iDelta;
			iDelta *= AS_DELTA_FH_MULT;
			if (iBeta > 2000)
				iBeta = INFINITO;
			if (!Bus_DoyMatePronto(iScore))
				s32Prof -= (s32Prof > 10);
			iIncertidumbre++;
		}
	}
	iIncertidumbre = max(iIncertidumbre - 1, 0);
}

SINT32 Raiz(TPosicion* pPos, TJugada* pJugPV, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Prof)
{
	static UINT32 u32TotalLegales = 10000;

	TJugada* pJug;
	UINT32 u32Legales = 0;
	UINT32 u32Temp;
	int iVal;
	int iMejor = -INFINITO;
	char szJugada[6];

	// Recorrer las jugadas
	for (pJug = pJugPV; pJug >= pPos->pListaJug; pJug--)
	{
		// Mover
		if (!Mover(pPos, pJug))
			continue;
		(pPos + 1)->pListaJug = pJugPV + 1;	// Porque Mover pone ese puntero justo detrás de la jugada, y me lía

		// Imprimir lo que está analizando
		if (s32Prof > 9)
		{
			SINT64 s64Centisegundos = GetCentisegundosTranscurridos(&g_tReloj);

			if (s64Centisegundos > 200)
			{
				Jug2Algebraica(*pJug, szJugada);
				printf("stat01: %lld %u %d %u %u %s\n",
					s64Centisegundos,
					dbDatosBusqueda.u32NumNodos,
					dbDatosBusqueda.s32ProfRoot,
					u32TotalLegales - u32Legales - 1,
					u32TotalLegales,
					szJugada);
			}
		}

		if (++u32Legales == 1)
		{
			//
			// Estamos en la PV (intentamos aspiración)
			//
			dbDatosBusqueda.bAnalizandoPV = TRUE;
			dbDatosBusqueda.jugJugadaRoot = *pJug;

			iMejor = -AlfaBetaPV(pPos + 1, -s32Beta, -s32Alfa, s32Prof - 1, 1);
			if (dbDatosBusqueda.bAbortar)
				goto Salir;

			dbDatosBusqueda.s32EvalRoot = iMejor;
			dbDatosBusqueda.jugJugadaRoot = *pJug;
			Bus_ActualizarPV(*pJug, 0);
			ImprimirParcialPlyConPV(pPos, dbDatosBusqueda.s32ProfRoot, iMejor, (iMejor >= s32Beta ? "+" : iMejor <= s32Alfa ? "-" : "\0"));

			// Salir por fail high
			if (iMejor >= s32Beta)
				return(iMejor);
		}
		else
		{
			//
			// No es PV (ventanas nulas)
			//
			dbDatosBusqueda.bAnalizandoPV = FALSE;

			// Ventana nula alrededor de iMejor
			u32Temp = dbDatosBusqueda.u32NumNodos;

			iVal = -AlfaBeta(pPos + 1, -iMejor - 1, -iMejor, s32Prof - 1, 1, TRUE);
			if (dbDatosBusqueda.bAbortar)
				goto Salir;

			// Para el orden de las jugadas en la raíz
			if (Jug_GetVal(*pJug) < CONTADOR_ORDEN_ROOT_INI)
			{
				u32Temp = dbDatosBusqueda.u32NumNodos - u32Temp;
				u32Temp /= ((UINT32)pow(2, s32Prof) + 1);
				if (u32Temp >= CONTADOR_ORDEN_ROOT_INI)
					u32Temp = CONTADOR_ORDEN_ROOT_INI - 1;
				Jug_SetVal(pJug, u32Temp);
			}

			// Comprobamos cómo ha ido la búsqueda
			if (iVal > iMejor)
			{
				dbDatosBusqueda.bAnalizandoPV = TRUE;
				dbDatosBusqueda.jugObvia.u32Mov = 0;
				Jug2SAN(pPos + 1, *pJug, szJugada); 

				// Rebúsqueda abriendo ventana hasta beta
				iVal = -AlfaBetaPV(pPos + 1, -s32Beta, -s32Alfa, s32Prof - 1, 1);

				if (dbDatosBusqueda.bAbortar)
					goto Salir;

				if (iVal > iMejor)
				{
					// Se confirma el fail high con la ventana abierta
					dbDatosBusqueda.s32CambiosJugMejor += 2;
					dbDatosBusqueda.s32EvalRoot = iVal;
					dbDatosBusqueda.jugJugadaRoot = *pJug;
					Bus_ActualizarPV(*pJug, 0);
					Jug_SetVal(pJug, ++dbDatosBusqueda.u32ContadorOrdenRoot);
					ImprimirParcialPlyConPV(pPos, dbDatosBusqueda.s32ProfRoot, iVal, (iVal >= s32Beta ? "+" : iVal <= s32Alfa ? "-" : "\0"));
					iMejor = iVal;

					if (iVal >= s32Beta)
						return(iVal);
				}
			}

			if (dbDatosBusqueda.bAbortar)
				goto Salir;
		} // else [if (++u32Legales == 1)]
	} // Recorrer las jugadas

	//
	// Ya hemos analizado todas las jugadas
	// o bien estamos aquí porque se ha cortado por bandera de abortar y hacemos goto
	//
Salir:
	u32TotalLegales = u32Legales;

	// Comprobar si estamos "matados" o "ahogados"
	if (!u32Legales)
	{
		#if (FICHERO_LOG == AV_LOG)
			ImprimirALog("*DBG* AS_Root(): u32Legales == 0");
		#endif
		dbDatosBusqueda.bAbortar = TRUE;
		dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;
		dbDatosBusqueda.jugJugadaRoot = JUGADA_NULA;
		Bus_ActualizarPV(JUGADA_NULA, 0);
		if (Pos_GetJaqueado(pPos))
			dbDatosBusqueda.s32EvalRoot = -INFINITO;
		else
			dbDatosBusqueda.s32EvalRoot = TABLAS;
	}
	else if (u32Legales == 1)
	{
		if (dbDatosBusqueda.eTipoBusqueda == TBU_PARTIDA && s32Prof > 3)
      dbDatosBusqueda.bAbortar = TRUE;
	}

	return(iMejor);
}
