
#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"
#include "Bitboards_inline.h"

/*
 * JugadaCorrecta
 *
 * Recibe: Puntero a la posición actual, jugada a verificar
 *
 * Devuelve: TRUE si la jugada es pseudo-legal, FALSE en caso contrario
 *
 * Descripción: Dada una jugada que, normalmente, vendrá de la tabla hash, se hacen las comprobaciones que se pueden
 *  para verificar la pseudo-legalidad del movimiento
 *
 */
BOOL JugadaCorrecta(TPosicion * pPos, TJugada jug)
{
	UINT32	u32Desde = Jug_GetDesde(jug);
	UINT32	u32Hasta = Jug_GetHasta(jug);
	UINT64  u64BBDesde = BB_Mask(u32Desde);
	UINT64  u64BBHasta = BB_Mask(u32Hasta);

	// Fila, columna o diagonal libre, no bloqueada por ninguna pieza
	if ((pPos->u64TodasB | pPos->u64TodasN) & au64Entre[u32Desde][u32Hasta])
		return(FALSE);

	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		// Casilla desde ocupada por pieza blanca; casilla hasta no ocupada por pieza blanca
		if ((pPos->u64TodasB & u64BBDesde) == BB_TABLEROVACIO)
		{
			assert((!((pPos->u64TodasB & u64BBDesde) && ((~pPos->u64TodasB) & u64BBHasta))));
			return(FALSE);
		}
		if ((pPos->u64TodasB & u64BBHasta) != BB_TABLEROVACIO)
		{
			assert((!((pPos->u64TodasB & u64BBDesde) && ((~pPos->u64TodasB) & u64BBHasta))));
			return(FALSE);
		}
		assert((!((pPos->u64TodasB & u64BBDesde) && ((~pPos->u64TodasB) & u64BBHasta))) == FALSE);

		switch(PiezaEnCasilla(pPos, u32Desde))
		{
			case PB:
				if (Tab_GetFila(u32Hasta) == 0 && !Jug_GetPromo(jug))
					return(FALSE);
				if (Tab_GetColumna(u32Desde) != COLH && u32Hasta == u32Desde + TAB_ARRIBA_DCHA && (u64BBHasta & pPos->u64TodasN))
					return(TRUE);
				if (Tab_GetColumna(u32Desde) != COLA && u32Hasta == u32Desde + TAB_ARRIBA_IZDA && (u64BBHasta & pPos->u64TodasN))
					return(TRUE);
				if (u32Hasta == u32Desde + TAB_ARRIBA && (u64BBHasta & pPos->u64TodasN) == BB_TABLEROVACIO)
					return(TRUE);
				if (u32Hasta == u32Desde + TAB_ARRIBA + TAB_ARRIBA && (u64BBHasta & pPos->u64TodasN) == BB_TABLEROVACIO)
					return(TRUE);
				if (u32Hasta == pPos->u8AlPaso)
					return(TRUE); // Asumo que no voy a guardar jugadas absurdas en la tabla hash, como saltos de peones por el tablero

				return(FALSE);
				break;
			case CB:
				if ((BB_ATAQUES_CABALLO[u32Desde] & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			case AB:
				if ((BB_ATAQUES_ALFIL[u32Desde] & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			case TB:
				if ((au64AtaquesTorre[u32Desde] & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			case DB:
				if (((BB_ATAQUES_ALFIL[u32Desde] | au64AtaquesTorre[u32Desde]) & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			case RB:
				if (u32Desde == TAB_E1 && u32Hasta == TAB_G1)
				{
					if ((pPos->u64TodasB | pPos->u64TodasN) & BB_F1G1)
						return(FALSE);
					if ((pPos->u64TorresB & BB_H1) == BB_TABLEROVACIO)
						return(FALSE);
					if (pPos->u64AtaquesN & BB_F1G1)
						return(FALSE);
					return(TRUE);
				}
				if (u32Desde == TAB_E1 && u32Hasta == TAB_C1)
				{
					if ((pPos->u64TodasB | pPos->u64TodasN) & BB_B1C1D1)
						return(FALSE);
					if ((pPos->u64TorresB & BB_A1) == BB_TABLEROVACIO)
						return(FALSE);
					if (pPos->u64AtaquesN & (BB_C1 | BB_D1))
						return(FALSE);
					return(TRUE);
				}
				if ((BB_ATAQUES_REY[u32Desde] & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			default:
				__assume(0);
		}
	} // if (Pos_GetTurno(pPos) == BLANCAS)
	else
	{
    // Casilla desde ocupada por pieza negra; casilla hasta no ocupada por pieza negra
		if (!((pPos->u64TodasN & u64BBDesde) && ((~pPos->u64TodasN) & u64BBHasta)))
			return(FALSE);

		switch(PiezaEnCasilla(pPos, u32Desde))
		{
			case PN:
				if (Tab_GetFila(u32Hasta) == 7 && !Jug_GetPromo(jug))
					return(FALSE);
				if (Tab_GetColumna(u32Desde) != COLH && u32Hasta == u32Desde + TAB_ABAJO_DCHA && (u64BBHasta & pPos->u64TodasB))
				  return(TRUE);
				if (Tab_GetColumna(u32Desde) != COLA && u32Hasta == u32Desde + TAB_ABAJO_IZDA && (u64BBHasta & pPos->u64TodasB))
				  return(TRUE);
				if (u32Hasta == u32Desde + TAB_ABAJO && (u64BBHasta & pPos->u64TodasB) == BB_TABLEROVACIO)
				  return(TRUE);
				if (u32Hasta == u32Desde + TAB_ABAJO + TAB_ABAJO && (u64BBHasta & pPos->u64TodasB) == BB_TABLEROVACIO)
				  return(TRUE);
				if (u32Hasta == pPos->u8AlPaso)
				  return(TRUE); // Asumo que no voy a guardar jugadas absurdas en la tabla hash, como saltos de peones por el tablero

				return(FALSE);
				break;
			case CN:
				if ((BB_ATAQUES_CABALLO[u32Desde] & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			case AN:
				if ((BB_ATAQUES_ALFIL[u32Desde] & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			case TN:
				if ((au64AtaquesTorre[u32Desde] & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			case DN:
				if (((BB_ATAQUES_ALFIL[u32Desde] | au64AtaquesTorre[u32Desde]) & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			case RN:
				if (u32Desde == TAB_E8 && u32Hasta == TAB_G8)
				{
					if ((pPos->u64TodasB | pPos->u64TodasN) & BB_F8G8)
						return(FALSE);
					if ((pPos->u64TorresN & BB_H8) == BB_TABLEROVACIO)
						return(FALSE);
					if (pPos->u64AtaquesB & BB_F8G8)
						return(FALSE);
					return(TRUE);
				}
				if (u32Desde == TAB_E8 && u32Hasta == TAB_C8)
				{
					if ((pPos->u64TodasB | pPos->u64TodasN) & BB_B8C8D8)
						return(FALSE);
					if ((pPos->u64TorresN & BB_A8) == BB_TABLEROVACIO)
						return(FALSE);
					if (pPos->u64AtaquesB & (BB_C8 | BB_D8))
						return(FALSE);
					return(TRUE);
				}
				if ((BB_ATAQUES_REY[u32Desde] & u64BBHasta) == BB_TABLEROVACIO)
					return(FALSE);
				break;
			default:
				__assume(0);
		}
	} // else [if (Pos_GetTurno(pPos) == BLANCAS)]

	return(TRUE);
}
