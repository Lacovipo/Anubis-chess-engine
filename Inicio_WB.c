/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de las tareas
	a realizar en la inicialización del
	programa que son particulares al modo
	winboard, así como la comunicación con
	la interfaz (este es el thread de E/S)
*/

#include "Preprocesador.h"

#pragma warning(disable : 4115) // Warning en las librerías de Windows
#include <windows.h>
#include "Tipos.h"
#include "Constantes.h"
#include "Variables.h"
#include "Funciones.h"
#include "Inline.h"
#include "tbprobe.h"
#include "nnue.h"
#include "GPT_Magic.h"

void InicioWB(int argc, char * argv[])
{
	char		szComando[256];
	UINT32		u32Protover = 0;
	HANDLE		hMotor = NULL;	// handle al thread del motor
	TPosicion * pPos = aPilaPosiciones + 4; // Puntero a la posición actual SOBRE EL TABLERO
	DWORD		dwMotor;
	BOOL		bChikkiInicializado = FALSE;

	#if (FICHERO_LOG == AV_LOG)
		AbrirFicheroLog();
		ImprimirALog("%s",VERSION);
		ImprimirALog("(c) Jose Carlos Martinez Galan\n");
	#endif

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	// Datos precomputados
	InicializarDatosPrecomputados();
	// Valores hash aleatorios
	InicializarRandomHash();
	// Personaje
	if (!DimensionarTablasHash())
		Salir();

	// NNUE
	if (!load_weights("net00005_T0.bin", &nnue_weights_T0))
		Salir();
	if (!load_weights("net00005_T1.bin", &nnue_weights_T1))
		Salir();
	if (!load_weights("net00005_T2.bin", &nnue_weights_T2))
		Salir();
	if (!load_weights("net00005_T3.bin", &nnue_weights_T3))
		Salir();
	if (!load_weights("net00005_T4.bin", &nnue_weights_T4))
		Salir();
	if (!load_weights("net00005_T5.bin", &nnue_weights_T5))
		Salir();
	if (!load_weights("net00005_T6.bin", &nnue_weights_T6))
		Salir();
	if (!load_weights("net00005_T7.bin", &nnue_weights_T7))
		Salir();
	if (!load_weights("net00005_T8.bin", &nnue_weights_T8))
		Salir();
	if (!load_weights("net00005_T9.bin", &nnue_weights_T9))
		Salir();

	//if (!nnue_cargar_pesos("net00004_TX.bin"))
	//	Salir();
	printf("#NNUE cargado\n");

	// Magic
	InitMagicBitboards();

	// Historia
	Inicializar_Historia();

	//
	// Establecemos la posición inicial y nos situamos en la base de la pila
	// de posiciones
	//
	SetBoard(aPilaPosiciones,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	*(aPilaPosiciones + 1) = *aPilaPosiciones;
	*(aPilaPosiciones + 2) = *aPilaPosiciones;
	*(aPilaPosiciones + 3) = *aPilaPosiciones;
	*(aPilaPosiciones + 4) = *aPilaPosiciones;

	//
	// Por consideraciones de seguridad, exigimos (el protocolo así lo dice)
	// que el primer comando pasado al programa sea "xboard"
	//
	scanf_s("%s", szComando, 255);
	if (strcmp(szComando,"xboard"))
	{
		ImprimirAPantalla("telluser Error en protocolo: El primer comando debe ser 'xboard'");
		Salir();
	}
	// Y, a continuación, debe venir "protover"
	scanf_s("%s", szComando, 255);
	while(strcmp(szComando, "protover"))
		scanf_s("%s", szComando, 255);
	scanf_s("%s", szComando, 255);
	u32Protover = atoi(szComando);
	if (u32Protover < 2)
	{
		ImprimirAPantalla("telluser Error en protocolo: Debe ser 2 o superior");
		Salir();
	}

	// Lanzar el thread del motor
	ImprimirAPantalla("feature done=0");
	hMotor = CreateThread((LPSECURITY_ATTRIBUTES)NULL,0,(LPTHREAD_START_ROUTINE)MotorThread,NULL,0,&dwMotor);
	if (hMotor == NULL)
	{
		ImprimirAPantalla("telluser Error: No se puede cargar el motor");
		Salir();
	}

	ImprimirAPantalla("feature ping=1 setboard=1 playother=1 san=0 usermove=1 time=1 draw=0 sigint=0 sigterm=0 reuse=0 analyze=1 myname=\"%s\" variants=\"normal\" colors=0 ics=1 name=1 pause=0 feature option=\"SyzygyPath - path c:\\ajedrez\\TB_syzygy\" done=1", VERSION);
	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog("feature ping=1 setboard=1 playother=1 san=0 usermove=1 time=1 draw=0 sigint=0 sigterm=0 reuse=0 analyze=1 myname=\"%s\" variants=\"normal\" colors=0 ics=1 name=1 pause=0 feature option=\"SyzygyPath - path c:\\ajedrez\\TB_syzygy\" done=1", VERSION);
	#endif

	//
	// Inicialización de algunos parámetros que podrían no ser modificados (no
	// recibir comandos relacionados)
	//
	dpuDatos.bComputerOponente = FALSE;
	dpuDatos.bEnICS = FALSE;
	dpuDatos.szNombreOponente[0] = '\0';
	dpuDatos.u32RatingAnubis = 0;
	dpuDatos.u32RatingOponente = 0;

	/*
		+-----------------------------------------------------------+
		|	Bucle infinito de entrada de comandos en modo winboard	|
		+-----------------------------------------------------------+
	*/
	while(TRUE)
	{
		scanf_s("%s", szComando, 255);
		#if (FICHERO_LOG == AV_LOG)
			ImprimirALog("\n<%s",szComando);
		#endif
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando, "option"))
		{
			gets_s(szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s", szComando);
			#endif

			if (strncmp(szComando, " SyzygyPath", 11) == 0)
			{
				char * szPath;
				szPath = szComando + 12;

				//
				// EGTB (chikki)
				//
				if (bChikkiInicializado)
					tb_free();

				if (tb_init(szPath))
				{
					ImprimirAPantalla("EGTBs cargadas");
					#if (FICHERO_LOG == AV_LOG)
						ImprimirALog("EGTBs cargadas");
					#endif
					bChikkiInicializado = TRUE;
				}
				else
				{
					ImprimirAPantalla("No se pudo cargar EGTBs");
					#if (FICHERO_LOG == AV_LOG)
						ImprimirALog("No se pudo cargar EGTBs");
					#endif
					bChikkiInicializado = FALSE;
				}					
			}
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"playother"))
		{
			EnviarComandoAMotor(COM_STOP,0,0,NULL);
			while (dbDatosBusqueda.eEstadoBusqueda != BUS_ESPERANDO)
					Sleep(100);
			dbDatosBusqueda.u32ColorAnubis = !Pos_GetTurno(pPos);
			// Si ponder está activo, ponerse a ponderar ahora
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"usermove"))
		{
			TJugada jug;

			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif

			ComputarBB(pPos); // 03/08/25 3.01
			jug = Algebraica2Jug(szComando);
			if (JugadaCorrecta(pPos,jug))
			{
				dbDatosBusqueda.u32NumPlyPartida++;
				if (dbDatosBusqueda.eEstadoBusqueda == BUS_ESPERANDO)
				{
					//
					// El motor está parado (no estamos ponderando)
					//
					pPos->pListaJug[0] = jug;
					Mover(pPos, pPos->pListaJug);
					pPos++;
					ComputarBB(pPos); // 01/08/25 3.01

					as32PilaEvaluaciones[dbDatosBusqueda.u32NumPlyPartida] = INFINITO; // para que la primera vez que piense tras el libro, sea larga

					if (dbDatosBusqueda.u32ColorAnubis == Pos_GetTurno(pPos))
					{
						EnviarComandoAMotor(COM_GO, pPos, 0, NULL);
						pPos++;
					}
				} 
				else // if (dbDatosBusqueda.eEstadoBusqueda == BUS_ESPERANDO)
				{
					if (dbDatosBusqueda.eTipoBusqueda == TBU_ANALISIS)
					{
						//
						// Si estamos analizando y recibimos una jugada, paramos, la hacemos y seguimos en búsqueda
						//
						EnviarComandoAMotor(COM_STOP,0,0,NULL);
						while (dbDatosBusqueda.eEstadoBusqueda != BUS_ESPERANDO)
							Sleep(100);
						pPos->pListaJug[0] = jug;
						Mover(pPos,pPos->pListaJug);
						pPos++;
						EnviarComandoAMotor(COM_GO, pPos, 0, NULL);
					}
					else // if (dbDatosBusqueda.eTipoBusqueda == TBU_ANALISIS)
					{
						//
						// El motor está ocupado (ponderando). La idea aquí es aprovechar, si estamos ponderando sobre la jugada correcta,
						// y seguir como si nada. En caso contrario, pararé y empezaré una búsqueda desde cero, peladita
						//
						if (Jug_GetMov(jug) == Jug_GetMov(dbDatosBusqueda.jugJugadaPonder))
						{
							//
							// Esto es lo más delicado: hemos acertado en la predicción.
							// Quiero que el motor siga analizando como si nada, ajustando
							// el tiempo debidamente, pero es necesario que el puntero de
							// posición de la partida se incremente más allá de la jugada
							// ponderada
							//
							#if (FICHERO_LOG == AV_LOG)
								ImprimirALog("*DBG* Jugada ponder acertada");
							#endif

							pPos++; pPos++; // Dos veces, porque voy a lanzar otro ciclo ponder
							dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;
							dbDatosBusqueda.eTipoBusqueda = TBU_PARTIDA;
							dbDatosBusqueda.bOtroCicloPonder = TRUE;

							// Recalcular tiempos, porque justo antes de esta jugada ponder me ha venido el comando "time" con el nuevo tiempo exacto disponible
							DeterminarTiempoBusqueda();
							// Quiero resetear el reloj porque arena, a veces, falla y no me manda el comando time, y quiero saber el tiempo exacto que me queda
							PonerRelojEnMarcha(&g_tReloj);
							#if (FICHERO_LOG == AV_LOG)
								ImprimirALog("*DBG* Reloj puesto en marcha (ponder acertado)");
							#endif

						} // if (Jug_GetMov(jug) == Jug_GetMov(dbDatosBusqueda.jugJugadaPonder))
						else
						{
							//
							// No acertamos en la predicción (o no estábamos ponderando, sino
							// en modo análisis). Paramos el motor, hacemos la
							// jugada y le ordenamos que analice
							//
							#if (FICHERO_LOG == AV_LOG)
								ImprimirALog("*DBG* Jugada ponder no acertada");
							#endif

							EnviarComandoAMotor(COM_STOP,0,0,NULL);
							while (dbDatosBusqueda.eEstadoBusqueda != BUS_ESPERANDO)
								Sleep(100);
							pPos->pListaJug[0] = jug;
							dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;
							dbDatosBusqueda.eTipoBusqueda = TBU_PARTIDA;
							Mover(pPos,pPos->pListaJug);
							pPos++;

							if (dbDatosBusqueda.u32ColorAnubis == Pos_GetTurno(pPos))
							{
								EnviarComandoAMotor(COM_GO, pPos, 0, NULL);
								pPos++;
							}
						} // else [if (Jug_GetMov(jug) == Jug_GetMov(dbDatosBusqueda.jugJugadaPonder))]
					}
				} // else [if (dbDatosBusqueda.eEstadoBusqueda == BUS_ESPERANDO)]
			}
			else
				ImprimirAPantalla("Illegal move: %s",szComando);

			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"go"))
		{
			//
			// Cuando recibimos este comando, no debemos estar ponderando ni
			// pensando, pues no tiene sentido. El protocolo dice que "go" saca
			// al programa del modo "force". No obstante, me curo en salud y
			// mando al motor la orden de deternerse antes de procesar el "go",
			// por si algún GUI me hiciese una jugarreta
			//

			// Paro la búsqueda y dejo al motor en estado de no pensar nada
			EnviarComandoAMotor(COM_STOP,0,0,NULL);
			while (dbDatosBusqueda.eEstadoBusqueda != BUS_ESPERANDO)
					Sleep(100);

			dbDatosBusqueda.eEstadoBusqueda = BUS_NORMAL;
			dbDatosBusqueda.eTipoBusqueda = TBU_PARTIDA;
			dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;

			EnviarComandoAMotor(COM_GO, pPos, 0, NULL);
			pPos++;
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"otim"))
		{
			//
			// El tiempo del contrario no me interesa, pues no quiero tomar
			// decisiones extrañas basadas en una posible victoria por tiempo
			//
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"time"))
		{
			//
			// No paramos el motor, puesto que time puede llegar mientras ponderamos
			// y sólo nos indica cuánto nos queda de tiempo de partida (en centisegundos)
			//
			scanf_s("%s", szComando, 255);
			g_tReloj.m_s32MilisegundosDisponibles = 10 * atoi(szComando);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"rating"))
		{
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			dpuDatos.u32RatingAnubis = atoi(szComando);

			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			dpuDatos.u32RatingOponente = atoi(szComando);
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"computer"))
		{
			//
			// Este comando me indica que me enfrento a otro programa,
			// especialmente importante en servidores de ajedrez, donde
			// esto condiciona el "kibitzing".
			//
			dpuDatos.bComputerOponente = TRUE;
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"force"))
		{
			//
			// En modo force, el programa no debe pensar ni ponderar, aunque sí
			// aceptar movimientos y comprobar su legalidad
			//

			// Caso de error: estamos ponderando, el rival hace la jugada ponder (pero aún no lo sabemos), el GUI detecta que tenemos jugada de libro, nos manda force y usermove.
			// Como acertamos la jugada ponder, si el usermove llega a tiempo, damos por buena la ponderación y contestamos con nuestra jugada, el GUI nos manda la jugada libro,
			// que ya no es legal para nosotros, y petamos
			if (dbDatosBusqueda.eTipoBusqueda == TBU_PONDER)
			{
				dbDatosBusqueda.eTipoBusqueda = TBU_FORCE;
				dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;
				dbDatosBusqueda.u32ColorAnubis = COLOR_INDEFINIDO;
			}

			// Paro la búsqueda y dejo al motor en estado de no pensar nada
			EnviarComandoAMotor(COM_STOP,0,0,NULL);
			dbDatosBusqueda.u32ColorAnubis = COLOR_INDEFINIDO;
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando, "result"))
		{
			// Sacabao la partida; me la sopla el resultado
			dbDatosBusqueda.bAbortar = TRUE;
			EnviarComandoAMotor(COM_STOP, 0, 0, NULL);
			dbDatosBusqueda.u32ColorAnubis = COLOR_INDEFINIDO;
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"ping"))
		{
			UINT32 u32Pong;

			//
			// Sincronización. Devolvemos el correspondiente pong sin molestar al motor
			// IMPORTANTE: El protocolo especifica que todos los comandos
			//	previos deben ser procesados antes de responder con el pong.
			//	Dada la estructura de Anubis, esto está garantizado. En caso
			//	de no ocurrir, delatará un bug, pues el diseño así lo exige
			//
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			u32Pong = atoi(szComando);
			ImprimirAPantalla("pong %d",u32Pong);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" pong %d",u32Pong);
			#endif
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"name"))
		{
			//
			// Nombre del oponente
			//
			gets_s(dpuDatos.szNombreOponente, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",dpuDatos.szNombreOponente);
			#endif
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"easy"))
		{
			//
			// Indica al programa que NO debe ponderar.
			// No es preciso parar el motor para esto. Lo dejo ahí y, llegado
			// el momento, él dejará de ponderar
			// El protocolo no especifica nada sobre qué hacer si se recibe
			// este comando mientras ponderamos. Mi opción es no detener la
			// ponderación
			//
			dbDatosBusqueda.bPonderar = FALSE;
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"hard"))
		{
			//
			// Indica al programa que debe ponderar.
			// No es preciso parar el motor para esto. Lo dejo ahí y, llegado
			// el momento, él empezará a ponderar solito
			//
			dbDatosBusqueda.bPonderar = TRUE;
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"post"))
		{
			//
			// El comando post sirve para indicar que el usuario quiere ver
			// información de búsqueda. Para desactivarlo, winboard envía nopost
			// Yo acepto sólo post, para que wb sepa que voy a enviar la info siempre
			//
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"level"))
		{
			UINT32	u32;
			char	sz[256];

			//
			// Paramos el motor porque estamos modificando un parámetro fundamental
			// de la partida, como es el control de tiempo
			//
			EnviarComandoAMotor(COM_STOP,0,0,NULL);

			//
			// Número de jugadas en el control de tiempo
			//
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			g_tReloj.m_s32NumJugadasControl = atoi(szComando);
			//
			// Minutos / segundos por control de tiempo
			//
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			for (u32 = 0; u32 < strlen(szComando); u32++)
			{
				if (szComando[u32] == ':')
					break;
			}
			if (u32 == strlen(szComando))
				g_tReloj.m_s32NumSegundosControl = 60 * atoi(szComando);
			else
			{
				strcpy_s(sz, 255, &szComando[u32+1]);
				szComando[u32] = '\0';
				g_tReloj.m_s32NumSegundosControl = 60 * atoi(szComando);
				g_tReloj.m_s32NumSegundosControl += atoi(sz);
			}
			g_tReloj.m_s32MilisegundosDisponibles = 1000 * g_tReloj.m_s32NumSegundosControl;
			//
			// Incremento
			//
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			g_tReloj.m_s32NumSegundosIncremento = atoi(szComando);
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"new"))
		{
			//
			// EGTB (chikki)
			//
			// Si todavía no he inicializado el chikki path, lo inicializo ahora con un valor por defecto
			if (!bChikkiInicializado)
			{
				if (tb_init("c:\\Ajedrez\\TB_syzygy;e:\\Ajedrez\\TB_syzygy;c:\\ajedrez\\syzygy"))
				{
					ImprimirAPantalla("#EGTBs cargadas");
					#if (FICHERO_LOG == AV_LOG)
						ImprimirALog("EGTBs cargadas");
					#endif
					bChikkiInicializado = TRUE;
				}
				else
				{
					ImprimirAPantalla("#No se pudo cargar EGTBs");
					#if (FICHERO_LOG == AV_LOG)
						ImprimirALog("No se pudo cargar EGTBs");
					#endif
					bChikkiInicializado = FALSE;
				}
			}

			// Desactivo el ponder para que, al parar la búsqueda, se quede en espera
			dbDatosBusqueda.bPonderar = FALSE;
			// Paro la búsqueda
			EnviarComandoAMotor(COM_STOP,0,0,NULL);
			while (dbDatosBusqueda.eEstadoBusqueda != BUS_ESPERANDO)
					Sleep(100);
			// Pongo la posición inicial
			SetBoard(aPilaPosiciones + 4,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			pPos = aPilaPosiciones + 4;
			// Datos generales de búsqueda
			dbDatosBusqueda.u32ColorAnubis = NEGRAS;
			dbDatosBusqueda.eEstadoBusqueda = BUS_ESPERANDO;
			dbDatosBusqueda.eTipoBusqueda = TBU_PARTIDA;
			dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;
			dbDatosBusqueda.bPonderar = TRUE;
			// Borrar tabla hash para la nueva partida (aunque hemos dicho que no queremos reuse, pero algunos GUIs pasan)
			memset(aTablaHash, 0, u32TamTablaHash * sizeof(TNodoHash));
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"analyze"))
		{
			EnviarComandoAMotor(COM_STOP,0,0,NULL);
			while (dbDatosBusqueda.eEstadoBusqueda != BUS_ESPERANDO)
					Sleep(100);

			dbDatosBusqueda.eTipoBusqueda = TBU_ANALISIS;
			dbDatosBusqueda.u32ColorAnubis = COLOR_INDEFINIDO;

			EnviarComandoAMotor(COM_GO, pPos, 0, NULL);
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"setboard"))
		{
			EnviarComandoAMotor(COM_STOP,0,0,NULL);

			dbDatosBusqueda.eTipoBusqueda = TBU_ANALISIS;
			dbDatosBusqueda.u32ColorAnubis = COLOR_INDEFINIDO;

			gets_s(szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			SetBoard(aPilaPosiciones + 4,szComando);
			pPos = aPilaPosiciones + 4;
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"accepted"))
		{
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			// Me ha aceptado esta feature, así que no tengo que hacer nada
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"rejected"))
		{
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			// Me ha rechazado esta feature. Por ahora, ignoraré los rechazos
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (szComando[0] == '{')
		{
			//
			// Comentario tras el final de una partida. Ignoro el resto de la línea
			//
			gets_s(szComando, 255);
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"quit"))
		{
			// Guardar el libro antes de salir
			// ...

			// Paro la búsqueda, espero un tiempo prudencial y salgo
			EnviarComandoAMotor(COM_STOP,0,0,NULL);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("\nSaliendo del programa...",szComando);
			#endif
			// Esperamos a que se pare el motor
			while (dbDatosBusqueda.eEstadoBusqueda != BUS_ESPERANDO)
				Sleep(100);
			Salir();
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"?"))
		{
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("Recibido comando '?'. Intentando parar el motor");
			#endif
			EnviarComandoAMotor(COM_STOP, 0, 0, NULL);
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"ics"))
		{
			scanf_s("%s", szComando, 255);
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog(" %s",szComando);
			#endif
			if (!strcmp(szComando,"-"))
				dpuDatos.bEnICS = FALSE;
			else
				dpuDatos.bEnICS = TRUE;
			continue;
		}
		////////////////////////////////////////////////////////////////////
		if (!strcmp(szComando,"exit"))
		{
			if (dbDatosBusqueda.eTipoBusqueda == TBU_ANALISIS)
			{
				EnviarComandoAMotor(COM_STOP,0,0,NULL);
				// Esperamos a que se pare el motor
				while (dbDatosBusqueda.eEstadoBusqueda != BUS_ESPERANDO)
					Sleep(100);
				dbDatosBusqueda.eTipoBusqueda = TBU_PARTIDA;
			}
			continue;
		}
		////////////////////////////////////////////////////////////////////
		ImprimirAPantalla("Error (unknown command): %s",szComando);
	}
}
