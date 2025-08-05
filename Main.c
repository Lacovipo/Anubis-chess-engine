/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de la función
	main
*/

#include "Preprocesador.h"
#include <stdio.h>
#include "Funciones.h"

void __cdecl main(int argc,char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

	InicioWB(argc, argv);

	Salir();
}