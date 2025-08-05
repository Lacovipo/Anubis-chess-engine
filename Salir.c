/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de la función
	de salida del programa, cerrando ficheros
	y liberando memoria
*/

#include "Preprocesador.h"
#include "Variables.h"
#include <stdio.h>

void Salir(void)
{
	if (aTablaHash != NULL)
		free(aTablaHash);
	if (aHashEval != NULL)
		free(aHashEval);

	#if (FICHERO_LOG == AV_LOG)
		if (pFLog != NULL)
			fclose(pFLog);
	#endif

	//
	// EGTB (chikki)
	//
	#include "tbprobe.h"
	tb_free();

	exit(0);
}