
#include "Preprocesador.h"
#include "Tipos.h"
#include "Funciones.h"
#include "Variables.h"
#include "Inline.h"
#include "Bitboards_inline.h"

/*
 *
 * EstoyMate
 *
 * Recibe: puntero a la posición actual
 *
 * Devuelve: TRUE si es jaque mate, FALSE en caso contrario
 *
 */
static BOOL EstoyMate(TPosicion * pPos)
{
	UINT64 u64Jaqueadores;
	UINT64 u64CasillasAccesibles;
	UINT64 u64Temp;
	UINT64 u64Entre;
	UINT64 u64Vacias = ~(pPos->u64TodasB | pPos->u64TodasN);
	UINT32 u32Desde;
	UINT32 u32Hasta;

	if (!Pos_GetJaqueado(pPos))
		return(FALSE);

	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		u64Jaqueadores = AtaquesN_Hacia(pPos, pPos->u8PosReyB);
		u64Temp = u64Jaqueadores;
		if (CuentaUnos(u64Jaqueadores) > 1)
		{
			// Si tenemos más de una pieza jaqueándonos, las únicas capturas legales posibles son a piezas enemigas jaqueadoras que no estén
			//	defendidas, y que la captura sea con nuestro rey
			// OJO: no es posible que una de esas piezas esté defendida indirectamente, a través de nuestro rey, porque para eso tendríamos
			//	que haber estado en jaque antes de que moviese el contrario, lo cual es ilegal, claro
			u64Jaqueadores &= (BB_ATAQUES_REY[pPos->u8PosReyB] & ~pPos->u64AtaquesN);
			if (u64Jaqueadores)
				return(FALSE);
		}
		else
		{
			// Sólo hay una pieza jaqueando, así que busco todas las mías que la atacan y me la zampo
			u32Hasta = PrimerUno(u64Jaqueadores);
			u64Jaqueadores = AtaquesB_Hacia(pPos, u32Hasta);
			while (u64Jaqueadores)
			{
				// Itero sobre los atacantes de la pieza jaqueadora
				u32Desde = BB_GetBitYQuitar(&u64Jaqueadores);
				if (u32Desde == pPos->u8PosReyB)
				{
					// Si el atacante es mi rey, sólo puedo comérmela si no está defendida
					if (BB_Mask(u32Hasta) & ~pPos->u64AtaquesN)
						return(FALSE);
				}
				else
				{
					// Antes que nada, me aseguro de que el atacante no está clavado
					if (!(BB_Mask(u32Desde) & pPos->u64Clavadas))
						return(FALSE);
				}
			}
		}
		// Otra opción es que el rey se zampe todo lo que tenga alrededor que esté desprotegido
		u64Jaqueadores = BB_ATAQUES_REY[pPos->u8PosReyB] & pPos->u64TodasN & ~pPos->u64AtaquesN & ~u64Temp;
		if (u64Jaqueadores)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Jaqueadores);

			u64Temp = u64Vacias;
			BB_SetBit(&u64Temp, pPos->u8PosReyB);
			switch(u32Hasta-pPos->u8PosReyB)
			{
				case TAB_ARRIBA_IZDA:
					u64Entre = AtaquesAbajoDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_whileX;
					break;
				case TAB_ARRIBA:
					u64Entre = AtaquesAbajo(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_whileX;
					break;
				case TAB_ARRIBA_DCHA:
					u64Entre = AtaquesAbajoIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_whileX;
					break;
				case TAB_IZQUIERDA:
					u64Entre = AtaquesDerecha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_whileX;
					break;
				case TAB_DERECHA:
					u64Entre = AtaquesIzquierda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_whileX;
					break;
				case TAB_ABAJO_IZDA:
					u64Entre = AtaquesArribaDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_whileX;
					break;
				case TAB_ABAJO:
					u64Entre = AtaquesArriba(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_whileX;
					break;
				case TAB_ABAJO_DCHA:
					u64Entre = AtaquesArribaIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_whileX;
					break;
			}

			return(FALSE);
otra_whileX: ;
		}

		//
		// Jugadas de rey a casillas libres
		//
		u64CasillasAccesibles = BB_ATAQUES_REY[pPos->u8PosReyB] & ~pPos->u64AtaquesN & u64Vacias;
		while (u64CasillasAccesibles)
		{
			u32Hasta = BB_GetBitYQuitar(&u64CasillasAccesibles);

			u64Temp = u64Vacias;
			BB_SetBit(&u64Temp, pPos->u8PosReyB);
			switch(u32Hasta-pPos->u8PosReyB)
			{
				case TAB_ARRIBA_IZDA:
					u64Entre = AtaquesAbajoDcha(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ARRIBA:
					u64Entre = AtaquesAbajo(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ARRIBA_DCHA:
					u64Entre = AtaquesAbajoIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_IZQUIERDA:
					u64Entre = AtaquesDerecha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_DERECHA:
					u64Entre = AtaquesIzquierda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ABAJO_IZDA:
					u64Entre = AtaquesArribaDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ABAJO:
					u64Entre = AtaquesArriba(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ABAJO_DCHA:
					u64Entre = AtaquesArribaIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
			}

			return(FALSE);
otra_while: ;
		}

		//
		// Veamos si podemos interponer algo
		//
		u64Jaqueadores = AtaquesN_Hacia(pPos, pPos->u8PosReyB);
		if (CuentaUnos(u64Jaqueadores) == 1)
		{
			u64Jaqueadores = AtaquesDeslizN_Hacia(pPos, pPos->u8PosReyB);
			if (u64Jaqueadores)
			{
				// Ya sabemos que sólo hay una pieza, y que es deslizante, así que intentamos tapar la línea
				// Ojo: no nos limitamos a casillas atacadas por las blancas porque puede haber una avance de peón que tape la fila
				u64Entre = au64Entre[PrimerUno(u64Jaqueadores)][pPos->u8PosReyB];

				// En u64Entre tenemos las casillas que están entre el jaqueador y nuestro rey, y que podemos ocupar con una pieza nuestra. Iteramos sobre dicho bitmap, localizando qué piezas nuestras atacan esas
				//	casillas, y generando las jugadas correspondientes, haciendo excepción de nuestro rey, claro está
				while (u64Entre)
				{
					u32Hasta = BB_GetBitYQuitar(&u64Entre);
					u64Jaqueadores = MovimientosB_Hacia(pPos, u32Hasta);
					while (u64Jaqueadores)
					{
						u32Desde = BB_GetBitYQuitar(&u64Jaqueadores);
						if (u32Desde != pPos->u8PosReyB)
						{
							// Antes que nada, me aseguro de que el bloqueador no está clavado
							if (!(BB_Mask(u32Desde) & pPos->u64Clavadas))
								return(FALSE);
						} // if (u32Desde != pPos->u8PosReyB)
					} // while (u64Jaqueadores)
				} // while (u64Entre)
			} // if (u64Jaqueadores)
		} // if (CuentaUnos(u64Jaqueadores) == 1)
	}
	else ///////////////////////////////////////////////////////////////////////////////////////////////////
	{
		//
		// NEGRAS
		//

		u64Jaqueadores = AtaquesB_Hacia(pPos, pPos->u8PosReyN);
		u64Temp = u64Jaqueadores;
		if (CuentaUnos(u64Jaqueadores) > 1)
		{
			// Si tenemos más de una pieza jaqueándonos, las únicas capturas legales posibles son a piezas enemigas jaqueadoras que no estén defendidas, y que la captura sea con nuestro rey
			u64Jaqueadores &= (BB_ATAQUES_REY[pPos->u8PosReyN] & ~pPos->u64AtaquesB);
			if (u64Jaqueadores)
				return(FALSE);
		}
		else
		{
			// Sólo hay una pieza jaqueando, así que busco todas las mías que la atacan y me la zampo
			u32Hasta = PrimerUno(u64Jaqueadores);
			u64Jaqueadores = AtaquesN_Hacia(pPos, u32Hasta);
			while (u64Jaqueadores)
			{
				// Itero sobre los atacantes de la pieza jaqueadora
				u32Desde = BB_GetBitYQuitar(&u64Jaqueadores);
				if (u32Desde == pPos->u8PosReyN)
				{
					// Si el atacante es mi rey, sólo puedo comérmela si no está defendida
					if (BB_Mask(u32Hasta) & ~pPos->u64AtaquesB)
						return(FALSE);
				}
				else
				{
					// Antes que nada, me aseguro de que el atacante no está clavado
					if (!(BB_Mask(u32Desde) & pPos->u64Clavadas))
						return(FALSE);
				}
			}
		}
		// Otra opción es que el rey se zampe todo lo que tenga alrededor que esté desprotegido
		u64Jaqueadores = BB_ATAQUES_REY[pPos->u8PosReyN] & pPos->u64TodasB & ~pPos->u64AtaquesB & ~u64Temp;
		if (u64Jaqueadores)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Jaqueadores);

			u64Temp = u64Vacias;
			BB_SetBit(&u64Temp, pPos->u8PosReyN);
			switch(u32Hasta-pPos->u8PosReyN)
			{
				case TAB_ARRIBA_IZDA:
					u64Entre = AtaquesAbajoDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_whileNX;
					break;
				case TAB_ARRIBA:
					u64Entre = AtaquesAbajo(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_whileNX;
					break;
				case TAB_ARRIBA_DCHA:
					u64Entre = AtaquesAbajoIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_whileNX;
					break;
				case TAB_IZQUIERDA:
					u64Entre = AtaquesDerecha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_whileNX;
					break;
				case TAB_DERECHA:
					u64Entre = AtaquesIzquierda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_whileNX;
					break;
				case TAB_ABAJO_IZDA:
					u64Entre = AtaquesArribaDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_whileNX;
					break;
				case TAB_ABAJO:
					u64Entre = AtaquesArriba(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_whileNX;
					break;
				case TAB_ABAJO_DCHA:
					u64Entre = AtaquesArribaIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_whileNX;
					break;
			}

			return(FALSE);
otra_whileNX: ;
		}

		//
		// Jugadas de rey a casillas libres
		//
		u64CasillasAccesibles = BB_ATAQUES_REY[pPos->u8PosReyN] & ~pPos->u64AtaquesB & u64Vacias;
		while (u64CasillasAccesibles)
		{
			u32Hasta = BB_GetBitYQuitar(&u64CasillasAccesibles);

			u64Temp = u64Vacias;
			BB_SetBit(&u64Temp, pPos->u8PosReyN);
			switch(u32Hasta-pPos->u8PosReyN)
			{
				case TAB_ARRIBA_IZDA:
					u64Entre = AtaquesAbajoDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_whileN;
					break;
				case TAB_ARRIBA:
					u64Entre = AtaquesAbajo(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_whileN;
					break;
				case TAB_ARRIBA_DCHA:
					u64Entre = AtaquesAbajoIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_whileN;
					break;
				case TAB_IZQUIERDA:
					u64Entre = AtaquesDerecha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_whileN;
					break;
				case TAB_DERECHA:
					u64Entre = AtaquesIzquierda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_whileN;
					break;
				case TAB_ABAJO_IZDA:
					u64Entre = AtaquesArribaDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_whileN;
					break;
				case TAB_ABAJO:
					u64Entre = AtaquesArriba(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_whileN;
					break;
				case TAB_ABAJO_DCHA:
					u64Entre = AtaquesArribaIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_whileN;
					break;
			}

			return(FALSE);
otra_whileN: ;
		}

		//
		// Veamos si podemos interponer algo
		//
		u64Jaqueadores = AtaquesB_Hacia(pPos, pPos->u8PosReyN);
		if (CuentaUnos(u64Jaqueadores) == 1)
		{
			u64Jaqueadores = AtaquesDeslizB_Hacia(pPos, pPos->u8PosReyN);
			if (u64Jaqueadores)
			{
				// Ya sabemos que sólo hay una pieza, y que es deslizante, así que intentamos tapar la línea
				// Ojo: no nos limitamos a casillas atacadas por las negras porque puede haber una avance de peón que tape la fila
				u64Entre = au64Entre[PrimerUno(u64Jaqueadores)][pPos->u8PosReyN];

				// En u64Entre tenemos las casillas que están entre el jaqueador y nuestro rey, y que podemos ocupar con una pieza nuestra. Iteramos sobre dicho bitmap, localizando qué piezas nuestras atacan esas
				// casillas, y generando las jugadas correspondientes, haciendo excepción de nuestro rey, claro está
				while (u64Entre)
				{
					u32Hasta = BB_GetBitYQuitar(&u64Entre);
					u64Jaqueadores = MovimientosN_Hacia(pPos, u32Hasta);
					while (u64Jaqueadores)
					{
						u32Desde = BB_GetBitYQuitar(&u64Jaqueadores);
						if (u32Desde != pPos->u8PosReyN)
						{
							// Antes que nada, me aseguro de que el bloqueador no está clavado
							if (!(BB_Mask(u32Desde) & pPos->u64Clavadas))
								return(FALSE);
						} // if (u32Desde != pPos->u8PosReyB)
					} // while (u64Jaqueadores)
				} // while (u64Entre)
			} // if (u64Jaqueadores)
		} // if (CuentaUnos(u64Jaqueadores) == 1)
	}

	// Si no hemos podido encontrar ninguna jugada salvadora, estamos fritos
	return(TRUE);
}

/*
 *
 * QSearch
 *
 */
SINT32 QSearch(TPosicion * pPos, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Ply)
{
	TJugada		jugMejor = JUGADA_NULA;
	TJugada *	pJug;
	UINT32		u32Legales;
	SINT32		s32Val;
	SINT32		s32Mejor;
	SINT32		s32AlfaOriginal = s32Alfa;

	// Profundidad selectiva
	if (s32Ply > dbDatosBusqueda.s32ProfSel)
		dbDatosBusqueda.s32ProfSel = s32Ply;

	//
	// Salida por indicación externa
	//
	if (GetEsHoraDeParar(&g_tReloj, &dbDatosBusqueda, as32PilaEvaluaciones))
	{
		dbDatosBusqueda.bAbortar = TRUE;
		return(TABLAS);
	}
	if (ProcesarComando())
	{
		dbDatosBusqueda.bAbortar = TRUE;
		return(TABLAS);
	}
	if (InsuficienteMaterial(pPos))
		return(TABLAS);

	//
	// Poda de distancia a mate
	//
	{
		s32Alfa = max(-INFINITO + s32Ply, s32Alfa);
		s32Beta = min(INFINITO - s32Ply - 1, s32Beta);

		if (s32Alfa >= s32Beta)
			return(s32Alfa);
	}

	//
	// Consultar tabla hash
	//
	TNodoHash * pNodoHash = NULL;
	SINT32 s32EvalHash = -INFINITO;
	int iBoundHash = 0;
	if (ConsultarHash(pPos, 0, s32Alfa, s32Beta, &pNodoHash) && !Bus_DoyMatePronto(abs(pNodoHash->jug.s32Val)))
		return(EvalFromTT(pNodoHash->jug.s32Val, s32Ply));
	if (pNodoHash != NULL)
	{
		s32EvalHash = EvalFromTT(pNodoHash->jug.s32Val, s32Ply);
		iBoundHash = Hash_GetBound(pNodoHash);
	}

	//
	// Ver si me puedo ahorrar una llamada a eval
	//
	if (Pos_GetJaqueado(pPos))
		Pos_SetEval(pPos, NO_EVAL);
	else
	{
		if ((iBoundHash == BOUND_EXACTO)
			|| (iBoundHash == BOUND_UPPER && s32EvalHash < s32Alfa)
			|| (iBoundHash == BOUND_LOWER && s32EvalHash > s32Beta))
		{
			Pos_SetEval(pPos, s32EvalHash);
		}
		else
			pPos->s32Eval = Evaluar(pPos);
	}

	//
	// Salida por profundidad agotada
	//
	if (s32Ply >= MAX_PLIES - 1)
		return(pPos->s32Eval);

	//
	// Comprobación de seguridad: si estoy en jaque, compruebo si además es mate
	//
	if (EstoyMate(pPos))
		return(-INFINITO + s32Ply);

	//
	// Salida por "stand-pat" (compruebo el jaqueado más abajo)
	//
	s32Mejor = pPos->s32Eval;
	#if defined(QS_STANDPAT_AUM)
		s32Mejor += QS_STANDPAT_AUM;
	#endif

	if (pPos->u8Jaqueado)
		s32Mejor = -INFINITO + s32Ply;

	if (s32Mejor > s32Alfa)
	{
		if (s32Mejor >= s32Beta)
		{
			GrabarHash(pPos, 0, EvalToTT(s32Mejor, s32Ply), JUGADA_NULA, s32Alfa, s32Beta);
			return(s32Mejor);
		}
		s32Alfa = s32Mejor;
	}

	#if defined(QS_PODA_DELTA)
		//
		// Clover 8.1:	mejor + 1010 < alfa
		//
		if (!Pos_GetJaqueado(pPos) && s32Mejor + QS_PODA_DELTA < s32Alfa)
			return(s32Alfa);
	#endif

	//
	// Bucle principal
	//
	if (Pos_GetJaqueado(pPos))
	{
		TJugada * pJugTmp;

		//
		// Generar jugadas: las genero todas (capturas y no capturas) para saber cuántas legales hay (en jaque, sólo genero jugadas
		//	legales). pJugTmp me sirve para saber dónde empiezan las capturas, y aplicar ordenación SEE
		//
		pJugTmp = GenerarMovimientos(pPos, pPos->pListaJug);
		pJug = GenerarCapturas(pPos, pJugTmp + 1, FALSE);
		if (pJug < pPos->pListaJug)
			u32Legales = 0;
		else
			u32Legales = (UINT32)(pJug - pPos->pListaJug) + 1;

		// Nunca debe entrar aquí porque ya comprobé al principio que no estoy mate
		if (u32Legales == 0)
		{
			assert(0);
			return(-INFINITO + s32Ply);
		}

		// Aquí debe entrar siempre. Acabaré quitando esta comprobación
		if (pJug >= pPos->pListaJug)
		{
			// Asignar valor SEE a las capturas para ordenación
			AsignarValorSEE(pPos, pJugTmp + 1, pJug);
			for (; pJug >= pPos->pListaJug; pJug--)
			{
				CambiarJugada(pPos, pJug);
				assert(JugadaCorrecta(pPos, *pJug));

				//
				// Mover y analizar
				//
				if (!Mover(pPos, pJug))
				{
					u32Legales--;
					if (!u32Legales)
					{
						// Es posible estar aquí no estando en jaque, si el rey no tiene casillas. Si además no tengo jugadas legales disponibles, estoy ahogado
						if (Pos_GetJaqueado(pPos))
							return(-INFINITO + s32Ply - 1);
						else
							return(TABLAS);
					}
					continue;
				}
				dbDatosBusqueda.u32NumNodosQ++;
				s32Val = -QSearch(pPos + 1, -s32Beta, -s32Alfa, s32Ply + 1);
				if (dbDatosBusqueda.bAbortar)
					return(TABLAS);

				//
				// Salir si beta-cutoff. Reajustar Alfa y Mejor
				//
				if (s32Val > s32Mejor)
				{
					if (s32Val > s32Alfa)
					{
						if (s32Val >= s32Beta)
						{
							GrabarHash(pPos, 0, EvalToTT(s32Val, s32Ply), *pJug, s32Alfa, s32Beta);
							return(s32Val);
						}
						s32Alfa = s32Val;
					}
					jugMejor = *pJug;
					s32Mejor = s32Val;
				}
			} // for (; pJug >= pPos->pListaJug; pJug--)
		} // if (pJug >= pPos->pListaJug)
	}
	else
	{
		// No jaqueado
		u32Legales = 0;
		pJug = GenerarCapturas(pPos, pPos->pListaJug, FALSE);
		if (pJug >= pPos->pListaJug)
		{
			// Asignar valor SEE a las capturas para ordenación
			// Voy a podar en cuanto encuentre una jugada perdedora, así que necesito la lista completamente ordenada, y luego no hago "cambiar"
			AsignarValorSEE(pPos, pPos->pListaJug, pJug);
			Ordenar(pPos, pJug);
			for (; pJug >= pPos->pListaJug; pJug--)
			{
				// Si la jugada pierde material (según SEE), no me molesto en analizarla
				// OJO: y no sólo eso, sino que acabo la búsqueda (porque tengo la lista ordenada, y estoy seguro de que no quedan jugadas con SEE >= 0)
				#if defined(QS_PODA_SEE_NEGATIVO)
					if (Jug_GetVal(*pJug) <= -VAL_PEON
						&& !Bus_ReciboMatePronto(s32Mejor))
					{
						goto salir_QSearch;
					}
				#endif

				// Poda de futility en qsearch
				// OJO: una vez que podamos un movimiento, podemos podarlos todos, porque están ordenados por valor SEE, lo
				//  que significa que ningún otro puede llegar a pasar por encima de alfa
				#if defined(QS_PODA_FUTIL)
					if (pPos->u32Fase < FAS_FINAL
						&& pPos->s32Eval + Jug_GetVal(*pJug) + QS_PODA_FUTIL < s32Alfa)
					{
						s32Mejor = max(s32Mejor, pPos->s32Eval + Jug_GetVal(*pJug) + QS_PODA_FUTIL);
						goto salir_QSearch;
					}
				#endif

				//
				// Mover y analizar
				//
				if (!Mover(pPos,pJug))
					continue;
				u32Legales++;
				dbDatosBusqueda.u32NumNodosQ++;
				s32Val = -QSearch(pPos + 1, -s32Beta, -s32Alfa, s32Ply + 1);
				if (dbDatosBusqueda.bAbortar)
					return(TABLAS);

				//
				// Salir si beta-cutoff. Reajustar Alfa y Mejor
				//
				if (s32Val > s32Mejor)
				{
					if (s32Val > s32Alfa)
					{
						if (s32Val >= s32Beta)
						{
							GrabarHash(pPos, 0, EvalToTT(s32Val, s32Ply), *pJug, s32Alfa, s32Beta);
							return(s32Val);
						}
						s32Alfa = s32Val;
					}
					jugMejor = *pJug;
					s32Mejor = s32Val;
				}
			}
		}
	} // else de if (jaqueado)

salir_QSearch:
	#if defined(QS_INTENTAR_MATES_EN_QSNORMAL)
	{
		BOOL bIntentar = FALSE;

		if (Pos_GetTurno(pPos) == BLANCAS)
		{
			if ((BB_ATAQUES_REY[pPos->u8PosReyN] & ~pPos->u64AtaquesB & ~pPos->u64TodasN) < 3)
				bIntentar = TRUE;
		}
		else
		{
			if ((BB_ATAQUES_REY[pPos->u8PosReyB] & ~pPos->u64AtaquesN & ~pPos->u64TodasB) < 3)
				bIntentar = TRUE;
		}
		if (bIntentar)
		{
			pJug = GenerarJaquesNoCapturas(pPos,pPos->pListaJug);
			for (;pJug >= pPos->pListaJug;pJug--)
			{
				if (!Mover(pPos,pJug))
					continue;
				dbDatosBusqueda.u32NumNodosQ++;
				if (EstoyMate(pPos + 1))
					return(INFINITO - s32Ply);
			}
		}
	}
	#endif

	if (dbDatosBusqueda.bAbortar)
		return(TABLAS);

	if (s32Mejor > s32AlfaOriginal && s32Mejor < s32Beta && !Jug_GetEsNula(jugMejor))
		Bus_ActualizarPV(jugMejor, s32Ply);

	return(s32Mejor);
}
