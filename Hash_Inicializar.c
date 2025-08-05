/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de las funciones
	de inicialización de datos relativos a
	hashing
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"

/************************************************************************/
/*	   Generación de números aleatorios e inicialización de tablas		*/
/*																		*/
/*				Números aleatorios tomados de Crafty					*/
/*																		*/
/*				 con permiso explícito de Bob Hyatt						*/
/*																		*/
/************************************************************************/
static UINT32 Random32(void)
{
	/*
	random numbers from Mathematica 2.0.
	SeedRandom = 1;
	Table[Random[Integer, {0, 2^32 - 1}]
	*/
	static const unsigned long x[55] =
	{
		1410651636UL, 3012776752UL, 3497475623UL, 2892145026UL, 1571949714UL, 3253082284UL, 3489895018UL, 387949491UL, 2597396737UL, 1981903553UL,
		3160251843UL, 129444464UL, 1851443344UL, 4156445905UL, 224604922UL, 1455067070UL, 3953493484UL, 1460937157UL, 2528362617UL, 317430674UL, 
		3229354360UL, 117491133UL, 832845075UL, 1961600170UL, 1321557429UL, 747750121UL, 545747446UL, 810476036UL, 503334515UL, 4088144633UL,
		2824216555UL, 3738252341UL, 3493754131UL, 3672533954UL, 29494241UL, 1180928407UL, 4213624418UL, 33062851UL, 3221315737UL, 1145213552UL,
		2957984897UL, 4078668503UL, 2262661702UL, 65478801UL, 2527208841UL, 1960622036UL, 315685891UL, 1196037864UL, 804614524UL, 1421733266UL,
		2017105031UL, 3882325900UL, 810735053UL, 384606609UL, 2393861397UL
	};
	static int init = 1;
	static unsigned long y[55];
	static int j, k;
	unsigned long ul;
  
	if (init)
	{
		int i;
    
		init=0;
		for (i=0;i<55;i++)
			y[i]=x[i];
		j=24-1;
		k=55-1;
	}
	
	ul=(y[k]+=y[j]);
	if (--j<0)
		j=55-1;
	if (--k<0)
		k=55-1;
	return((UINT32)ul);
}

static UINT64 Random64(void)
{
	UINT64 result;
	UINT32 r1, r2;

	r1 = Random32();
	r2 = Random32();
	result=r1 | (UINT64) r2<<32;
	return (result);
}

/*
	*************************************
	*									*
	*	Inicialización de los valores	*
	*	aleatorios para la generación	*
	*	al vuelo de las claves hash		*
	*									*
	*************************************
*/
void InicializarRandomHash(void)
{
	UINT32 i;

	for (i = 0;i < 64;i++) 
	{
		au64PeonB_Random[i] = Random64();
		au64PeonN_Random[i] = Random64();
		au64CaballoB_Random[i] = Random64();
		au64CaballoN_Random[i] = Random64();
		au64AlfilB_Random[i] = Random64();
		au64AlfilN_Random[i] = Random64();
		au64TorreB_Random[i] = Random64();
		au64TorreN_Random[i] = Random64();
		au64DamaB_Random[i] = Random64();
		au64DamaN_Random[i] = Random64();
		au64ReyB_Random[i] = Random64();
		au64ReyN_Random[i] = Random64();

		au64AlPaso_Random[i] = Random64();
	}

	au64AlPaso_Random[TAB_ALPASOIMPOSIBLE] = Random64();

	for (i = 0;i < 16;i++) 
		au64Enroques_Random[i] = Random64();
}


