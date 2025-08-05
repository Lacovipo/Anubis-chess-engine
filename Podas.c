
#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"
#include "Constantes.h"
#include "Variables.h"
#include "Funciones.h"
#include "Bitboards_inline.h"

/*
 * PodarContraBeta
 *
 * Recibe: Puntero a la posición actual (sin hacer ningún movimiento), ply actual, profundidad restante, alfa,
 *  beta y el valor original de alfa antes de ninguna rectificación que haya podido ocurrir durante	la búsqueda
 *  (incrementos)
 *
 * Devuelve: TRUE si hay que podar completamente el subárbol o FALSE en caso contrario
 *
 * Descripción: Dependiendo de una serie de condiciones, decide qué método de poda corresponde intentar, lo intenta y devuelve si se debe podar o no
 *
 */
BOOL PodarContraBeta(TPosicion * pPos, SINT32 s32Ply, SINT32 s32Prof, SINT32 s32Beta, BOOL bMejorando)
{
	SINT32 s32NullProf;
	SINT32 s32Val;

	// Condiciones de NO poda
	if (Pos_GetJaqueado(pPos))
		return(FALSE);
	if (pPos->u32MovExclu != 0)
		return(FALSE);
	if (abs(s32Beta) > VICTORIA)
		return(FALSE);
	if (!Pos_GetIntentarNull(pPos))
		return(FALSE);
	if (pPos->s32Eval < s32Beta)
		return(FALSE);
	// Aquí distingo: si tengo NM_ADAP con verif, no limito por fase; si no tengo verif, evito los finales más propensos a zugzwang
	#if (BUS_NM_ADAP == 3)
		// El modo 3 lleva verificación -> voy directo a NM
		if (pPos->u32Fase >= FAS_FINAL_MENORES)
			goto fin_Static;
	#else
		if (pPos->u32Fase >= FAS_FINAL_MENORES)
			return(FALSE);
	#endif

	//
	// Poda JC
	//
	// Nota: es como la reverse futility pruning que hacen algunos programas
	//			Obsidian 16:	prof <=11  &&  eval >= beta + max(87 * (prof - improving), 22))
	//			Patricia 4		prof <=10  &&  eval >= beta +  84 * (prof - improving)
	//			Alexandria 8:	prof < 10  &&  eval >= beta +  88 * prof - 62 * improving - 85 * (prof >= 4 no jug hash)
	//			Astra 5.1:		prof < 10  &&  eval >= beta +  99 * prof - 92 * improving
	//			Arasan 25:		prof <= 6  &&  eval >= beta + max(32, 96 * (prof - improving))
	//			Sirius 8.0:		prof <= 8  &&  eval >= beta + (improving ? 40 : 86) * prof + stack[-1].histScore / 410
	//			Rodent IV:		prof <= 7  &&  eval >  beta + (175 - 50 * improving) * prof
	//			Horsie 1.0:		prof <= 6  &&  eval >= beta +  47 * (prof - improving)
	//			Starzix 6:		prof <= 7  &&  eval >= beta +  71 * (prof - improving)
	//			Perseus 1.0:	prof <= 8  &&  eval >= beta +  99 * (prof - improving)
	//			Altair 7.1.5:	prof <= 7  &&  eval >= beta + 110 * (prof - improving)
	//			PlentiChess:	prof <  8  &&  eval >= beta +  97 * (prof - improving)
	//			Cadie 2.1:		prof <= 7  &&  eval >  beta + 100 * (prof - improving)
	//			Schoe...:		prof <= 5  &&  eval >= beta +  80 * (prof - improving)
	//			Superultra 2.1: prof <= 8  &&  eval >= beta +  77 * (prof - improving)
	//			Renegade 1.1.9:	prof <= 7  &&  eval  > beta +  90 * (prof - improving)
	//			Spaghet 3.1.0:	prof <= 6  &&  eval >= beta +  80 * (prof - improving)
	//		Peacekeeper 3.01:	prof <  6  &&  eval >= beta +  91 * (prof - improving) + 35
	//			Koivisto 9.2:	prof <= 7  &&  eval >= beta +  68 * (prof - (improving && !enemyThreats))
	//			Weiss 2.1:		prof <  7  &&  eval >= beta +  92 * (prof - improving) + (ss - 1)->histScore / 128 
	//			Quanticade Q:	prof <= 7  &&  eval >= beta +  56 * prof 
	//			Senpai 2.0:		prof <= 2  &&  eval >= beta + 100 * prof
	//			Kobra 2.0		prof <  3  &&  eval >= beta + 171 * prof
	//			Xiphos 0.6.1:	prof <= 5  &&  eval >= beta +  80 * prof
	//			Laser 1.7:		prof <= 6  &&  eval >= beta +  70 * prof
	//			Zangdar 309:	prof <= 6  &&  eval >= beta +  70 * prof
	//			Prelude 2.0:	prof <  7  &&  eval >= beta + 100 * prof
	//			Arcanum 2.4:	prof <  9  &&  eval >= beta + 300 * prof
	//			RukChess:		prof <= 5  &&  eval >= beta +  50 * prof + 100
	//			Hakka rel 3:	prof <= 3  &&  eval >= beta + 150 * prof + 100
	//			Stash 37:		prof <= 7  &&  eval >= beta +  86 * prof - 79 * improving
	//			Cataphract 1.0:	prof <= 9  &&  eval >= beta + 120 * prof - 70 * improving
	//			Demolito 2025:	prof <= 6  &&  eval >= beta + {0, 130, 264, 410, 510, 672, 840}
	//			Fire 10:		prof <  7  &&  eval >= beta + {0, 112, 243, 376, 510, 646, 784}
	//
	//			Yo (modo 1):	prof <= 4  &&  eval >  beta + 100 * prof + amenaza
	//						 && prof <= ply / 3 -->	si ply  <  3 -> prof <= 0
	//											-->	si ply  <  6 -> prof <= 1
	//											-->	si ply  <  9 -> prof <= 2
	//											-->	si ply  < 12 -> prof <= 3
	//											-->	si ply >= 13 -> prof <= 4
	//									   
	#if defined(PODA_JC)
		#if (PODA_JC == 1)
			if (s32Prof <= 4
				&& s32Prof <= s32Ply / 3
				&& pPos->s32Eval - pPos->s32EvalAmenaza > s32Beta + (100 * s32Prof)
				&& pPos->s32Eval > 0)
			{
				return(TRUE);
			}
		#elif (PODA_JC == 2)
			if (s32Prof <= 5
				&& s32Prof <= s32Ply / 2
				&& pPos->s32Eval - pPos->s32EvalAmenaza > s32Beta + (100 * s32Prof)
				&& pPos->s32Eval > 0)
			{
				return(TRUE);
			}
		#elif (PODA_JC == 3)
			if (s32Prof <= 6
				&& s32Prof <= s32Ply / 2
				&& pPos->s32Eval - pPos->s32EvalAmenaza > s32Beta + (100 * max(s32Prof - bMejorando, 0))
				&& pPos->s32Eval > 0)
			{
				return(TRUE);
			}
		#endif
#endif

fin_Static:

	//
	// Null move
	//
	if (s32Prof < 3)
		goto fin_NM;
	#if defined(BUS_NM_NO_CREO_QUE_PUEDA_FH)
		// Condición de "no creo que pueda fallar high"
		// Si la eval está por debajo de beta-300 y no estoy atacando ninguna pieza rival, no intento null move
		// 13/12/24 - Dejo 2 comprobaciones: una con beta-300 sin condición de ataques, y otra con beta a secas, con condición de ataques
		if (pPos->s32Eval < s32Beta - 300)
			goto fin_NM;
		if (pPos->s32Eval < s32Beta)
		{
			if (Pos_GetTurno(pPos) == BLANCAS)
			{
				if ((pPos->u64AtaquesB & pPos->u64TodasN) == BB_TABLEROVACIO)
					goto fin_NM;
			}
			else
			{
				if ((pPos->u64AtaquesN & pPos->u64TodasB) == BB_TABLEROVACIO)
					goto fin_NM;
			}
		}
	#endif // BUS_NM_NO_CREO_QUE_PUEDA_FH

	MoverNull(pPos);

	#if defined BUS_NM_ADAP
		// SF 17:			R = 5 + prof / 3				+ min((eval - beta) / 202, 6)										// (peón == 208)			// >= 16
		// SF 17.1:			R = 5 + prof / 3				+ min((eval - beta) / 232, 6)										// (peón == 208)			// >= 16
		// Alexandria 8:	R = 4 + prof / 3				+ min((eval - beta) / 216, 3)																	// >= 15
		// Q Fenrir:		R = 4 + prof / 3				+ min((eval - beta) / 205, 6)																	// no verif
		// Berserk:			R = 4 + prof * 0.38				+ min((eval - beta) / 102, 4)																	// >= 15
		// Stash 37:		R = 3.5 + prof * 0.24			+ min((eval - beta) / 111, 5)																	// >  12
		// Laser 1.7:		R = 3 + prof / 4				+ min((eval - beta) / 128, 3)																	// >= 10
		// Zangdar 309:		R = 3 + prof / 4				+ min((eval - beta) / 128, 3)																	// no verif
		// Xiphos 0.6.1:	R = 3 + prof / 4				+ min((eval - beta) /  80, 3)																	// no verif
		// Renegade 1.1.9:	R = 3 + prof / 3				+ min((eval - beta) / 200, 3)																	// no verif
		// Perseus 1.0:		R = 3 + prof / 4				+ min((eval - beta) / 216, 3)																	// >  14
		// Sirius 8.0:		R = 5 + prof / 4				+ min((eval - beta) / 208, 4)																	// no verif
		// KF PB 050325:	R = 5 + prof / 3				+ min((eval - beta) / 235, 7)																	// >= 16
		// Astra 5.1:		R = 4 + prof / 3				+ min((eval - beta) / 215, 4)																	// no verif
		// Horsie 1.0:		R = 4 + prof / 4				+ min((eval - beta) / 181, 2)																	// >  15
		// PlentyChess:		R = 4 + prof / 3				+ min((eval - beta) / 186, 4)																	// >= 15
		// Patricia 4:		R = 4 + prof / 5				+ min((eval - beta) / 173, 3)																	// no verif
		// Superultra 2.1:	R = 3 + prof / 3				+ min((eval - beta) / 200, 3)																	// no verif
		// Caissa 1.22:		R = 3 + prof / 3				+ min((eval - beta) / 133, 3) + improving														// >= 10
		// Cataphract 1.0:	R = 3 + prof / 5				+ min((eval - beta) / 200, 2) + improving														// no verif
		// Obsidian 16:		R = 4 + prof / 4				+ min((eval - beta) / 147, 4) + ttMoveNoisy														// no verif
		// ElBurro:			R = 4 +							+ min((eval - beta) /  32, 3)																	// no verif
		// Tucano 12:		R = 5 + ((prof-4)/4)            + min((eval - beta) / 200, 3) + (move_is_quiet(last_move) ? 0 : 1)								// no verif
		// Arasan 25:		R = 4 + prof / (4 + 4 * lowmat) + min((eval - beta) / 200, 4) - (lowmat * prof)						// lowmat = (piezas <= 3)	// >=  6
		// Altair 7.1.5:	R = 5 + prof / 4				+ (eval-beta) / 400 [entre -1 y 3]																// >  15
		// Clover 8.1:		R = 4 + prof / 3				+ (eval-beta) / 134 + improving + jug hash es noisy												// no verif
		// RukChess:		R = 4 + prof / 6																												// no verif
		// Myrdin 093:		R = 4 + prof / 6																												// no verif
		// Hakka rel3:		R = 3 + prof / 6																												// no verif
		// Senpai 2.0:		R = 3 + prof / 4																												// no verif
		// Cadie 2.1:		R = 3 + prof / 3																												// no verif
		// Schoe...:		R = 2 + prof / 3																												// no verif
		// Starzix 6:		R = 4 + prof * 0.4																												// no verif
		// Rodent IV:		R = 3.2 + prof * 0.26																											// > 6 + R
		// Kobra 2.0:		R = 2.6 + prof / 5																												// no verif
		// Demolito 2025:	R = 3 + prof / 4				+ (eval >= beta + 167)																			// no verif
		//
		// Yo:
		//	Modo 1:			R = 3 + prof / 5																												// no verif
		//	Modo 2:			R = 3 + prof / 3				+ (eval - amenaza - beta) / 100 [entre 0 y 3]													// no verif
		//	Modo 3:			R = 3 + prof / 3				+ (eval - amenaza - beta) / 100 [entre 0 y 3]													// >= 12
		// 
		s32NullProf = s32Prof - 3;
		if (s32NullProf > 0)
		{
			#if (BUS_NM_ADAP == 1)
				s32NullProf -= s32Prof / 5;
			#elif (BUS_NM_ADAP == 2 || BUS_NM_ADAP == 3)
				int iEvalMenosBeta = pPos->s32Eval - pPos->s32EvalAmenaza - s32Beta;
				s32NullProf -= s32Prof / 3;
				if (iEvalMenosBeta >= VAL_PEON)
					s32NullProf -= min(iEvalMenosBeta / VAL_PEON, 3);
				// Idea: si bMejorando -> prof--;
			#endif
		}
	#endif

	// Buscar
	if (s32NullProf <= 0)
		s32Val = -QSConJaques(pPos + 1, -s32Beta, -s32Beta + 1, s32Ply + 1, QS_PROF_JAQUES);
	else
		s32Val = -AlfaBeta(pPos + 1, -s32Beta, -s32Beta + 1, s32NullProf, s32Ply + 1, FALSE);
	if (dbDatosBusqueda.bAbortar)
		return(FALSE);

	#if (BUS_NM_ADAP == 1 || BUS_NM_ADAP == 2)
		if (s32Val >= s32Beta)
			return(TRUE);
	#elif (BUS_NM_ADAP == 3)
		// El modo 3 lleva verificación
		if (s32Prof >= 12 && s32Val >= s32Beta)
			s32Val = AlfaBeta(pPos, s32Beta - 1, s32Beta, s32NullProf, s32Ply, FALSE);

		if (s32Val >= s32Beta)
			return(TRUE);	// Idea: devolver aquí s32Val para hacer fail soft
	#endif

	// Si me amenazan mate, no vuelvo a intentar null move aquí
	if (Bus_ReciboMatePronto(s32Val))
		Pos_SetNoIntentarNull(pPos);

fin_NM:

	return(FALSE);
}

/*
 * PodarFutility
 *
 * Recibe: Puntero a la posición antes del movimiento, profundidad restante y alfa
 *
 * Devuelve: TRUE si se puede podar completamente el subárbol que cuelga de este movimiento. FALSE en caso contrario
 *
 *		Starzix 6		[sin hacer jug]: prof <=  7  &&  !enjaque(antes)  &&  alfa no mate		&&  alfa >  eval(antes) + 160 + max(lmrprof + mejorando, 0) * 165
 *		Altair 7.5.1	[sin hacer jug]: prof <=  7												&&  alfa >= eval(antes) +  70 + (prof - !mejorando) * 170
 *		Senpai 2.0		[sin hacer jug]: prof <=  4  &&  !enjaque(antes)						&&  alfa >= eval(antes)       + prof * 60
 *		Hakka rel 3		[sin hacer jug]: prof <=  4  &&  !enjaque(antes)  &&  alfa no mate		&&  alfa >= eval(antes) + 100 + prof * 25
 *		Renegade 1.1.9	[sin hacer jug]: prof <=  5  &&						 beta no mate		&&  alfa >  eval(antes) +  30 + prof * 100
 *		Perseus 1.0		[sin hacer jug]: prof <=  8  &&  !enjaque(antes)	 &&  alfa no mate   &&  alfa >= eval(antes) + 241 + prof *  54
 *		Prelude 2.0		[sin hacer jug]: prof <   6	 &&  !enjaque(antes)	 &&  alfa no mate   &&  alfa >  eval(antes) + 100 + prof *  80
 *		Clover 8.1		[sin hacer jug]: prof <= 10	 &&  !enjaque(antes)						&&	alfa >= eval(antes) + 96  + prof *  97
 *		Patricia 4		[sin hacer jug]: prof <   9  &&  !enjaque(antes)						&&  alfa >  eval(antes) + 100 + prof * 115
 *		Obsidian 16		[sin hacer jug]: prof <= 10  &&  !enjaque(antes)						&&  alfa >= eval(antes) + 159 + prof * 153
 *
 *		Yo:				[con jug hecha]: prof <=  7  &&  !enjaque(despu)						&&  alfa >= eval(antes) + 100 + prof * 25 + variables por fase
 */
#if defined(PODA_FUTIL)
BOOL PodarFutility(TPosicion * pPos, SINT32 s32Prof, SINT32 s32Alfa)
{
	UINT32 u32Fase;
	SINT32 s32Margen;

	if (s32Prof > 7)
		return(FALSE);
	if (Pos_GetJaqueado(pPos))
		return(FALSE);
	if (Pos_GetJaqueado(pPos+1))
		return(FALSE);
	if (Pos_GetCaptura(pPos+1))
		return(FALSE);

	s32Margen = 100 + 25 * s32Prof;

	// Acciones particulares según la fase de la partida
	u32Fase = (pPos+1)->u32Fase;
	switch (u32Fase)
	{
		case FAS_APERTURA:
		case FAS_MED_JUG_INI:
			break;
		case FAS_MED_JUG_FIN:
			s32Margen += 50;
			break;
		case FAS_FINAL:
		case FAS_FINAL_PESADAS:
		case FAS_FINAL_DAMAS:
		case FAS_FINAL_TORRES_Y_MENORES:
		case FAS_FINAL_TORRES:
			s32Margen += 100;
			break;
		case FAS_FINAL_MENORES:
		case FAS_FINAL_ALFILES:
		case FAS_FINAL_CABALLOS:
			s32Margen += 150;
			break;
		case FAS_FINAL_PEONES:
			return(FALSE);
		case FAS_FINAL_SIN_PEONES:
			break;
		default:
			assert(0);
	}

	// ¡¡ Atención !! pPos apunta a la posición actual del nodo, es decir _antes_ del movimiento, luego a mí me interesa la eval de la posición posterior, pero con signo invertido
	int iEval;
	if ((pPos+1)->s32Eval != NO_EVAL)
		iEval = -(pPos + 1)->s32Eval;
	else
		iEval = pPos->s32Eval;

	if (s32Alfa < iEval + s32Margen)
		return(FALSE);

	// Todas las contraindicaciones superadas: podamos
	return(TRUE);
}
#endif

/*
 * PodarLMP
 * 
 * (antes de mover)
 * 
 *			Starzix 6:		prof <= 6  &&  legales >= 2 + espv + enjaque(antes) + prof * prof * 1.15	-ignorando pv y jaque-	:	(2,  3,  7, 12, 20, 31, 43)
 *			Hakka rel3:		prof <= 4  &&  legales >= round(2.98484 + pow(prof, 1.74716))										:	(3,  4,  6, 10, 14)
 *			Renegade 1.1.9:	prof <  5  &&  legales >  3 + prof * (prof - !improving)					-ignorando improving-	:	(3,  4,  7, 10, 19)
 *			Sirius 8.0:		prof <= 10 &&  legales >= 2 + prof * prof / (improving ? 1 : 2)				-ignorando improving-	:	(2,  2,  4,  6, 10, 14, 20, 26, 34, 42, 52)
 *			Altair 7.1.5:	prof <= 4  &&  legales >= prof * 10																	:	(0, 10, 20, 30, 40)
 *			Prelude 2.0:				   legales >= 6 + prof * prof															:	(0,  7, 10, 15, 22, 31, 42, 55...)
 *			Clover 8.1:		prof <= 7  &&  legales >= (1 + prof * prof) / (2 - improving)				-ignorando improving-	:	(0,  1,  2,  5,  8, 13, 18, 25)
 *			Patricia 4:		prof <  6  &&  legales >= (2 + prof * prof) / (2 - improving)				-ignorando improving-	:	(2,  3,  6, 11, 18, 27)
 *		PeaceKeeper 3.01:	prof <  8  &&  legales >= (2 + prof * prof) * (improving + 1))				-ignorando improving-	:	(0,  3,  6, 11, 18, 27, 38, 51)
 *			Demolito 2025:	prof <= 4  &&  legales >= 3 * prof + 2 * improving														(0,  3,  6,  9, 12)
 *			Obsidian 16:				   legales >= (3 + prof * prof) / (2 - improving)				-ignorando improving-	:	(1,  2,	 3,  6,  9, 14, 19, 26...)
 *
 *  Mucha gente utiliza el improving y ahora tengo una eval de garantías que me permitiría usarlo a mí también
 */
#if defined(PODA_LMP)
BOOL PodarLMP(SINT32 s32Prof, UINT32 u32Legales, TPosicion * pPos)
{
	static const UINT32 au32ProfLMP[] = { 2, 3, 7, 12, 20, 31, 43 };
	const BOOL bEsCaptura = Pos_GetCaptura(pPos + 1); // El pPos que recibimos es antes de mover, aunque llamemos aquí después de mover

	if (s32Prof < 7
		&& u32Legales > au32ProfLMP[s32Prof]
		&& !bEsCaptura)
	{
		return(TRUE);
	}

	return(FALSE);
}
#endif

/*
 * PodarRazoring
 * 
 * Si estamos tan lejos de alfa que únicamente podemos recuperar con capturas, intentamos llegar a alfa con qsearch
 * 
 *			SF 17.1:											 eval <  alfa - 461 - 315 * prof * prof
 *			DarkSeid 6:											 eval <  alfa - 462 - 297 * prof * prof
 *			KF PB 050325:										 eval <  alfa - 469 - 307 * prof * prof
 *			Obsidian 16:				   alfa < 2000		 &&  eval <  alfa		- 352 * prof
 *			Arcanum 2.4:				   alfa no es mate	 &&  eval <  alfa       - 200 * prof
 *			CFish 12.5:		prof <= 4						 &&  eval <  alfa - 369 - 254 * prof * prof
 *		Peacekeeper 3.01:	prof <  4						 &&  eval <= alfa -  30 -  90 * prof * prof
 *			Hakka rel 3:	prof <= 3						 &&  eval <= alfa - 150 -  50 * prof
 *			Perseus 1.0:	prof <= 3  &&  beta no es mate   &&  eval <  alfa - 119 - 233 * prof
 *			RukChess:		prof <= 3                        &&  eval <= alfa -  50 -  50 * prof
 *			Caissa 1.22:	prof <= 3  &&  beta no mate		 &&  eval <  beta -  23 - 155 * prof
 *			Laser 1.7:		prof <= 2                        &&  eval <  alfa - 300
 *			Xiphos 0.6.1:	prof <= 3  &&  beta no es mate	 &&  eval <  beta - 200
 *			Stash 37:		prof == 1						 &&  eval <= alfa - 144
 *			Fire 10:		prof <  4						 &&  eval <= alfa - 384
 *			Tucano 12:		prof <  6  &&  alfa no es mate   &&  eval <  alfa		- { 0, 250, 500, 750, 1000, 1250}
 *			Rodent IV:		prof <= 4						 &&  eval <  beta		- { 0, 300, 360, 420,  480		}
 *			Demolito 2025:	prof <= 5						 &&  eval <= alfa		- { 0, 229, 438, 495,  878, 1094}
 *			Starzix 6:		prof <= 6  &&  abs(alfa) < 2000  &&  eval <  alfa		- 407 * prof
 *			Sirius 8.0:		prof <= 3  &&  alfa < 2000		 &&  eval <= alfa		- 450 * prof
 *			Astra 5.1:		prof <= 5						 &&  eval <  alfa		- 236 * prof
 *			PlentyChess:	prof <  5  &&  alfa no mate		 &&  eval <  alfa		- 238 * prof
 *			Clover 8.1:		prof <= 2						 &&  eval <= alfa		- 153 * prof
 *			Alexandria 8:	prof <= 5						 &&  eval <  alfa		- 247 * prof
 *			Zangdar 309:	prof <= 3						 &&  eval <= alfa		- 200 * prof
 *			Koivisto 9.2:	prof <= 3						 &&  eval <  beta		- 190 * prof
 *			Cataphract 1.0:	prof <= 5						 &&  eval <= alfa		- 130 * prof
 *
 *			Yo:							   alfa no es mate	 &&  eval <  alfa		- 400 * prof
 */
BOOL PodarRazoring(TPosicion * pPos, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Prof, SINT32 s32Ply)
{
	#if !defined(BUS_RAZOR_MARGEN)
		return(FALSE);
	#endif

	int iVal;

	if (Pos_GetJaqueado(pPos))
		return(FALSE);
	if (pPos->s32Eval == NO_EVAL)
		return(FALSE);

	if (abs(s32Alfa) < VICTORIA && pPos->s32Eval < s32Alfa - BUS_RAZOR_MARGEN * s32Prof)
	{
		iVal = QSConJaques(pPos, s32Alfa, s32Beta, s32Ply, QS_PROF_JAQUES);
		if (iVal <= s32Alfa)
			return(TRUE);
	}

	return(FALSE);
}