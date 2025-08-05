
#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"
#include "Bitboards_inline.h"
#include "Funciones.h"

/*
 *
 * Reducciones
 *
 * Recibe: Puntero a la posición actual (después del movimiento), la jugada a realizar, la profundidad a la que se está analizando...
 *
 * Devuelve: Un entero con la variación de profundidad a aplicar
 *
 */
SINT32 Reducciones(TPosicion * pPos,
				   TJugada jug,
				   SINT32 s32Prof,
				   SINT32 s32Alfa,
				   UINT32 u32Legales,
				   BOOL bEsCutNode,
				   BOOL	bMejorando)
{
	assert(!Pos_GetJaqueado(pPos));   // El movimiento no da jaque (o habría sido extendido)

	SINT32 s32Red = 0; // 24/02/25 0.69 para permitir acumular reducciones (si no, con la LMR ya no se miran las demás aunque se activen)

	//
	// Condiciones de "no reducción"
	// La mayoría de las condiciones ya se han aplicado previamente, puesto que esta función sólo es invocada en la
	//  fase de "resto de jugadas + capturas perdedoras" y cuando no se ha extendido
	//
	if (s32Prof < 2)									// Nodo hoja
		return(0);
	if (u32Legales < 3)									// No reduzco las 2 primeras jugadas
		return(0);
	if (Jug_GetPromo(jug))								// 23/01/25 0.60 No reduzco promociones
		return(0);
	#if defined(BUS_RED_LEGALES_X_PROF)
		if (u32Legales < s32Prof)						// 0.81 - para que vayan apareciendo más jugadas conforme aumenta la profundidad
			return(0);
	#endif

	//
	// LMR
	//
	// Sabemos que no estamos valorando la jugada de la tabla hash ni los killers ni las capturas ganadoras,
	//  porque tras esas jugadas no llamamos a la función Reducciones(). Por lo tanto, nos basta la comprobación
	//  de que la jugada no es una de las dos primeras.
	// 
	//	A su manera, todos hacen lo de la tabla lmrRed con el doble logaritmo.
	//	Laser 1.7:
	//	Hakka rel3:		prof >= 3
	//	Renegade 1.1.9:	prof >= 3
	//	Astra 5.1:		prof >  1
	//  Alexandria 8;	prof >= 3				cutNode(+2)		quiet && wasPV(-1)		killer && quiet(-1)						!improving(+1) && quiet
	//  Superultra 2.1:	prof >= 3	PV(-1)						!quiet(-1)				killer(-1)			TT capture(+1)		!improving(+1)
	//	Stash 37:		prof >= 3	!PV(+1)		cutNode(+1)		!quiet(+1)				killer(-1)
	//  Zangdar 309:	prof >  2	PV(-1)																	TT capture(+1)		improving(-1)		opo_pocas_piezas(+1)
	//  Horsie 1.0:		prof >= 2	PV(-1)		cutNode(+2)		wasPV(-1)				killer(-1)								!improving(+1)
	//	Caissa 1.22:	prof >= 1	!PV(+)		cutNode(+)													TT capture(+)		!improving(+)		jaque(-)
	//	Myrddin 093:				PV(-1)												killer && quiet(-1)						improving(-1)		jaque(-1)
	//	Koivisto 9.2:	prof >  2	PV(-1)												killer (-1)								!improving(+1)		bajo_null_move(+1)		lejos de alfa(+)
	//  Obsidian 16:	prof >= 2	PV(-1)		cutNode(+2)		!quiet(+1)									TTprof>=prof(-1)	!improving(+1)		jaque(-1)				history(-)
	//	Cataphract 1.0:	prof >= 2	!PV(+1)		cutNode(+1)		quiet(-)
	//
	//	Yo:
	//		Modo 1: reduzco siempre 1
	//		Modo 2: modo 1 + red a partir de prof 7 y cut o no mejora
	//		Modo 3: modo 2 + red a partir de prof 17 y (cut o no mejora) y (legales >= prof + 5)
	#if defined(BUS_LMR)
		#if (BUS_LMR == 1)
			s32Red--;
		#elif (BUS_LMR == 2)
			s32Red--;
			if (s32Prof >= 7 && (bEsCutNode || !bMejorando))
				s32Red--;
		#elif (BUS_LMR == 3)
			s32Red--;
			if (s32Prof >= 7 && (bEsCutNode || !bMejorando))
				s32Red--;
			if (s32Prof >= 17 && (bEsCutNode || !bMejorando) && u32Legales >= (s32Prof + 5)) // OJO: probar con menos de 5
				s32Red--;
		#elif (BUS_LMR == 4)
			s32Red--;
			if (s32Prof >= 7 && (bEsCutNode || !bMejorando))
				s32Red--;
			if (s32Prof >= 12 && pPos->s32EvalMaterial != (pPos - 1)->s32EvalMaterial)
				s32Red--;
			if (s32Prof >= 17 && u32Legales >= s32Prof)
				s32Red--;
		#endif
	#endif // BUS_LMR

	//
	// Reducción de sosez
	//
	// Jugadas manifiestamente sosas, es decir, no son capturas ni jaques ni movimientos de peón ni nada que dé
	//  vidilla, y además mantienen una evaluación estable
	//
	// - No aplicar sosez si las casillas de origen o destino estaban / están atacadas
	// - No aplicar si el enemigo nos amenaza con ganar material
	// - Sozez profunda: puede ser que en esta jugada el cambio de eval esté un poquito por encima del umbral. En ese
	//    caso, miro más jugadas hacia atrás para ver la evolución de la "variante sosa"
	//
	#if defined(RED_SOSEZ)
		if (pPos->s32EvalAmenaza || pPos->u32Fase >= FAS_FINAL)
			goto fin_RedSosez;
		if (abs(pPos->s32Eval + (pPos-1)->s32Eval) >= RED_SOSEZ)
		{
			if (abs(pPos->s32Eval + (pPos-1)->s32Eval) >= RED_SOSEZ * 2)
				goto fin_RedSosez;
			else
			{
				// Sosez profunda: la variación de eval está entre el umbral y dos veces el umbral
				if (abs(pPos->s32Eval - (pPos-2)->s32Eval) >= RED_SOSEZ * 2 || abs(pPos->s32Eval + (pPos-3)->s32Eval) >= RED_SOSEZ * 2)
					goto fin_RedSosez;
			}
		}
		// Llegados aquí, estamos dentro de los márgenes admisibles para que haya sosez. Es hora de hacer otras comprobaciones
		if (Pos_GetTurno(pPos) == BLANCAS)
		{
			UINT64 u64AtacadasB;

			// La seguridad del rey debe mantenerse dentro de los límites de la sosez
			if (abs(pPos->s32EvalReyB - (pPos-1)->s32EvalReyB) >= RED_SOSEZ)
				goto fin_RedSosez;
			// Ni la casilla desde, ni la hasta, deben estar atacadas antes ni después del movimiento
			if ((BB_Mask(Jug_GetDesde(jug)) | BB_Mask(Jug_GetHasta(jug))) &	(pPos->u64AtaquesB | (pPos-1)->u64AtaquesB))
				goto fin_RedSosez;
			// El siguiente switch es feo, pero mejora la velocidad al comprobar los mapas de ataques de forma particular
			switch (pPos->u32Fase)
			{
				case FAS_APERTURA:
				case FAS_MED_JUG_INI:
				case FAS_MED_JUG_FIN:
				case FAS_FINAL:
				case FAS_FINAL_SIN_PEONES:
				case FAS_FINAL_TORRES_Y_MENORES:
					u64AtacadasB = pPos->u64TodasB & (pPos->u64AtaquesPN | pPos->u64AtaquesCN | pPos->u64AtaquesAN | pPos->u64AtaquesTN | pPos->u64AtaquesDN);
					if (u64AtacadasB != ((pPos-1)->u64TodasB & ((pPos-1)->u64AtaquesPN | (pPos-1)->u64AtaquesCN | (pPos-1)->u64AtaquesAN | (pPos-1)->u64AtaquesTN | (pPos-1)->u64AtaquesDN)))
					   goto fin_RedSosez;
					break;
				case FAS_FINAL_PESADAS:
					u64AtacadasB = pPos->u64TodasB & (pPos->u64AtaquesDN | pPos->u64AtaquesTN);
					if (u64AtacadasB && u64AtacadasB != ((pPos-1)->u64TodasB & ((pPos-1)->u64AtaquesDN | (pPos-1)->u64AtaquesTN)))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_DAMAS:
					u64AtacadasB = pPos->u64TodasB & pPos->u64AtaquesDN;
					if (u64AtacadasB && u64AtacadasB != ((pPos-1)->u64TodasB & (pPos-1)->u64AtaquesDN))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_TORRES:
					u64AtacadasB = pPos->u64TodasB & pPos->u64AtaquesTN;
					if (u64AtacadasB && u64AtacadasB != ((pPos-1)->u64TodasB & (pPos-1)->u64AtaquesTN))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_MENORES:
					u64AtacadasB = pPos->u64TodasB & (pPos->u64AtaquesAN | pPos->u64AtaquesCN);
					if (u64AtacadasB && u64AtacadasB != ((pPos-1)->u64TodasB & ((pPos-1)->u64AtaquesAN | (pPos-1)->u64AtaquesCN)))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_ALFILES:
					u64AtacadasB = pPos->u64TodasB & pPos->u64AtaquesAN;
					if (u64AtacadasB && u64AtacadasB != ((pPos-1)->u64TodasB & (pPos-1)->u64AtaquesAN))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_CABALLOS:
					u64AtacadasB = pPos->u64TodasB & pPos->u64AtaquesCN;
					if (u64AtacadasB && u64AtacadasB != ((pPos-1)->u64TodasB & (pPos-1)->u64AtaquesCN))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_PEONES:
					break;
				default:
					assert(0);
			}
		}
		else
		{
			UINT64 u64AtacadasN;

			// La seguridad del rey debe mantenerse dentro de los límites de la sosez
			if (abs(pPos->s32EvalReyN - (pPos-1)->s32EvalReyN) >= RED_SOSEZ)
				goto fin_RedSosez;
			// Ni la casilla desde, ni la hasta, deben estar atacadas antes ni después del movimiento
			if ((BB_Mask(Jug_GetDesde(jug)) | BB_Mask(Jug_GetHasta(jug))) & (pPos->u64AtaquesN | (pPos-1)->u64AtaquesN))
				goto fin_RedSosez;
			// El siguiente switch es feo, pero mejora la velocidad al comprobar los mapas de ataques de forma particular
			switch (pPos->u32Fase)
			{
				case FAS_APERTURA:
				case FAS_MED_JUG_INI:
				case FAS_MED_JUG_FIN:
				case FAS_FINAL:
				case FAS_FINAL_SIN_PEONES:
				case FAS_FINAL_TORRES_Y_MENORES:
					u64AtacadasN = pPos->u64TodasN & (pPos->u64AtaquesPB | pPos->u64AtaquesCB | pPos->u64AtaquesAB | pPos->u64AtaquesTB | pPos->u64AtaquesDB);
					if (u64AtacadasN != ((pPos-1)->u64TodasN & ((pPos-1)->u64AtaquesPB | (pPos-1)->u64AtaquesCB | (pPos-1)->u64AtaquesAB | (pPos-1)->u64AtaquesTB | (pPos-1)->u64AtaquesDB)))
					   goto fin_RedSosez;
					break;
				case FAS_FINAL_PESADAS:
					u64AtacadasN = pPos->u64TodasN & (pPos->u64AtaquesDB | pPos->u64AtaquesTB);
					if (u64AtacadasN && u64AtacadasN != ((pPos-1)->u64TodasN & ((pPos-1)->u64AtaquesDB | (pPos-1)->u64AtaquesTB)))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_DAMAS:
					u64AtacadasN = pPos->u64TodasN & pPos->u64AtaquesDB;
					if (u64AtacadasN && u64AtacadasN != ((pPos-1)->u64TodasN & (pPos-1)->u64AtaquesDB))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_TORRES:
					u64AtacadasN = pPos->u64TodasN & pPos->u64AtaquesTB;
					if (u64AtacadasN && u64AtacadasN != ((pPos-1)->u64TodasN & (pPos-1)->u64AtaquesTB))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_MENORES:
					u64AtacadasN = pPos->u64TodasN & (pPos->u64AtaquesAB | pPos->u64AtaquesCB);
					if (u64AtacadasN && u64AtacadasN != ((pPos-1)->u64TodasN & ((pPos-1)->u64AtaquesAB | (pPos-1)->u64AtaquesCB)))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_ALFILES:
					u64AtacadasN = pPos->u64TodasN & pPos->u64AtaquesAB;
					if (u64AtacadasN && u64AtacadasN != ((pPos-1)->u64TodasN & (pPos-1)->u64AtaquesAB))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_CABALLOS:
					u64AtacadasN = pPos->u64TodasN & pPos->u64AtaquesCB;
					if (u64AtacadasN && u64AtacadasN != ((pPos-1)->u64TodasN & (pPos-1)->u64AtaquesCB))
						goto fin_RedSosez;
					break;
				case FAS_FINAL_PEONES:
					break;
				default:
					assert(0);
			}
		}
		//
		// Superadas todas las pruebas, reducimos
		//
		dbDatosBusqueda.u32ReduccionesSosez++;
		s32Red--;
fin_RedSosez:
	#endif


	//
	// Reducción acumulada
	//
	return(s32Red);
}

