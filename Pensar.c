#include "Preprocesador.h"
#include "Tipos.h"
#include <stdio.h>
#include "Variables.h"
#include "Inline.h"
#include "Funciones.h"
#include "math.h"
#include "nnue.h"

/*
 *
 * ImprimirParcialPlyConPV
 *
 */
static void ImprimirPlyConPV(TPosicion* pPos, SINT32 s32Prof, SINT32 s32Eval)
{
	char	szPV[1600];

	{
		ExtraerPVdeArray(pPos, szPV);
		ImprimirCadenaGUI(s32Prof, s32Eval, szPV);
	}
}

/*
 *
 * ImprimirCabeceraBusqueda
 *
 */
static void ImprimirCabeceraBusqueda(void)
{
	double dTiempoEsperadoNormal, dTiempoEsperadoExtendido;

	dTiempoEsperadoNormal = GetTiempoEsperadoNormal(&g_tReloj);
	dTiempoEsperadoExtendido = GetTiempoEsperadoExtendido(&g_tReloj);

#if (FICHERO_LOG == AV_LOG)
	ImprimirALog("\nTiempo esperado (1): %2.2f", dTiempoEsperadoNormal);
	ImprimirALog("Tiempo limite (2): %2.2f", dTiempoEsperadoExtendido);
	ImprimirALog("Prof\tEval\tTiempo\t  Nodos   PV");
	ImprimirALog("----\t----\t------\t--------- ------");
#endif
}

/*
 * 
 * Ordenar
 *
 */
void Ordenar(TPosicion * pPos, TJugada * pJugada)
{
	BOOL			bOrdenado = FALSE;
	TJugada		jug;
	TJugada	*	pJug1;
	TJugada	*	pJug2;

	while (!bOrdenado)
	{
		bOrdenado = TRUE;
		for (pJug2 = pJugada; pJug2 > pPos->pListaJug; pJug2--)
		{
			pJug1 = pJug2 - 1;
			if ((Jug_GetVal(*pJug1)) > (Jug_GetVal(*pJug2)))
			{
				bOrdenado = FALSE;
				jug = *pJug2;
				*pJug2 = *pJug1;
				*pJug1 = jug;
			}
		}
	}
}

/*
 * 
 * ImprimirResumenBusqueda
 *
 */
static void ImprimirResumenBusqueda(SINT32 s32Prof)
{
#if (FICHERO_LOG == AV_LOG)
	double dTiempoTranscurrido;
	double dNPS;
	double dPorCientoQ = 0.0;
	double dPorCientoQJ = 0.0;
	double dPorCientoNodosNulos = 0.0;
	double dRestante = 0.0;
	static UINT32 u32NodosIterAnt = 1;

	dTiempoTranscurrido = GetSegundosTranscurridos(&g_tReloj);
	if (s32Prof == 1)
		u32NodosIterAnt = 1;

	if (dTiempoTranscurrido < 0.01)
		dTiempoTranscurrido = 0.01;
	dNPS = dbDatosBusqueda.u32NumNodos / dTiempoTranscurrido;
	if (dbDatosBusqueda.u32NumNodos)
	{
		dPorCientoQ = (double)dbDatosBusqueda.u32NumNodosQ * 100.0 / (double)dbDatosBusqueda.u32NumNodos;
		dPorCientoQJ = (double)dbDatosBusqueda.u32NumNodosQJ * 100.0 / (double)dbDatosBusqueda.u32NumNodos;
		dPorCientoNodosNulos = (double)dbDatosBusqueda.u32NumNulos * 100.0 / (double)dbDatosBusqueda.u32NumNodos;
	}

	dRestante = GetTiempoDisponible(&g_tReloj);
#endif

#if (FICHERO_LOG == AV_LOG)
	ImprimirALog("\nResultados de la busqueda:");
	ImprimirALog("--------------------------");
	ImprimirALog("Tiempo usado: %2.2f  Restante: %2.2f  NPS: %2.2f", dTiempoTranscurrido, dRestante, dNPS);
	ImprimirALog("Nodos: %d  Qnodos: %d (%2.2f)  QJnodos: %d (%2.2f)  Nulos: %d (%2.2f)", dbDatosBusqueda.u32NumNodos, dbDatosBusqueda.u32NumNodosQ, dPorCientoQ, dbDatosBusqueda.u32NumNodosQJ, dPorCientoQJ, dbDatosBusqueda.u32NumNulos, dPorCientoNodosNulos);
	ImprimirALog("BF. Actual: %2.2f  Acum: %2.2f", (double)dbDatosBusqueda.u32NumNodos / (double)u32NodosIterAnt, pow((double)dbDatosBusqueda.u32NumNodos,1.0 / (double)s32Prof));
#endif

#if (FICHERO_LOG == AV_LOG)
	u32NodosIterAnt = dbDatosBusqueda.u32NumNodos;
#endif
}

/*
 * 
 * TerceraRepeticion
 *
 */
static BOOL TerceraRepeticion(TPosicion * pPos)
{
	UINT32 u32Apariciones = 1;

	TPosicion * pPos2 = pPos - 1;

	for (; pPos2 >= pPos->pPunteroRepeticion; pPos2--)
	{
		// La última condición significa que el turno es igual
		if (pPos->u64HashSignature == pPos2->u64HashSignature && Pos_GetTurno(pPos) == Pos_GetTurno(pPos2))
		{
			assert(pPos->u64PeonesB == pPos2->u64PeonesB);
			assert(pPos->u64PeonesN == pPos2->u64PeonesN);
			assert(pPos->u64CaballosB == pPos2->u64CaballosB);
			assert(pPos->u64CaballosN == pPos2->u64CaballosN);
			assert(pPos->u64AlfilesB == pPos2->u64AlfilesB);
			assert(pPos->u64AlfilesN == pPos2->u64AlfilesN);
			assert(pPos->u64TorresB == pPos2->u64TorresB);
			assert(pPos->u64TorresN == pPos2->u64TorresN);
			assert(pPos->u64DamasB == pPos2->u64DamasB);
			assert(pPos->u64DamasN == pPos2->u64DamasN);
			assert(pPos->u8PosReyB == pPos2->u8PosReyB);
			assert(pPos->u8PosReyN == pPos2->u8PosReyN);
			assert(pPos->u64TodasB == pPos2->u64TodasB);
			assert(pPos->u64TodasN == pPos2->u64TodasN);
			u32Apariciones++;
			if (u32Apariciones >= 3)
				return(TRUE);
		}
	}
	return(FALSE);
}

/*
 * 
 * ComprobarYMover
 *
 */
static void ComprobarYMover(TPosicion * pPos)
{
	char szJug[10];

	// Validación previa: la jugada root debe coincidir con la primera jugada de la pv. Si no es así, es que algo chungo ha ocurrido.
	// De hecho, no tiene sentido tener, por separado, una jugada root y una primera jugada del pv, ¿no?
	if (Jug_GetMov(dbDatosBusqueda.jugJugadaRoot) !=  Jug_GetMov(dbDatosBusqueda.ajugPV[0][0]))
	{
		char szJug2[10];

		#if (FICHERO_LOG == AV_LOG)
			ImprimirALog("*DBG* Catástrofe 1");
		#endif

		ImprimirAPantalla("*DBG* Catástrofe 1");

		Jug2Algebraica(dbDatosBusqueda.jugJugadaRoot, szJug);
		Jug2Algebraica(dbDatosBusqueda.ajugPV[0][0], szJug2);

		#if (FICHERO_LOG == AV_LOG)
			ImprimirALog("*DBG* La jugada root era: %s, y la PV[0][0] era: %s", szJug, szJug2);
		#endif

		ImprimirAPantalla("*DBG* La jugada root era: %s, y la PV[0][0] era: %s", szJug, szJug2);

		if (!Jug_GetEsNula(dbDatosBusqueda.jugJugadaRoot))
		{
			if (JugadaCorrecta(pPos, dbDatosBusqueda.jugJugadaRoot))
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Me quedo con la jugada root")
				#endif
				;
			else
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Válgame el señor. La jugada root no era correcta. La pongo a nula");
				#endif
				dbDatosBusqueda.jugJugadaRoot = JUGADA_NULA;
			}
		}
		if (Jug_GetEsNula(dbDatosBusqueda.jugJugadaRoot))
		{
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("*DBG* Entramos por aquí porque la jugada root es nula");
			#endif
			
			if (!Jug_GetEsNula(dbDatosBusqueda.ajugPV[0][0]))
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Ave María purísima, que la de pv no era nula y la podemos utilizar");
				#endif
				dbDatosBusqueda.jugJugadaRoot = dbDatosBusqueda.ajugPV[0][0];
			}
		}
	}

	switch (dbDatosBusqueda.s32EvalRoot)
	{
		case -INFINITO:
			// Nos han dado mate
			// OJO: No debería entrar nunca aquí porque el GUI ya debería haber interrumpido la partida
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("Uf, eval -INFINITO. ¿Por qué he entrado aquí, señor GUI?");
			#endif
			if (dbDatosBusqueda.u32ColorAnubis == BLANCAS)
				ImprimirAPantalla("\n0-1 {Anubis recibe jaque mate 1}");
			else
				ImprimirAPantalla("\n1-0 {Anubis recibe jaque mate 1}");
			break;
		case TABLAS:
			// Puede que nos hayan ahogado
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("Ah, eval tablas, menos mal");
			#endif
			if (Jug_GetEsNula(dbDatosBusqueda.jugJugadaRoot))
			{
				// A veces se lía y entra por aquí cuando le dan mate
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("Qué lastimica, no tengo jugada. ¿Por qué he entrado aquí, señor GUI?");
				#endif
				if (Pos_GetJaqueado(pPos))
				{
					if (dbDatosBusqueda.u32ColorAnubis == BLANCAS)
						ImprimirAPantalla("\n0-1 {Anubis recibe jaque mate 2}");
					else
						ImprimirAPantalla("\n1-0 {Anubis recibe jaque mate 2}");
				}
				else
					ImprimirAPantalla("\n1/2-1/2 {Anubis está ahogado}");
				return;
			}
			// No hay break porque puede ser una eval de tablas sin que se interrumpa la partida
		default:
			// Se supone que disponemos de una jugada para realizar
			if (Jug_GetEsNula(dbDatosBusqueda.jugJugadaRoot))
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Catástrofe 2");
				#endif
				ImprimirAPantalla("*DBG* Catástrofe 2");
				return;
			}

			Jug2Algebraica(dbDatosBusqueda.jugJugadaRoot, szJug);
			ImprimirAPantalla("\nmove %s", szJug);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("\n*** Mi jugada (%d) es: %s", (dbDatosBusqueda.u32NumPlyPartida + 1) / 2, szJug);
			#endif

			pPos->pListaJug[0] = dbDatosBusqueda.jugJugadaRoot;

			Mover(pPos, &pPos->pListaJug[0]);
			pPos++;
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("Ea, pues ya hemos movido y no se ha roto nada");
			#endif
			// A ver si podemos ponderar (nótese que pPos ya ha avanzado)
			if (dbDatosBusqueda.bPonderar)
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* bPonderar es TRUE");
				#endif

				if (Jug_GetEsNula(dbDatosBusqueda.ajugPV[0][1]))
				{
					#if (FICHERO_LOG == AV_LOG)
						ImprimirALog("*DBG* La jugada ponder es nula");
					#endif
					dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;
				}
				else
				{
					#if (FICHERO_LOG == AV_LOG)
						ImprimirALog("*DBG* Tenemos una juagada para ponder");
					#endif
					Jug2Algebraica(dbDatosBusqueda.ajugPV[0][1], szJug);
					#if (FICHERO_LOG == AV_LOG)
						ImprimirALog("*DBG* Jugada para ponder: %s", szJug);
					#endif
					if (JugadaCorrecta(pPos, dbDatosBusqueda.ajugPV[0][1]))
					{
						if (Mover(pPos, &dbDatosBusqueda.ajugPV[0][1]))
						{
							dbDatosBusqueda.jugJugadaPonder = dbDatosBusqueda.ajugPV[0][1];
							Jug2Algebraica(dbDatosBusqueda.jugJugadaPonder, szJug);
							ImprimirAPantalla("Hint: %s", szJug);

							#if (FICHERO_LOG == AV_LOG)
								ImprimirALog("\n*** Jugada para ponder realizada: %s", szJug);
							#endif
						}
						else
						{
							char szJugPonder[10];

							Jug2Algebraica(dbDatosBusqueda.ajugPV[0][1], szJugPonder);
							dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;
							#if (FICHERO_LOG == AV_LOG)
								ImprimirALog("*DBG* Había una jugada para ponder, pero era chunga: %s", szJugPonder);
							#endif
						}
					}
					else
					{
						dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;
						#if (FICHERO_LOG == AV_LOG)
							ImprimirALog("*DBG* La jugada es pseudochunga, parece");
						#endif
					}
				}
			}
	}
}

/*
 * 
 * Pensar
 *
 */
void Pensar(TPosicion * pPos)
{
	SINT32	  s32Prof = 0;
	TJugada * pJug;
	TJugada * pJug2;
	SINT32	  s32EvalRootAnterior;
	BOOL	  bHayJugadaEGTB = FALSE;

	dbDatosBusqueda.bAbortar = FALSE;
	PonerRelojEnMarcha(&g_tReloj);
	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog("*DBG* Reloj puesto en marcha");
	#endif
	DeterminarTiempoBusqueda();
	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog("DBG* Determinado tiempo de búsqueda");
	#endif
	InicializarBusqueda(pPos);
	ImprimirCabeceraBusqueda();

	if (Pos_GetJaqueado(pPos))
		Pos_SetEval(pPos, NO_EVAL);
	else
		pPos->s32Eval = Evaluar(pPos);

	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++)
			as32Historia[i][j] /= 2;

	//
	// EGTB (chikki)
	//
	#include "tbprobe.h"
	if (dbDatosBusqueda.eTipoBusqueda == TBU_PARTIDA && (pPos->u8NumPeonesB + pPos->u8NumPeonesN + pPos->u8NumPiezasB + pPos->u8NumPiezasN + 2 <= TB_LARGEST))
	{
		UINT64 u64Reyes = au64Mask[pPos->u8PosReyN] | au64Mask[pPos->u8PosReyB];
		//RETURN:
		//-A TB_RESULT value comprising :
		//	1) The WDL value(TB_GET_WDL)
		//	2) The suggested move(TB_GET_FROM, TB_GET_TO, TB_GET_PROMOTES, TB_GET_EP)
		//	3) The DTZ value(TB_GET_DTZ)
		//	The suggested move is guaranteed to preserved the WDL value.
		//
		//	Otherwise:
		//	1) TB_RESULT_STALEMATE is returned if the position is in stalemate.
		//	2) TB_RESULT_CHECKMATE is returned if the position is in checkmate.
		//	3) TB_RESULT_FAILED is returned if the probe failed.
		unsigned tbres = tb_probe_root(pPos->u64TodasB, pPos->u64TodasN,
										u64Reyes,
										pPos->u64DamasB | pPos->u64DamasN,
										pPos->u64TorresB | pPos->u64TorresN,
										pPos->u64AlfilesB | pPos->u64AlfilesN,
										pPos->u64CaballosB | pPos->u64CaballosN,
										pPos->u64PeonesB | pPos->u64PeonesN,
										pPos->u8Cincuenta, pPos->u8Enroques,
										pPos->u8AlPaso == TAB_ALPASOIMPOSIBLE ? 0 : pPos->u8AlPaso, pPos->u8Turno,
										NULL);
		switch (tbres)
		{
			case TB_RESULT_STALEMATE:
				printf("#stalemate\n");
				break;
			case TB_RESULT_CHECKMATE:
				printf("#checkmated\n");
				break;
			case TB_RESULT_FAILED:
				printf("#EGTB failed\n");
				break;
			default:
				int wdl = TB_GET_WDL(tbres);
				TJugada jug;
				Jug_SetNula(&jug);
				Jug_SetDesde(&jug, 63 - TB_GET_FROM(tbres));
				Jug_SetHasta(&jug, 63 - TB_GET_TO(tbres));
				int promo = TB_GET_PROMOTES(tbres);
				switch (promo)
				{
					case TB_PROMOTES_NONE:
						break;
					case TB_PROMOTES_QUEEN:
						Jug_SetPromo(&jug, DB);
						break;
					case TB_PROMOTES_ROOK:
						Jug_SetPromo(&jug, TB);
						break;
					case TB_PROMOTES_BISHOP:
						Jug_SetPromo(&jug, AB);
						break;
					case TB_PROMOTES_KNIGHT:
						Jug_SetPromo(&jug, CB);
						break;
					default:
						assert(0);
				}
					
				dbDatosBusqueda.s32EvalRoot = wdl == 0 ? TB_PIERDE : wdl == 4 ? TB_GANA : TABLAS;
				dbDatosBusqueda.jugJugadaRoot = jug;
				Bus_ActualizarPV(jug, 0);
				dbDatosBusqueda.bPonderar = FALSE;
				bHayJugadaEGTB = TRUE;
				goto fin_pensar;
		}
	}

	dbDatosBusqueda.s32ProfSel = 0;
	pPos->u32MovExclu = 0;
	pJug = GenerarMovimientos(pPos, pPos->pListaJug);
	pJug2 = GenerarCapturas(pPos, pJug + 1, TRUE);

	// Una pequeña salvaguarda de seguridad: recorrer los movimientos generados y quitar de la lista los que no sean legales
	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog("*DBG* Generados los movimientos de la raíz, procedemos a descartar los que no sean legales");
	#endif
	{
		TJugada * pJugLegalidad;
		BOOL	  bLegal;
		UINT32	  u32Legales = 0;

		for (pJugLegalidad = pJug2; pJugLegalidad >= pPos->pListaJug; pJugLegalidad--)
		{
			bLegal = TRUE;
			if (JugadaCorrecta(pPos, *pJugLegalidad) == FALSE)
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Cosa rarísima: la jugada es pseudo-ilegal");
				#endif
				bLegal = FALSE;
			}
			else if (Mover(pPos, pJugLegalidad) == FALSE)
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* La jugada es ilegal, la quitamos");
				#endif
				bLegal = FALSE;
			}

			if (bLegal)
				continue;
			else
			{
				// La jugada no ha pasado el corte
				if (pJugLegalidad == pJug2)
					pJug2--;
				else
				{
					*pJugLegalidad = *pJug2;
					pJug2--;
				}
			}
		}

		assert((pJug2 - pPos->pListaJug) >= 0);
		u32Legales = 1 + (pJug2 - pPos->pListaJug);
		#if (FICHERO_LOG == AV_LOG)
			ImprimirALog("*DBG* Jugadas legales... me salen %d", u32Legales);
		#endif
	}

	//
	// Veamos si disponemos de una JUGADA OBVIA
	// Pasos a seguir:
	//	- Asignamos valor de QSearch a cada jugada
	//	- Ordenamos -> la mejor, al final
	//	- Si la mejor jugada no supera a la siguiente en, al menos, 2 peones, pasamos
	//	- Si no es la jugada de la tabla hash, pasamos
	//
	dbDatosBusqueda.jugObvia.u32Mov = 0;
	pJug = pJug2;	// Apunta al final de la lista de jugadas
	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog("*DBG* Voy a comprobar si hay jugada obvia");
	#endif
	for (pJug = pJug2; pJug >= pPos->pListaJug; pJug--)
	{
		if (!Mover(pPos, pJug))
		{
			// Si la jugada no es legal, la quitamos (cambiándola por la que está en la cabeza) y probamos de nuevo
			// OJO: Con la comprobación de seguridad que he introducido más arriba, nunca debe ya entrar por aquí
			assert(FALSE);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("*DBG* Catástrofe 4");
			#endif
			ImprimirAPantalla("*DBG* Catástrofe 4");
			if (pJug == pJug2)
				pJug2--;
			else
			{
				*pJug = *pJug2;
				pJug2--;
				pJug++;	// porque el for hace --
			}
			continue;
		}
		(pPos+1)->pListaJug = pJug2 + 1;	// Porque Mover pone ese puntero justo detrás de la jugada, y me lía
		pJug->s32Val = -QSConJaques(pPos + 1, -INFINITO, +INFINITO, 1, QS_PROF_JAQUES);
	}
	if (pJug2 >= pJug)
	{
		Ordenar(pPos, pJug2);
		if (pJug2->s32Val >= 200 + (pJug2-1)->s32Val)
		{
			TNodoHash * pNodoHash = NULL;

			ConsultarHash(pPos, 1000, -INFINITO, INFINITO, &pNodoHash);
			TJugada jugHash = (pNodoHash == NULL ? JUGADA_NULA
												 : pNodoHash->jug);
			if (Jug_GetMov(jugHash) == Jug_GetMov(*pJug2))
				dbDatosBusqueda.jugObvia = *pJug2;
		}
	}

	dbDatosBusqueda.s32CambiosJugMejor = 0;

	//
	//
	// Bucle principal
	//
	//
	dbDatosBusqueda.jugJugadaRoot = JUGADA_NULA;
	for (s32Prof = 2; s32Prof <= MAX_PLIES; s32Prof++)
	{
		dbDatosBusqueda.s32ProfSel = 0;

		// Ordenar raíz y buscar
		Ordenar(pPos, pJug2);
		if (Jug_GetMov(dbDatosBusqueda.jugJugadaRoot))
			assert(Jug_GetMov(*pJug2) == Jug_GetMov(dbDatosBusqueda.jugJugadaRoot));

		if (s32Prof == 2) // Tras la primera iteración
			Jug_SetVal(pJug2, CONTADOR_ORDEN_ROOT_INI);

		dbDatosBusqueda.s32ProfRoot = s32Prof;
		s32EvalRootAnterior = dbDatosBusqueda.s32EvalRoot;
		
		Aspiracion(pPos, pJug2, s32Prof);

		// Salir si se abortó la búsqueda
		if (dbDatosBusqueda.bAbortar)
		{
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("*DBG* Saliendo de la búsqueda porque bandera de abortar es true");
			#endif
			break;
		}

		dbDatosBusqueda.s32CambiosJugMejor--;	// Cada iteración completada, decrementa el contador
		dbDatosBusqueda.s32CambiosJugMejor = max(dbDatosBusqueda.s32CambiosJugMejor, 0);

		// En partida debemos controlar el tiempo
		if (dbDatosBusqueda.eTipoBusqueda == TBU_PARTIDA)
		{
			// Si estoy viendo un mate, y lo veo más cerca que en la búsqueda anterior, no necesito analizar más
			// OJO: Debería quitar esto de la pila de evaluaciones, que no da más que problemas, y crear un nuevo atributo en dbDatosBusqueda que fuera s32EvalBusquedaAnterior,
			//	y que se actualizaría en ComprobarYMover, tras hacer la jugada propia
			if (Bus_DoyMatePronto(dbDatosBusqueda.s32EvalRoot) && s32Prof > 4)
			{
				SINT32 s32EvalBusquedaAnterior = dbDatosBusqueda.u32NumPlyPartida >= 2 ? as32PilaEvaluaciones[dbDatosBusqueda.u32NumPlyPartida - 2] : TABLAS;

				if (dbDatosBusqueda.s32EvalRoot > s32EvalBusquedaAnterior)
				{
					#if (FICHERO_LOG == AV_LOG)
						ImprimirALog("*DBG* Saliendo de la búsqueda porque veo un mate cada vez más cerca");
					#endif
					break;
				}
			}

			// ¿Jugada obvia?
			if (dbDatosBusqueda.jugObvia.u32Mov == dbDatosBusqueda.jugJugadaRoot.u32Mov && GetSuperadoCorteObvia(&g_tReloj))
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Saliendo de la búsqueda por jugada obvia");
				#endif
				break;
			}

			if (GetEsHoraDeParar(&g_tReloj, &dbDatosBusqueda, as32PilaEvaluaciones))
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Saliendo de la búsqueda porque es hora de parar");
				#endif
				break;
			}

			// Un pequeño intento de acabar plies
			// Habiendo cumplido la mitad del tiempo establecido sin caer la eval, acabamos
			SINT32 s32EvalAnterior = dbDatosBusqueda.u32NumPlyPartida > 1 ? as32PilaEvaluaciones[dbDatosBusqueda.u32NumPlyPartida - 2] : -INFINITO;
			if (GetSuperadoCorteMedNormal(&g_tReloj)
				&& dbDatosBusqueda.s32EvalRoot >= s32EvalRootAnterior
				&& dbDatosBusqueda.s32EvalRoot >= s32EvalAnterior)
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Saliendo de la búsqueda por haber pasado la mitad sin caer la eval");
				#endif
				break;
			}

			// Y ahora un intento de cortar en ply terminado
			// Si llevo 2/3 partes del tiempo asignado es poco probable que pueda terminar la PV siguiente, así que, si las cosas no van muy mal, corto
			if (GetSuperadoCorteDosTerNormal(&g_tReloj)
				&& dbDatosBusqueda.s32EvalRoot > s32EvalRootAnterior - 10
				&& dbDatosBusqueda.s32EvalRoot > s32EvalAnterior - 10)
			{
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Saliendo de la búsqueda por dos terceras partes sin problemas");
				#endif
				break;
			}
		}
	} // Bucle principal

fin_pensar:
	if (dbDatosBusqueda.eTipoBusqueda == TBU_PARTIDA)
	{
		// Guardamos la eval en este ply de la partida para la gestión del tiempo
		as32PilaEvaluaciones[dbDatosBusqueda.u32NumPlyPartida] = dbDatosBusqueda.s32EvalRoot;

		ComprobarYMover(pPos);
		DetenerReloj(&g_tReloj);
		// 09/03/25 0.74
		if (bHayJugadaEGTB)
			ImprimirPlyConPV(pPos, s32Prof, dbDatosBusqueda.s32EvalRoot);
	}

	// Enviar PV a ICS si procede

	// Si he terminado la búsqueda, y estoy en modo ponder, significa que he cortado por algo forzado, como un mate, ahogado, tercera repetición, etc.
	// En ese caso, no quiero que lance otro ciclo de ponder, obviamente
	if (dbDatosBusqueda.eTipoBusqueda == TBU_PONDER)
		dbDatosBusqueda.bOtroCicloPonder = FALSE;
	else
		ImprimirResumenBusqueda(s32Prof);
	dbDatosBusqueda.eEstadoBusqueda = BUS_ESPERANDO;
}