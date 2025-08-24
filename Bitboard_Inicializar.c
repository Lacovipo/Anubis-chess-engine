/*
	Anubis

	José Carlos Martínez Galán
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"
#include "Inline.h"
#include "Bitboards_inline.h"
#include "Funciones.h"

#define MismaColumna(i,j)		(((i)%8) == ((j)%8))
#define MismaFila(i,j)			(((i)/8) == ((j)/8))
#define MismaDiagonalA1H8(i,j)	((((i)/8) + ((i)%8)) == (((j)/8) + ((j)%8)))
#define MismaDiagonalA8H1(i,j)	((((i)/8) - ((i)%8)) == (((j)/8) - ((j)%8)))
#define	EnLinea(i,j)			(MismaFila(i,j) || MismaColumna(i,j) || MismaDiagonalA1H8(i,j) || MismaDiagonalA8H1(i,j))

/*
	Inicializa los bitboards de ataques de rey
*/
static void InicializarAtaquesRey(void)
{
	SINT32 s32Origen;

	for (s32Origen = 0; s32Origen < 64; s32Origen++)
	{
		au64ZonaReyExtendidaAbajo[s32Origen] = BB_ATAQUES_REY[s32Origen] | (BB_ATAQUES_REY[s32Origen] >> 8) | BB_Mask(s32Origen);
		au64ZonaReyExtendidaArriba[s32Origen] = BB_ATAQUES_REY[s32Origen] | (BB_ATAQUES_REY[s32Origen] << 8) | BB_Mask(s32Origen);
	}
}

/*
	Inicializar bitboards de ataques de torres
	(filas y columnas)
*/
static void InicializarAtaquesTorre(void)
{
	UINT32 u32Origen;

	for (u32Origen = 0;u32Origen < 64;u32Origen++)
	{
		au64AtaquesTorre[u32Origen] = au64Fila[Tab_GetFila(u32Origen)] | au64Columna[Tab_GetColumna(u32Origen)];
		BB_ClearBit(&au64AtaquesTorre[u32Origen],u32Origen);
	}
}

/*
	Inicializar ataques de peón (B y N)
	No hay peones en la primera ni última filas, luego restrinjo el bucle
*/
static void InicializarAtaquesPeon(void)
{
	UINT32 u32Origen;

	for (u32Origen = 8;u32Origen < 56;u32Origen++)
	{
		au64AtaquesPeonB[u32Origen] = AtaquesPeonB(BB_Mask(u32Origen));
		au64AtaquesPeonN[u32Origen] = AtaquesPeonN(BB_Mask(u32Origen));
	}
}

/*
	Inicializar algunos bitboards auxiliares
*/
static void InicializarBitboardsAuxiliares(void)
{
	UINT32 i,j,k;

	// Mask
	for (i = 0;i < 64;i++)
		au64Mask[i] = (UINT64)0x8000000000000000 >> i;

	// Entre
	for (i = 0;i < 64;i++)
	{
		for (j = i;j < 64;j++)
		{
			au64Entre[i][j] = BB_TABLEROVACIO;
			au64Entre[j][i] = BB_TABLEROVACIO;
			if (i != j)
			{
				// i < j por la estructura de los bucles
				if (MismaColumna(i,j))
				{
					for (k = i+8;k < j;k += 8)
					{
						BB_SetBit(&au64Entre[i][j],k);
						BB_SetBit(&au64Entre[j][i],k);
					}
				}
				else if (MismaFila(i,j))
				{
					for (k = i+1;k < j;k++)
					{
						BB_SetBit(&au64Entre[i][j],k);
						BB_SetBit(&au64Entre[j][i],k);
					}
				}
				else if (MismaDiagonalA1H8(i,j))
				{
					for (k = i+7;k < j;k += 7)
					{
						BB_SetBit(&au64Entre[i][j],k);
						BB_SetBit(&au64Entre[j][i],k);
					}
				}
				else if (MismaDiagonalA8H1(i,j)) // Diagonal a8-h1
				{
					for (k = i+9;k < j;k += 9)
					{
						BB_SetBit(&au64Entre[i][j],k);
						BB_SetBit(&au64Entre[j][i],k);
					}
				}
			}
		}
	} // Entre

	// AdyacenteH (casillas adyacentes horizontalmente)
	for (i = 0;i < 64;i++)
	{
		au64AdyacenteH[i] = BB_TABLEROVACIO;
		if (Tab_GetColumna(i) > 0)
			au64AdyacenteH[i] |= BB_Mask(i - 1);
		if (Tab_GetColumna(i) < 7)
			au64AdyacenteH[i] |= BB_Mask(i + 1);
	}

	// Direcciones diagonales y rectas
	for (i = 0;i < 64;i++)
	{
		au64DireccionesDiag[i] = BB_TABLEROVACIO;
		au64DireccionesRect[i] = BB_TABLEROVACIO;

		if (Tab_GetColumna(i) > 0)
			au64DireccionesRect[i] |= BB_Mask(i - 1);
		if (Tab_GetColumna(i) < 7)
			au64DireccionesRect[i] |= BB_Mask(i + 1);
		if (Tab_GetFila(i) > 0)
			au64DireccionesRect[i] |= BB_Mask(i - 8);
		if (Tab_GetFila(i) < 7)
			au64DireccionesRect[i] |= BB_Mask(i + 8);

		if (Tab_GetColumna(i) > 0 && Tab_GetFila(i) > 0)
			au64DireccionesDiag[i] |= BB_Mask(i - 9);
		if (Tab_GetColumna(i) > 0 && Tab_GetFila(i) < 7)
			au64DireccionesDiag[i] |= BB_Mask(i + 7);
		if (Tab_GetColumna(i) < 7 && Tab_GetFila(i) > 0)
			au64DireccionesDiag[i] |= BB_Mask(i - 7);
		if (Tab_GetColumna(i) < 7 && Tab_GetFila(i) < 7)
			au64DireccionesDiag[i] |= BB_Mask(i + 9);
	}

	// Círculo 2 (considero tb el interno porque es a efectos de ataques)
	for (i = 0;i < 64;i++)
	{
		au64Circunferencia2[i] = BB_TABLEROVACIO;

		for (j = 0;j < 64;j++)
		{
			if (Tab_GetDistancia(i,j) == 2 || Tab_GetDistancia(i,j) == 1)
				au64Circunferencia2[i] |= BB_Mask(j);
		}
	}

	// Frontal
	for (i = 0;i < 64;i++)
	{
		au64FrontalReyBlanco[i] = BB_TABLEROVACIO;
		au64FrontalReyNegro[i] = BB_TABLEROVACIO;

		if (Tab_GetFila(i) > 0)
		{
			au64FrontalReyBlanco[i] |= BB_Mask(i - 8);
			if (Tab_GetColumna(i) > 0)
				au64FrontalReyBlanco[i] |= BB_Mask(i - 9);
			if (Tab_GetColumna(i) < 7)
				au64FrontalReyBlanco[i] |= BB_Mask(i - 7);
		}
		if (Tab_GetFila(i) < 7)
		{
			au64FrontalReyNegro[i] |= BB_Mask(i + 8);
			if (Tab_GetColumna(i) > 0)
				au64FrontalReyNegro[i] |= BB_Mask(i + 7);
			if (Tab_GetColumna(i) < 7)
				au64FrontalReyNegro[i] |= BB_Mask(i + 9);
		}
	}

	// Distancia
	for (i = 0;i < 64;i++)
	{
		for (j = i;j < 64;j++)
		{
			au8Distancia[i][j] = (UINT8)Tab_GetDistancia(i,j);
			au8Distancia[j][i] = (UINT8)Tab_GetDistancia(j,i);
		}
	}
}

static void InicializarHaciaElBorde(void)
{
	UINT64 u64;
	UINT32 i,j;

	// HaciaElBorde (dadas dos casillas, ir de la primera al borde que
	// no pasa por la segunda)
	for (i = 0;i < 64;i++)
	{
		for (j = i;j < 64;j++)
		{
			au64HaciaElBorde[i][j] = BB_TABLEROVACIO;
			au64HaciaElBorde[j][i] = BB_TABLEROVACIO;
			if (EnLinea(i,j))
			{
				//
				// ...i...
				// ...j...
				//
				if (MismaColumna(i,j))
				{
					if (i > 7)
					{
						u64 = BB_Mask(i - 8);
						u64 |= (u64 << 8);
						u64 |= (u64 << 16);
						u64 |= (u64 << 32);
						au64HaciaElBorde[i][j] |= (u64 & au64Columna[Tab_GetColumna(i)]);
					}
					if (j < 56)
					{
						u64 = BB_Mask(j + 8);
						u64 |= (u64 >> 8);
						u64 |= (u64 >> 16);
						u64 |= (u64 >> 32);
						au64HaciaElBorde[j][i] |= (u64 & au64Columna[Tab_GetColumna(i)]);
					}
				}
				//
				// ...ij...
				//
				if (MismaFila(i,j))
				{
					if (Tab_GetColumna(i) > 0)
					{
						u64 = BB_Mask(i - 1);
						u64 |= (u64 << 1);
						u64 |= (u64 << 2);
						u64 |= (u64 << 4);
						au64HaciaElBorde[i][j] |= (u64 & au64Fila[Tab_GetFila(i)]);
					}
					if (Tab_GetColumna(j) < 7)
					{
						u64 = BB_Mask(j + 1);
						u64 |= (u64 >> 1);
						u64 |= (u64 >> 2);
						u64 |= (u64 >> 4);
						au64HaciaElBorde[j][i] |= (u64 & au64Fila[Tab_GetFila(i)]);
					}
				}
				//
				// ...i...
				// ..j....
				//
				if (MismaDiagonalA1H8(i,j))
				{
					if ((Tab_GetFila(i) > 0) && (Tab_GetColumna(i) < 7))
					{
						u64 = BB_Mask(i - 7);
						u64 |= (u64 << 7);
						u64 |= (u64 << 14);
						u64 |= (u64 << 28);
						au64HaciaElBorde[i][j] |= (u64 & BB_ATAQUES_ALFIL[i]);
					}
					if ((Tab_GetFila(j)) < 7 && (Tab_GetColumna(j) > 0))
					{
						u64 = BB_Mask(j + 7);
						u64 |= (u64 >> 7);
						u64 |= (u64 >> 14);
						u64 |= (u64 >> 28);
						au64HaciaElBorde[j][i] |= (u64 & BB_ATAQUES_ALFIL[i]);
					}
				}
				//
				// ..i....
				// ...j...
				//
				if (MismaDiagonalA8H1(i,j))
				{
					if ((Tab_GetFila(i) > 0) && (Tab_GetColumna(i) > 0))
					{
						u64 = BB_Mask(i - 9);
						u64 |= (u64 << 9);
						u64 |= (u64 << 18);
						u64 |= (u64 << 36);
						au64HaciaElBorde[i][j] |= (u64 & BB_ATAQUES_ALFIL[i]);
					}
					if ((Tab_GetFila(j)) < 7 && (Tab_GetColumna(j) < 7))
					{
						u64 = BB_Mask(j + 9);
						u64 |= (u64 >> 9);
						u64 |= (u64 >> 18);
						u64 |= (u64 >> 36);
						au64HaciaElBorde[j][i] |= (u64 & BB_ATAQUES_ALFIL[i]);
					}
				}
			}
		}
	} // HaciaElBorde
}

static void InicializarDirecciones(void)
{
	UINT32 i,j;

	for (i = 0;i < 64;i++)
	{
		for (j = i;j < 64;j++)
		{
			as8Direccion[i][j] = 0;
			as8Direccion[j][i] = 0;
			if (i != j)
			{
				if (MismaColumna(i,j))
				{
					as8Direccion[i][j] = 1;
					as8Direccion[j][i] = -1;
				}
				else if (MismaFila(i,j))
				{
					as8Direccion[i][j] = 8;
					as8Direccion[j][i] = -8;
				}
				else if (MismaDiagonalA1H8(i,j))
				{
					as8Direccion[i][j] = 7;
					as8Direccion[j][i] = -7;
				}
				else if (MismaDiagonalA8H1(i,j))
				{
					as8Direccion[i][j] = 9;
					as8Direccion[j][i] = -9;
				}
			}
		}
	}
}

/*
	Inicializa tablas para evaluar peones
*/
static void InicializarPeones(void)
{
	UINT32 i,j;

	// Pasados
	for (i = 0;i < 64;i++)
	{
		j = Tab_GetColumna(i);
		au64AreaPasadoB[i] = au64Entre[i][j];
		au64AreaPasadoN[i] = au64Entre[i][j + 56];
		if (j > 0)
		{
			au64AreaPasadoB[i] |= au64Entre[i - 1][j - 1];
			au64AreaPasadoN[i] |= au64Entre[i - 1][j + 55];
		}
		if (j < 7)
		{
			au64AreaPasadoB[i] |= au64Entre[i + 1][j + 1];
			au64AreaPasadoN[i] |= au64Entre[i + 1][j + 57];
		}
	}

	// Distancia al cuadro cuadro. i -> Peón   j -> Rey
	// ¡OJO! Indica cómo de lejos está de entrar en el cuadro, es decir, vale 0 si está dentro
	for (i = 0;i < 64;i++)
	{
		for (j = 0;j < 64;j++)
		{
			au8DistCuadroPB[i][j] = 0;
			au8DistCuadroPN[i][j] = 0;

			if (Tab_GetFila(j) > Tab_GetFila(i))
				au8DistCuadroPB[i][j] += (UINT8)(abs(Tab_GetFila(j) - Tab_GetFila(i)));	// Cómo de lejos está de entrar en cuadro
			if ((UINT32)abs(Tab_GetColumna(j) - Tab_GetColumna(i)) > Tab_GetFila(i))
				au8DistCuadroPB[i][j] = (UINT8)max
												(
													au8DistCuadroPB[i][j]
													,
													abs(Tab_GetColumna(j) - Tab_GetColumna(i)) - Tab_GetFila(i)
												);
			if (Tab_GetFila(j) < Tab_GetFila(i))
				au8DistCuadroPN[i][j] += (UINT8)(abs(Tab_GetFila(i) - Tab_GetFila(j)));	// Cómo de lejos está de entrar en cuadro
			if ((UINT32)abs(Tab_GetColumna(j) - Tab_GetColumna(i)) > (7 - Tab_GetFila(i)))
				au8DistCuadroPN[i][j] = (UINT8)max
												(
													au8DistCuadroPN[i][j]
													,
													abs(Tab_GetColumna(j) - Tab_GetColumna(i)) - (7 - Tab_GetFila(i))
												);
		}
	}

	// Rey no llega a parar peón
	for (i = 0;i < 64;i++)
	{
		au64ZonaReyBNoLlega[i] = BB_TABLEROVACIO;
		au64ZonaReyNNoLlega[i] = BB_TABLEROVACIO;
		au64ZonaReyBNoLlegaConTurno[i] = BB_TABLEROVACIO;
		au64ZonaReyNNoLlegaConTurno[i] = BB_TABLEROVACIO;
		for (j = 0;j < 64;j++)
		{
			if (au8DistCuadroPB[j][i])	// [peón blanco][rey negro] ** > 0 si está fuera del cuadro
				au64ZonaReyNNoLlega[i] |= BB_Mask(j);
			if (au8DistCuadroPN[j][i])	// [peón negro][rey blanco] ** > 0 si está fuera del cuadro
				au64ZonaReyBNoLlega[i] |= BB_Mask(j);
			if (au8DistCuadroPB[j][i] > 1)	// [peón blanco][rey negro] ** > 1 si está fuera del cuadro a pesar del turno
				au64ZonaReyNNoLlegaConTurno[i] |= BB_Mask(j);
			if (au8DistCuadroPN[j][i] > 1)	// [peón negro][rey blanco] ** > 1 si está fuera del cuadro a pesar del turno
				au64ZonaReyBNoLlegaConTurno[i] |= BB_Mask(j);
		}
	}

	// Zona donde poner el rey para ganar un finak KPK
	for (i = 0;i < 64;i++)
	{
		if (Tab_GetFila(i) == 0)
			au64ZonaKPKGanaB[i] = BB_TABLEROVACIO;
		else if (Tab_GetFila(i) == 1)
		{
			au64ZonaKPKGanaB[i] = BB_Mask(i - 8)
				| BB_GetShiftIzda(BB_Mask(i - 8))
				| BB_GetShiftDcha(BB_Mask(i - 8));
		}
		else
		{
			au64ZonaKPKGanaB[i] = BB_Mask(i - 16)
				| BB_GetShiftIzda(BB_Mask(i - 16))
				| BB_GetShiftDcha(BB_Mask(i - 16));
			au64ZonaKPKGanaB[i] |= AtaquesArriba(au64ZonaKPKGanaB[i],BB_TABLEROLLENO);
		}

		if (Tab_GetFila(i) == 7)
			au64ZonaKPKGanaN[i] = BB_TABLEROVACIO;
		else if (Tab_GetFila(i) == 6)
		{
			au64ZonaKPKGanaN[i] = BB_Mask(i + 8)
				| BB_GetShiftIzda(BB_Mask(i + 8))
				| BB_GetShiftDcha(BB_Mask(i + 8));
		}
		else
		{
			au64ZonaKPKGanaN[i] = BB_Mask(i + 16)
				| BB_GetShiftIzda(BB_Mask(i + 16))
				| BB_GetShiftDcha(BB_Mask(i + 16));
			au64ZonaKPKGanaN[i] |= AtaquesAbajo(au64ZonaKPKGanaN[i],BB_TABLEROLLENO);
		}
	}

	// Oposición
	for (i = 0;i < 64;i++)
	{
		for (j = i;j < 64;j++)
		{
			switch(abs(i-j))
			{
				case 16:		// Vertical
				case 2:			// Horizontal
//				case 18:		// Diagonal A8H1
//				case 14:		// Diagonal A1H8
					au8Oposicion[i][j] = 1;
					au8Oposicion[j][i] = 1;
					break;
				case 32:		// Vertical distante
				case 4:			// Horizontal distante
//				case 36:		// Diagonal A8H1 distante
//				case 28:		// Diagonal A1H8 distante
					au8Oposicion[i][j] = 2;
					au8Oposicion[j][i] = 2;
					break;
				default:
					au8Oposicion[i][j] = 0;
					au8Oposicion[j][i] = 0;
			}
		}
	}
}

/*
	Inicializa todos los datos precomputados del programa
*/
void InicializarDatosPrecomputados(void)
{
	InicializarBitboardsAuxiliares();
	InicializarAtaquesRey();
	InicializarAtaquesTorre();
	InicializarAtaquesPeon();
	InicializarHaciaElBorde();
	InicializarDirecciones();
	InicializarPeones();
}
