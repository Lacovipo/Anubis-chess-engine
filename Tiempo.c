
#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"
#include "Inline.h"
#include <time.h>

/*
 *
 * DeterminarTiempoBusqueda
 *
 *
 * Descripción: Calcula los tiempos normal y extendido para la búsqueda. También incrementa los contadores adecuados,
 *  considerando que esta función es llamada siempre antes de iniciar una búsqueda.
 *
 */
void DeterminarTiempoBusqueda(void)
{
	SINT32 s32NumJugadasQuedan;
	SINT64 s64TiempoEnMilisegundos;

	#if (FICHERO_LOG	== AV_LOG)
		ImprimirALog(" ");
		ImprimirALog(" --------------------- DeterminarTiempoBusqueda --------------------- ");
	#endif

	if (g_tReloj.m_s32NumJugadasControl < 0)
		g_tReloj.m_s32NumJugadasControl = 0;

	//
	// Número de jugadas que estimamos que nos quedan para usar en ellas el tiempo disponible
	//
	s32NumJugadasQuedan = dbDatosBusqueda.u32NumPlyPartida > 80 ? 10
																: (100 - dbDatosBusqueda.u32NumPlyPartida) / 2;

	//
	// Si estamos en un control con número de jugadas, contraponemos ese dato a la estimación genérica
	//
	if (g_tReloj.m_s32NumJugadasControl > 0)
	{
		SINT32 s32NumJugadasQuedan2 = (1 + 2 * g_tReloj.m_s32NumJugadasControl - dbDatosBusqueda.u32NumPlyPartida) / 2;
		// Si es un valor negativo, significa que ya hemos pasado el primer control; en ese caso, opto por regirme por la estimación genérica
		// En caso contrario, me interesa el valor más bajo (no me interesa, por ejemplo, que se crea que le quedan 60 jugadas cuando ya hemos pasado la jugada 60 una vez)
		if (s32NumJugadasQuedan2 > 0)
			s32NumJugadasQuedan = min(s32NumJugadasQuedan, s32NumJugadasQuedan2);
	}
	// Pequeño control de seguridad, para no comerme nunca todo el tiempo que queda en una jugada
	if (s32NumJugadasQuedan == 1)
		s32NumJugadasQuedan = 2;

	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog(" u32NumPlyPartida            : %d", dbDatosBusqueda.u32NumPlyPartida);
		ImprimirALog(" s32NumJugadasQuedan         : %d", s32NumJugadasQuedan);
	#endif

	// 09/01/25 Se ha dado algún caso anómalo, por error del GUI o lo que sea, que se toman tiempos negativos, así que asumo que siempre me queda, al menos, 1 segundo
	// 22/08/25 Algunas personas están usando controles de tiempo muy cortos, así que no voy a reservar un segundo entero, sino una décima (100 milésimas)
	if (g_tReloj.m_s32MilisegundosDisponibles < 100)
	{
		g_tReloj.m_s32MilisegundosDisponibles = 100;
		#if (FICHERO_LOG == AV_LOG)
			ImprimirALog("OJO: Reajuste a 1 décima");
		#endif		
	}

	//
	// Obvia y normal
	//

	// Tiempo a: sin contar el incremento
	s64TiempoEnMilisegundos = g_tReloj.m_s32MilisegundosDisponibles / s32NumJugadasQuedan;
	#if (FICHERO_LOG	== AV_LOG)
		ImprimirALog(" s64TiempoEnMilisegundos a   : %lld", s64TiempoEnMilisegundos);
	#endif

	// Asigno tiempo de obvia y normal
	g_tReloj.m_s64TiempoCorteObvia = g_tReloj.m_s64MomentoInicioBusqueda + (SINT64)(s64TiempoEnMilisegundos / 10);
	g_tReloj.m_s64TiempoCorteNormal = g_tReloj.m_s64MomentoInicioBusqueda + s64TiempoEnMilisegundos;
	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog(" m_s64TiempoCorteObvia       : %lld", g_tReloj.m_s64TiempoCorteObvia);
		ImprimirALog(" m_s64TiempoCorteNormal      : %lld", g_tReloj.m_s64TiempoCorteNormal);
	#endif

	//
	// Tiempo extendido
	//
	s64TiempoEnMilisegundos *= 6;
	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog(" s64TiempoEnMilisegundos c   : %lld", s64TiempoEnMilisegundos);
	#endif
	if (s64TiempoEnMilisegundos > 0.95 * g_tReloj.m_s32MilisegundosDisponibles)
		s64TiempoEnMilisegundos = (SINT64)(0.95 * g_tReloj.m_s32MilisegundosDisponibles);
	#if (FICHERO_LOG == AV_LOG)
		ImprimirALog(" s64TiempoEnMilisegundos d   : %lld", s64TiempoEnMilisegundos);
	#endif
	g_tReloj.m_s64TiempoCorteExtendido = g_tReloj.m_s64MomentoInicioBusqueda + s64TiempoEnMilisegundos;
	#if (FICHERO_LOG	== AV_LOG)
		ImprimirALog(" m_s64TiempoCorteExtendido   : %lld", g_tReloj.m_s64TiempoCorteExtendido);
		ImprimirALog(" --------------------- Fin DeterminarTiempoBusqueda --------------------- ");
		ImprimirALog(" ");
	#endif
}
