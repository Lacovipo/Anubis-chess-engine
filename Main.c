/*
	Anubis

	Copyright Jos� Carlos Mart�nez Gal�n
	Todos los derechos reservados

	-------------------------------------

	M�dulo de implementaci�n de la funci�n
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