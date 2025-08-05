/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de las funciones
	relacionadas con el envío de datos a la
	pantalla (stdout)
*/
#include "Preprocesador.h"
#include <stdarg.h>
#include <stdio.h>
#include "Funciones.h"
#include "Variables.h"

void __cdecl ImprimirAPantalla(const char * szFmt, ...)
{
	char	aszBuf[1024];
	va_list	lpArgPtr;

	va_start(lpArgPtr, szFmt);
	vsprintf(aszBuf, szFmt, lpArgPtr);
	printf("%s\n", aszBuf);
	fflush(stdout);
}

#if (FICHERO_LOG == AV_LOG)
void __cdecl ImprimirALog(const char * szFmt, ...)
{
	char	aszBuf[1024];
	va_list	lpArgPtr;

	va_start(lpArgPtr, szFmt);
	vsprintf(aszBuf, szFmt, lpArgPtr);
	fprintf(pFLog,"%s\n", aszBuf);
	fflush(stdout);
}
#endif

/*
 * Envía a la salida estándar una cadena en formato del GUI que nos esté soportando, es decir, protocolo Winboard o consola
 * El tiempo lo calculamos aquí (tomamos el actual)
 * El número de nodos lo tomamos de dbDatosBusqueda
 *
 */
void ImprimirCadenaGUI(SINT32 s32Prof, SINT32 s32Eval, char * szPV)
{
	UINT32 u32Centisegundos = GetCentisegundosTranscurridos(&g_tReloj);

	ImprimirAPantalla("%u %d %u %u \t%s", (UINT32)s32Prof, s32Eval, u32Centisegundos, dbDatosBusqueda.u32NumNodos, szPV);
	
	#if (FICHERO_LOG == AV_LOG)
	{
		double dSegundosTranscurridos = GetSegundosTranscurridos(&g_tReloj);

		ImprimirALog("%u/%u\t%2.2f\t%2.2f\t%8u  %s",
		(UINT32)s32Prof, dbDatosBusqueda.s32ProfSel, (double)s32Eval / 100.0, dSegundosTranscurridos, dbDatosBusqueda.u32NumNodos, szPV);
	}
	#endif
}