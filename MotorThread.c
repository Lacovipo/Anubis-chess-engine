/*
	Anubis

	José Carlos Martínez Galán
*/

// Include pthread.h for portability and remove Windows.h
#ifdef _MSC_VER
	#pragma warning(disable : 4115) // Warning en las librerías de Windows
	#include <windows.h>
#else
	#include <pthread.h>
	#include <unistd.h> // For usleep/nanosleep
	#include <time.h>   // For nanosleep
#endif
#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"
#include "Inline.h"
#include "Funciones.h"

/*
	*********************************************************************
	*																	*
	*	ProcesarComando													*
	*																	*
	*																	*
	*	Devuelve: TRUE si ha encontrado (y procesado) algún comando		*
	*			  que exiga detener la búsqueda.						*
	*			  FALSE en caso contrario.								*
	*																	*
	*	Descripción: Consulta la cola de comandos en buscar de alguno	*
	*				 nuevo. Mientras encuentre comandos nuevos los		*
	*				 va ejecutando.										*
	*				 12/01/03 - Quito la consulta de toda la cola, pues	*
	*					quiero que, en caso de encontrar un comando,	*
	*					tome las acciones pertinentes y vuelva a la		*
	*					búsqueda. El comando más frecuente será el de	*
	*					parar la búsqueda, así que cancelo la búsqueda	*
	*					antes de continuar procesando comandos			*
	*																	*
	*********************************************************************
*/
BOOL ProcesarComando(void)
{
	static  UINT32		u32UltComandoLeido  = 0;
	static  TPosicion *	pPos              = aPilaPosiciones;	// El puntero que siempre indica la sig pos a buscar
	BOOL    bDetenerBusqueda              = FALSE;

	if (u32UltComandoEnviado != u32UltComandoLeido)
	{
		u32UltComandoLeido++;
		if (u32UltComandoLeido >= MAX_COMANDOS)
			u32UltComandoLeido = 0;
		//
		// Leer y ejecutar el comando
		//
		switch (acmListaComandos[u32UltComandoLeido].u32Codigo)
		{
			case COM_GO:
				#if (FICHERO_LOG	== AV_LOG)
					ImprimirALog("*DBG* Recibido comando GO");
				#endif
				if (acmListaComandos[u32UltComandoLeido].u32Param1)
					pPos = (TPosicion *)acmListaComandos[u32UltComandoLeido].u32Param1;
				dbDatosBusqueda.u32ColorAnubis = Pos_GetTurno(pPos);
				dbDatosBusqueda.eEstadoBusqueda = BUS_NORMAL;
				Pensar(pPos++);
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Finalizado pensar");
				#endif
				// Si hay que ponderar, pongo aquí la jugadita. Luego, desde fuera,
				// se me dará la orden de ponderar
				do
				{
					dbDatosBusqueda.eEstadoBusqueda = BUS_ESPERANDO;
					// 30/03/2012 - Voy a probar, a ver si no lo reviento, a ponderar aquí
					if (dbDatosBusqueda.bPonderar && JugadaCorrecta(pPos,dbDatosBusqueda.jugJugadaPonder))
					{
						dbDatosBusqueda.bOtroCicloPonder = FALSE;
						dbDatosBusqueda.eEstadoBusqueda = BUS_NORMAL;
						dbDatosBusqueda.eTipoBusqueda = TBU_PONDER;

						pPos->pListaJug[0] = dbDatosBusqueda.jugJugadaPonder;
						if (Mover(pPos, pPos->pListaJug))
						{
							pPos++;

							#if (FICHERO_LOG == AV_LOG)
								ImprimirALog("*DBG* Voy a ponderar");
							#endif

							Pensar(pPos++);

							#if (FICHERO_LOG == AV_LOG)
								ImprimirALog("*DBG* Finalizado pensar de ponderar");
							#endif

							if (!JugadaCorrecta(pPos,dbDatosBusqueda.jugJugadaPonder))
								dbDatosBusqueda.bOtroCicloPonder = FALSE;
							if (dbDatosBusqueda.bOtroCicloPonder)
								#if (FICHERO_LOG == AV_LOG)
									ImprimirALog("*DBG* Voy a lanzar otro ciclo de ponder");
								#endif
								// Si he fallado la predicción, me sacarán de aquí; si he acertado, la terminación de esta búsqueda
								// debe iniciar otro ciclo de ponder
								;
						}
						else
						{
							pPos->pListaJug[0] = dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;

							#if (FICHERO_LOG == AV_LOG)
								ImprimirALog("*DBG* No voy a ponderar porque la jugada ponder es chunga");
							#endif
						}
					}
				} while (dbDatosBusqueda.bOtroCicloPonder);

				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Salgo del ciclo de ponder");
				#endif

				dbDatosBusqueda.eTipoBusqueda = TBU_PARTIDA; // OJO: lo pongo para ver si me quito las pérdidas por bandera
				break;
			case COM_STOP:
				#if (FICHERO_LOG == AV_LOG)
					ImprimirALog("*DBG* Recibido comando STOP");
				#endif

				dbDatosBusqueda.bAbortar = TRUE;
				bDetenerBusqueda = TRUE;
				break;
			default:
				UNREACHABLE();
				break;
		}

		// 25/04/2012 - La siguiente instrucción no parece tener ningún sentido, salvo que recibamos un comando mientras
		//  estamos procesando otro. Supongamos que estamos procesando el último (MAX_COMANDOS-1) y recibimos un comando
		//  del otro thread; sumaríamos 1, le daríamos la vuelta a la cola (iríamos al 0) y lo procesaríamos; al salir
		//  de aquí, el valor que veríamos en la variable u32UltComandoLeido sería 0. Por tanto, no veo cómo puede ser
		//  mayor que el máximo. Así que le voy a poner una anotación en el log, por si pasa...
		if (u32UltComandoLeido >= MAX_COMANDOS)
		{
			#if (FICHERO_LOG == AV_LOG)
				ImprimirALog("*DBG* Mala cosa, nos hemos pasado de MAX_COMANDOS-1");
			#endif
			u32UltComandoLeido = 0;
		}
	}

	return (bDetenerBusqueda);
}

/*
	*********************************************************************
	*																	*
	*	EnviarComandoAMotor												*
	*																	*
	*																	*
	*	Recibe: Los componentes de un registro de comando del motor		*
	*																	*
	*	Descripción: gestiona los índices de la cola de comandos		*
	*																	*
	*********************************************************************
*/
void EnviarComandoAMotor(UINT32 u32Codigo,TPosicion * u32Param1,UINT32 u32Param2,char * szParam)
{
	// El "-1" es poco intuitivo, pero es correcto
	UINT32	u32NuevoCom = u32UltComandoEnviado >= (MAX_COMANDOS - 1) ? 0 : u32UltComandoEnviado + 1;

	acmListaComandos[u32NuevoCom].u32Codigo = u32Codigo;
	acmListaComandos[u32NuevoCom].u32Param1 = u32Param1;
	acmListaComandos[u32NuevoCom].u32Param2 = u32Param2;
	if (szParam != NULL)
		// Use strncpy for a safer, portable alternative to strcpy_s
		strncpy((char*)acmListaComandos[u32NuevoCom].szParam, szParam, 1023);

	u32UltComandoEnviado = u32NuevoCom;
}

/*
	*************************************************************************
	*																		*
	*	MotorThread															*
	*																		*
	*																		*
	*	Descripción: Bucle que mantiene con vida el thread del motor.		*
	*				 Comprueba constantemente la aparición de un nuevo		*
	*				 comando en la cola.									*
	*				 Tras cada consulta, espera una décima de segundo		*
	*				 antes de realizar la siguiente, para no bloquear la	*
	*				 CPU.													*
	*																		*
	*************************************************************************
*/
#ifdef _MSC_VER
UINT32 __stdcall MotorThread(void)
{
	#pragma warning (disable: 4127)
	while (TRUE)
	{
		ProcesarComando();
		Sleep(100);
	}
	#pragma warning (default: 4127)
}
#else
// Change the function signature for pthreads compatibility
void* MotorThread(void* arg)
{
#pragma warning (disable: 4127)
	while (TRUE)
	{
		ProcesarComando();
		// Use nanosleep for a portable, high-resolution sleep
		nanosleep(&(struct timespec) { 0, 100000000 }, NULL);
	}
#pragma warning (default: 4127)
	return NULL;
}
#endif