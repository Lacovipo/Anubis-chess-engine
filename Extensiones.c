
#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"
#include "Inline.h"
#include "Bitboards_inline.h"
#include "Funciones.h"

/*
 *
 * Extensiones
 *
 * Recibe: Puntero a la posición actual (tras realizar el movimiento), la jugada a realizar
 *
 * Devuelve: Un entero con la variación de profundidad a aplicar
 *
 * Descripción: Revisa todas las posibles extensiones. En cuanto encuentra un motivo para extender, devuelve 1. Si
 *  llega hasta el final, devuelve cero
 *
 * Nota: No olvidar que tengo una copia casi literal de esta función en Inicio consola, que uso para recorrer de
 *  forma manual variantes observando el efecto de las extensiones y podas. Si toco algo aquí, debo también cambiarlo
 *  allí.
 *
 */
/*
	Idea: Hay ciertos criterios (amenazas, capturas cerca del rey, ataque) que no siempre justifican de por sí una
        extensión. Se podría probar a extender sólo cuando se dieran dos de estos casos y, en el caso del ataque,
        una fracción que fuese directamente proporcional al incremento de la presión. Por ejemplo, podríamos empezar
        por asignar media extensión a cada uno de estos tipos
*/
SINT32 Extensiones(TPosicion * pPos, TJugada jug, SINT32 s32Prof, SINT32 s32Ply)
{
	assert(s32Prof >= 0);

	//
	// Extensión de jaque
	//
	if (Pos_GetJaqueado(pPos))
	{
		assert(Pos_GetTurno(pPos) == BLANCAS ? Atacado(pPos, pPos->u8PosReyB, NEGRAS) : Atacado(pPos, pPos->u8PosReyN, BLANCAS));
		return(1);
	}

	//
	// Limitación de profundidad: como las extensiones van contra el efecto horizonte, entiendo que cuanta más profundidad, menos efecto horizonte
	// 
	if (s32Prof > s32Ply)
		return(0);
	
	//
	// Extensión de recaptura (dos capturas consecutivas que devuelven la eval casi a lo mismo)
	// Siguiendo las recomendaciones de Ed en su artículo (adaptadas a mi esquema):
	//	 - No extender si valor SEE < 0
	//	 - No extender si estamos muy lejos de la eval en la raíz
	//   - No extender recapturas en los finales (esto es cosecha mía)
	//
	#if defined(EXT_RECAPTURA)
		if (Pos_GetRecaptura(pPos)
			&& pPos->u32Fase < FAS_FINAL
			&& abs(pPos->s32EvalMaterial - (pPos-2)->s32EvalMaterial) < 25          // El material es virtualmente el mismo
			&& Jug_GetVal(jug) >= 0)                                                // SEE positivo
		{
			return(1);
		}
	#endif

	//
	// Extensiones que dependen del turno
	// OJO: que el turno que estamos considerando aquí es el de después del movimiento
	//
	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		//
		// Extensión de peón en 7
		//
		#if defined(EXT_PEON_7)
			if (Tab_GetFila(Jug_GetHasta(jug)) == 6
				&& (pPos->u64PeonesN & BB_Mask(Jug_GetHasta(jug))))
			{
				return(1);
			}
		#endif

		//
		// Extensiones de capturas cerca del rey
		//
		#if defined(EXT_CAPT_CERCA)
			if (Pos_GetCaptura(pPos) && Tab_GetDistancia(pPos->u8PosReyB,Jug_GetHasta(jug)) < 2 && pPos->u32Fase < FAS_FINAL)
			{
				dbDatosBusqueda.u32ExtensionesCaptCercaRey++;
				return(1);
			}
		#endif
	}
	else // Le toca a las negras después del movimiento, es decir, es el blanco quien va a extender
	{
		//
		// Extensión de peón en 7
		//
		#if defined(EXT_PEON_7)
			if (Tab_GetFila(Jug_GetHasta(jug)) == 1
				&& (pPos->u64PeonesB & BB_Mask(Jug_GetHasta(jug))))
			{
				return(1);
			}
		#endif

		//
		// Extensiones de capturas cerca del rey
		//
		#if defined(EXT_CAPT_CERCA)
			if (Pos_GetCaptura(pPos) && Tab_GetDistancia(pPos->u8PosReyN,Jug_GetHasta(jug)) < 2 && pPos->u32Fase < FAS_FINAL)
			{
				dbDatosBusqueda.u32ExtensionesCaptCercaRey++;
				return(1);
			}
		#endif
	}

	//
	// Extensión de paso al final
	//
	#if defined(EXT_FINAL)
		// Sólo tiene sentido examinar capturas, porque es cuando se producen las transiciones
		// Nos limitamos a una diferencia de eval con la raíz que sea razonable
		if (Pos_GetCaptura(pPos))
		{
			// Paso a final de peones - extendemos 3 plies
			if (!pPos->u8NumPiezasB
				&& !pPos->u8NumPiezasN
				&& ((pPos-1)->u8NumPiezasB || (pPos-1)->u8NumPiezasN))
			{
				return(3);
			}

			// Paso a final de piezas ligeras - extender 1 ply si se simplifica hacia un final con una sola pieza menor
			//  o torre por barba
			if (pPos->u8NumPiezasB == 1
				&& pPos->u8NumPiezasN == 1
				&& ((pPos-1)->u8NumPiezasB > 1 || (pPos-1)->u8NumPiezasN > 1)
				&& !pPos->u64DamasB
				&& !pPos->u64DamasN)
			{
				assert(pPos->u8NumPiezasB == 1);
				assert(pPos->u8NumPiezasN == 1);
				assert(((pPos-1)->u8NumPiezasB > 1 || (pPos-1)->u8NumPiezasN > 1));
				assert(pPos->u64DamasB == BB_TABLEROVACIO);
				assert(pPos->u64DamasN == BB_TABLEROVACIO);
				return(1);
			}
		}
	#endif

	//
	// No extendemos
	//
	return(0);
}

