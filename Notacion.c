/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de las funciones
	relacionadas con conversiones desde y
	hacia las distintas notaciones soportadas
	(SAN, Algebraica)
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Constantes.h"
#include "Inline.h"
#include "Funciones.h"
#include "Bitboards_inline.h"
#include <string.h>

#if !defined(MismaColumna)
#define MismaColumna(i,j)		(((i)%8) == ((j)%8))
#endif

/*
	*****************************************************
	*													*
	*	Algebraica2Jug									*
	*													*
	*													*
	*	Recibe: Una jugada en notación algebraica		*
	*			(entiendo por algebraica la				*
	*			algebraica extendida, la otra es		*
	*			SAN), en forma de cadena de texto		*
	*													*
	*	Devuelve: Una jugada en el formato interno		*
	*			  de Anubis								*
	*													*
	*	Descripción: Traduce de notación algebraíca		*
	*				 extendida al formato interno		*
	*													*
	*****************************************************
*/
TJugada	Algebraica2Jug(char * szJugada)
{
	UINT32	u32Col;
	UINT32	u32Fil;
	UINT32	u32Desde;
	UINT32	u32Hasta;
	TJugada jug = JUGADA_NULA;
	char	* szPromo;

	u32Col = szJugada[0] - 'a';
	u32Fil = 8 - (szJugada[1] - '0');
	u32Desde = u32Fil * 8 + u32Col;
	Jug_SetDesde(&jug,(UINT16)u32Desde);

	u32Col = szJugada[2] - 'a';
	u32Fil = 8 - (szJugada[3] - '0');
	u32Hasta = u32Fil * 8 + u32Col;
	Jug_SetHasta(&jug,(UINT16)u32Hasta);

	if (szJugada[4] == '=')
		szPromo = &szJugada[5];
	else
		szPromo = &szJugada[4];

	switch (*szPromo)
	{
		case 'Q':
		case 'q':
			Jug_SetPromo(&jug,DB);
			break;
		case 'R':
		case 'r':
			Jug_SetPromo(&jug,TB);
			break;
		case 'N':
		case 'n':
			Jug_SetPromo(&jug,CB);
			break;
		case 'B':
		case 'b':
			Jug_SetPromo(&jug,AB);
			break;
	}

	return(jug);
}

/*
 *
 * Jug2Algebraica
 *
 */
void Jug2Algebraica(TJugada jug, char * szJugada)
{
	static char * aszPieza[15] = {" ", "P", "N", "B", "R", "Q", "K", " ", " ", "P", "N", "B", "R", "Q", "K"};

	if (Jug_GetPromo(jug))
		sprintf(szJugada, "%s%s%s\0", aszCuadros[Jug_GetDesde(jug)], aszCuadros[Jug_GetHasta(jug)], aszPieza[Jug_GetPromo(jug)]);
	else
		sprintf(szJugada,"%s%s\0", aszCuadros[Jug_GetDesde(jug)], aszCuadros[Jug_GetHasta(jug)]);
}

/*
 * 
 * 31/01/25 0.61 Jug2SAN
 * 
 * Llamamos a esta función después de hacer la jugada, luego la posición de las piezas es la de pPos-1 y el jaque es pPos
 * 
 */
void Jug2SAN(TPosicion * pPos, TJugada jug, char * szJugada)
{
	static char* aszPieza[15] = { " ", "P", "N", "B", "R", "Q", "K", " ", " ", "P", "N", "B", "R", "Q", "K" };
	UINT32 u32Pieza = PiezaEnCasilla(pPos-1, Jug_GetDesde(jug));
	BOOL bEsCaptura = PiezaEnCasilla(pPos-1, Jug_GetHasta(jug)) != VACIO;

	if (u32Pieza == PB || u32Pieza == PN)
	{
		if (MismaColumna(Jug_GetDesde(jug), Jug_GetHasta(jug)))
			sprintf(szJugada, "%s\0", aszCuadros[Jug_GetHasta(jug)]);
		else
			sprintf(szJugada, "%c%s\0", aszCuadros[Jug_GetDesde(jug)][0], aszCuadros[Jug_GetHasta(jug)]);

		if (Jug_GetPromo(jug))
			strcat(szJugada, aszPieza[Jug_GetPromo(jug)]);
	}
	else
	{
		if ((u32Pieza == RB || u32Pieza == RN) && ((Jug_GetDesde(jug) == TAB_E1 && Jug_GetHasta(jug) == TAB_G1) || (Jug_GetDesde(jug) == TAB_E8 && Jug_GetHasta(jug) == TAB_G8)))
			sprintf(szJugada, "0-0\0");
		else if ((u32Pieza == RB || u32Pieza == RN) && ((Jug_GetDesde(jug) == TAB_E1 && Jug_GetHasta(jug) == TAB_C1) || (Jug_GetDesde(jug) == TAB_E8 && Jug_GetHasta(jug) == TAB_C8)))
				sprintf(szJugada, "0-0-0\0");
		else if (bEsCaptura)
			sprintf(szJugada, "%sx%s\0", aszPieza[u32Pieza], aszCuadros[Jug_GetHasta(jug)]);
		else
			sprintf(szJugada, "%s%s\0", aszPieza[u32Pieza], aszCuadros[Jug_GetHasta(jug)]);
	}
	if (Pos_GetJaqueado(pPos))
		strcat(szJugada, "+");
}

/*
 *
 * ExtraerPVdeArray
 *
 * Recibe: puntero a la posición actual, puntero a carácter donde devuelve la PV
 *
 * Descripción: Lee el array de PV que se construye durante la búsqueda y lo convierte en texto. Después, trata de
 *  sacar más jugadas de la tabla hash
 *
 */
void ExtraerPVdeArray(TPosicion * pPos,char * szPV)
{
	static TPosicion  aPos[2 * MAX_PLIES];
	char			  szJugada[10];
	UINT32			  i, j;
	TPosicion *       pPosTemp;
	size_t			  len;

	aPos[0] = *pPos;
	pPosTemp = &aPos[0];
	szPV[0] = '\0';

	// 1. Array que se va construyendo durante la búsqueda
	for (i = 0; i < MAX_PLIES; i++)
	{
		if (Jug_GetEsNula(dbDatosBusqueda.ajugPV[0][i]))
			break;
		if (!JugadaCorrecta(pPosTemp, dbDatosBusqueda.ajugPV[0][i]))
			break;
		if (!Mover(pPosTemp, &(dbDatosBusqueda.ajugPV[0][i])))
			break;
		dbDatosBusqueda.u32NumNodos--; // Lo incrementa Mover

		pPosTemp++;
		// 31/01/25 0.61 Cambio algebraica por SAN
		//Jug2Algebraica(dbDatosBusqueda.ajugPV[0][i], szJugada);
		Jug2SAN(pPosTemp, dbDatosBusqueda.ajugPV[0][i], szJugada);
		strcat(szPV, szJugada);
		strcat(szPV," ");

		if (Pos_GetTurno(pPosTemp) == BLANCAS)
			strcat(szPV, ". ");
	}
	len = strlen(szPV);
	if (len >= 2)
	{
		if (strcmp(szPV + len - 2, "* ") == 0 || strcmp(szPV + len - 2, ". ") == 0)
			szPV[len - 2] = '\0';  // Eliminamos los últimos dos caracteres
	}
	strcat(szPV, "* ");

	// 2. Tabla hash
	for (j = 0; j < 30 && i < MAX_PLIES; j++, i++)
	{
		TNodoHash * pNodoHash = NULL;
		ConsultarHash(pPosTemp, 500, -INFINITO, INFINITO, & pNodoHash);
		TJugada jug = (pNodoHash == NULL ? JUGADA_NULA
										 : pNodoHash->jug);

		if (Jug_GetEsNula(jug))
			break;
		if (!JugadaCorrecta(pPosTemp, jug))
			break;
		if (!Mover(pPosTemp, &jug))
			break;
		dbDatosBusqueda.u32NumNodos--;

		pPosTemp++;
		// 31/01/25 0.61 Cambio algebraica por SAN
		//Jug2Algebraica(pNodoHash->jug, szJugada);
		Jug2SAN(pPosTemp, jug, szJugada);
		strcat(szPV, szJugada);
		strcat(szPV," ");

		if (Pos_GetTurno(pPosTemp) == BLANCAS)
			strcat(szPV, ". ");

		// 14/12/24 - Para tener una jugada que ponderar (de hash) en caso de no tenerla de PV
		if (j == 0 && Jug_GetEsNula(dbDatosBusqueda.ajugPV[0][1]))
			dbDatosBusqueda.ajugPV[0][1] = jug;

		// 24/12/24 - Para no repetir muchas jugadas de la tabla hash (no puedo llamar a SegundaRepetición porque uso otra pila)
		if (pPosTemp > &aPos[0])
		{
			for (TPosicion* p = pPosTemp - 1; p > &aPos[0]; p--)
			{
				if ((p->u64HashSignature == pPosTemp->u64HashSignature) && (Pos_GetTurno(p) == Pos_GetTurno(pPosTemp)))
					return;
			}
		}
	}

	len = strlen(szPV);
	if (len >= 2)
	{
		if (strcmp(szPV + len - 2, "* ") == 0 || strcmp(szPV + len - 2, ". ") == 0)
			szPV[len - 2] = '\0';  // Eliminamos los últimos dos caracteres
	}
	len = strlen(szPV);
	if (len >= 2)
	{
		if (strcmp(szPV + len - 2, "* ") == 0 || strcmp(szPV + len - 2, ". ") == 0)
			szPV[len - 2] = '\0';  // Eliminamos los últimos dos caracteres
	}
}

