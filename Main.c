/*
	Anubis

	José Carlos Martínez Galán
*/

#include "Preprocesador.h"
#include <stdio.h>
#include "Funciones.h"

int __cdecl main(int argc,char *argv[])
{
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

	InicioWB(argc, argv);

	Salir();
}