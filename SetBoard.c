/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación la función de
	inicialización del tablero
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Constantes.h"
#include "Variables.h"
#include <ctype.h>
#include "Bitboards_inline.h"
#include "Inline.h"
#include "Funciones.h"
#include "nnue.h"
#include <math.h>

BOOL SetBoard(TPosicion * pPos,char * szFEN)
{
	UINT32 u32Casilla;

	pPos->u64PeonesB = BB_TABLEROVACIO;
	pPos->u64CaballosB = BB_TABLEROVACIO;
	pPos->u64AlfilesB = BB_TABLEROVACIO;
	pPos->u64TorresB = BB_TABLEROVACIO;
	pPos->u64DamasB = BB_TABLEROVACIO;
	pPos->u64PeonesN = BB_TABLEROVACIO;
	pPos->u64CaballosN = BB_TABLEROVACIO;
	pPos->u64AlfilesN = BB_TABLEROVACIO;
	pPos->u64TorresN = BB_TABLEROVACIO;
	pPos->u64DamasN = BB_TABLEROVACIO;
	pPos->u64TodasB = BB_TABLEROVACIO;
	pPos->u64TodasN = BB_TABLEROVACIO;

	pPos->u64HashSignature = 0;

	pPos->pPunteroRepeticion = pPos;
	pPos->pListaJug = aPilaJugadas;

	pPos->u8Cincuenta = 0;
	pPos->u8Enroques = 0;
	pPos->u8AlPaso = TAB_ALPASOIMPOSIBLE;

	pPos->u8PosReyB = 65;
	pPos->u8PosReyN = 65;
	pPos->u8NumPiezasB = 0;
	pPos->u8NumPiezasN = 0;

	pPos->u8NumPeonesB = 0;
	pPos->u8NumPeonesN = 0;

#if defined(DEBUG_NNUE)
	char szNNUE[255];
	strcpy(szNNUE, szFEN);
#endif

	for (u32Casilla = 0;u32Casilla < 64;szFEN++)
	{
		if (*szFEN == '\0')
		{ 
			puts("Error en FEN: cadena incompleta");
			return(FALSE);
		}

		if(isdigit(*szFEN)) // Si es un número, nos saltamos esas casillas
		{
			u32Casilla += *szFEN - ('1' - 1);
			continue;
		}

		switch(*szFEN)
		{
			case 'K':
				pPos->u8PosReyB = (UINT8)u32Casilla;
				pPos->u64TodasB |= BB_Mask(u32Casilla);
				break;
			case 'k':
				pPos->u8PosReyN = (UINT8)u32Casilla;
				pPos->u64TodasN |= BB_Mask(u32Casilla);
				break;
			case 'Q':
				pPos->u64DamasB |= BB_Mask(u32Casilla);
				pPos->u8NumPiezasB++;
				pPos->u64TodasB |= BB_Mask(u32Casilla);
				break;
			case 'q':
				pPos->u64DamasN |= BB_Mask(u32Casilla);
				pPos->u8NumPiezasN++;
				pPos->u64TodasN |= BB_Mask(u32Casilla);
				break;
			case 'R':
				pPos->u64TorresB |= BB_Mask(u32Casilla);
				pPos->u8NumPiezasB++;
				pPos->u64TodasB |= BB_Mask(u32Casilla);
				break;
			case 'r':
				pPos->u64TorresN |= BB_Mask(u32Casilla);
				pPos->u8NumPiezasN++;
				pPos->u64TodasN |= BB_Mask(u32Casilla);
				break;
			case 'B':
				pPos->u64AlfilesB |= BB_Mask(u32Casilla);
				pPos->u8NumPiezasB++;
				pPos->u64TodasB |= BB_Mask(u32Casilla);
				break;
			case 'b':
				pPos->u64AlfilesN |= BB_Mask(u32Casilla);
				pPos->u8NumPiezasN++;
				pPos->u64TodasN |= BB_Mask(u32Casilla);
				break;
			case 'N':
				pPos->u64CaballosB |= BB_Mask(u32Casilla);
				pPos->u8NumPiezasB++;
				pPos->u64TodasB |= BB_Mask(u32Casilla);
				break;
			case 'n':
				pPos->u64CaballosN |= BB_Mask(u32Casilla);
				pPos->u8NumPiezasN++;
				pPos->u64TodasN |= BB_Mask(u32Casilla);
				break;
			case 'P':
				pPos->u64PeonesB |= BB_Mask(u32Casilla);
				pPos->u8NumPeonesB++;
				pPos->u64TodasB |= BB_Mask(u32Casilla);
				break;
			case 'p':
				pPos->u64PeonesN |= BB_Mask(u32Casilla);
				pPos->u8NumPeonesN++;
				pPos->u64TodasN |= BB_Mask(u32Casilla);
				break;
			case '/':
			case ' ':
			case ';':
				u32Casilla--;
				break;
			default:
				puts("Error en FEN: línea incorrecta");
				return(FALSE);
		}
		u32Casilla++;
	}

	while (*szFEN == ' ') szFEN++;
	switch(tolower(*szFEN))
	{
		case 'w':
			Pos_SetTurnoB(pPos);
			break;
		case 'b':
			Pos_SetTurnoN(pPos);
			break;
		default:
			puts("Error en FEN: color en turno debe ser 'w' o 'b'");
			return(FALSE);
	}

	szFEN++;
	while (*szFEN == ' ') szFEN++;
	if (*szFEN == '\0')
	{
		// Todos los enroques que sean posibles, permitidos
		if (pPos->u8PosReyB == TAB_E1)
		{
			if (pPos->u64TorresB & BB_Mask(TAB_H1))
				Pos_SetCortoB(pPos);
			if (pPos->u64TorresB & BB_Mask(TAB_A1))
				Pos_SetLargoB(pPos);
		}
		if (pPos->u8PosReyN == TAB_E8)
		{
			if (pPos->u64TorresN & BB_Mask(TAB_H8))
				Pos_SetCortoN(pPos);
			if (pPos->u64TorresN & BB_Mask(TAB_A8))
				Pos_SetLargoN(pPos);
		}
	}
	else
	{
		UINT32 u32Mas = TRUE;
		while (u32Mas)
		{
			switch (*szFEN)
			{
				case 'K':
					Pos_SetCortoB(pPos);
					break;
				case 'Q':
					Pos_SetLargoB(pPos);
					break;
				case 'k':
					Pos_SetCortoN(pPos);
					break;
				case 'q':
					Pos_SetLargoN(pPos);
					break;
				default:
					u32Mas = FALSE;
					break;
			}
			szFEN++;
		}
	}
	// A ver si hay casilla al paso
	while (*szFEN == ' ') szFEN++;
	if (*szFEN != '\0' && *(szFEN+1) != '\0')
	{
		if (*szFEN >= 'a'
			&& *szFEN <= 'h'
			&& *(szFEN+1) >= '0'
			&& *(szFEN+1) <= '9'
		   )
		{
			UINT32 u32Col,u32Fila;

			u32Col = szFEN[0] - 'a';
			u32Fila = 8 - (szFEN[1] - '0');
			pPos->u8AlPaso = (UINT8)(u32Fila*8 + u32Col);
			szFEN++;
		}
	}
	szFEN++;
	// Aquí hay un número que no sé qué significa
	// Es el número de medias jugadas reversibles (contador de 50) XD
	while (*szFEN == ' ')
		szFEN++;
	while (*szFEN >= '0' && *szFEN <= '9')
		szFEN++;

	// A ver si hay número de jugada
	while (*szFEN == ' ')
		szFEN++;
	if (*szFEN != '\0')
	{
		UINT32 i = 1;
		UINT32 u32NumJug = 0;

		while (*szFEN >= '0'
			&& *szFEN <= '9')
		{
			u32NumJug *= i;
			u32NumJug += szFEN[0] - '0';
			szFEN++;
			i *= 10;
		}
		dbDatosBusqueda.u32NumPlyPartida = 2 * (u32NumJug - 1) + 1;
	}
	else
		dbDatosBusqueda.u32NumPlyPartida = 1;

	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		dbDatosBusqueda.u32NumPlyPartida--;
		if (Atacado(pPos,pPos->u8PosReyB,NEGRAS))
			Pos_SetJaqueado(pPos);
		else
			Pos_SetNoJaqueado(pPos);
	}
	else
	{
		if (Atacado(pPos,pPos->u8PosReyN,BLANCAS))
			Pos_SetJaqueado(pPos);
		else
			Pos_SetNoJaqueado(pPos);
	}

	dbDatosBusqueda.u32ColorAnubis = COLOR_INDEFINIDO;
	dbDatosBusqueda.s32EvalRoot = 0;
	as32PilaEvaluaciones[0] = 0;

	BorrarTablasHash();

	CalcularHash(pPos);

	ComputarBB(pPos);
	pPos->s32EvalMaterial = EvaluarMaterial(pPos);
	pPos->s32Eval = Evaluar(pPos);

#if defined(DEBUG_NNUE)
	//float evalfromfen = nnue_from_fen(&szNNUE, pPos);
	//if (Pos_GetTurno(pPos) == NEGRAS)
	//	evalfromfen = -evalfromfen;
	//printf("Eval from FEN: %d\n", (int)roundf(evalfromfen));
	//SINT32 evalold = Evaluar_old(pPos);
	//printf("Eval Anubis: %d\n", evalold);
	printf("Eval NNUE: %d\n", pPos->s32Eval);
#endif
	return(TRUE);
}