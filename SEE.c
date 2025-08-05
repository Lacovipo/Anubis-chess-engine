/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación de la función
	SEE, el evaluador estático de cambios
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Funciones.h"
#include "Inline.h"
#include "Bitboards_inline.h"

/*
 *
 * AsignarValorSEE
 *
 * Recibe: Puntero a la primera y a la última jugadas para ordenar.	OBLIGATORIO: pJug1 <= pJug2
 *
 * Descripción: Recorre la lista de capturas y va asignando su correspondiente valor SEE
 *
 * OJO: Puesto que esta función se va a usar para ordenar las capturas durante la búsqueda, sería interesante
 *  incluir aquí otro factor: el de MVV-LVA (Most Valuable Victim - Least Valuable Aggressor). La idea es que el
 *  SEE determine el signo (para saber si ganamos material o lo perdemos) y, sobre las capturas con SEE positivo,
 *  establecer una ordenación MVV-LVA, que, según dicen, es más efectiva para ordenar los movimientos
 */
void AsignarValorSEE(TPosicion * pPos, TJugada * pJug1, TJugada * pJug2)
{
	SINT32 s32Val;

	for (; pJug2 >= pJug1; pJug2--)
	{
		s32Val = SEE(pPos, *pJug2);
		Jug_SetVal(pJug2, (SINT16)s32Val);
	}
}

/*
 *
 * CambiarPeonAlPaso
 *
 *
 * Recibe: Puntero a la posición actual, casilla de al paso
 *
 * Descripción: Función a apoyo a SEE. Dada una casilla al paso, pone ahí el peón que va a ser zampado y lo quita de la casilla
 *	adyacente. Si se llama de nuevo a esta función, lo vuelve a poner todo en su sitio
 *
 */
static void InvertirPeonAlPaso(TPosicion * pPos, UINT32 u32Hasta)
{
	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		BB_InvertirBit(&pPos->u64PeonesN, u32Hasta);
		BB_InvertirBit(&pPos->u64PeonesN, (u32Hasta+8));
		BB_InvertirBit(&pPos->u64TodasN, u32Hasta);
		BB_InvertirBit(&pPos->u64TodasN, (u32Hasta+8));
	}
	else
	{
		BB_InvertirBit(&pPos->u64PeonesB, u32Hasta);
		BB_InvertirBit(&pPos->u64PeonesB, (u32Hasta-8));
		BB_InvertirBit(&pPos->u64TodasB, u32Hasta);
		BB_InvertirBit(&pPos->u64TodasB, (u32Hasta-8));
	}
}

/*
	*********************************************************************
	*																	*
	*	SEE																*
	*																	*
	*																	*
	*	Recibe: Puntero a la posición actual, jugada a evaluar			*
	*																	*
	*	Devuelve: Evaluación SEE de la jugada							*
	*																	*
	*	Descripción: Calcula el valor SEE (Static Exchange Evaluation)	*
	*				 de la jugada, es decir, evalúa cómo de buena		*
	*				 parece en función de las sucesivas capturas que	*
	*				 sean posibles sobre la casilla de destino			*
	*				 20/12/2002 - Como valor para el rey no pongo inf	*
	*				 porque luego debe caber en un entero de 16 bits	*
	*				 con signo,	así que pongo 30000 que es				*
	*				 suficientemente grande								*
	*																	*
	*********************************************************************
*/
SINT32 SEE(TPosicion * pPos,TJugada jug)
{
	UINT64			u64Ataques;
	UINT64			u64Temp;
	UINT32			u32Temp;
	SINT32			as32Evals[32];
	UINT32			u32NumEvals = 1;
	UINT32			u32Desde;
	UINT32			u32Hasta;
	UINT32			u32PiezaEnOrigen;
	UINT32			u32PiezaEnDestino;
	UINT32			u32Turno = !Pos_GetTurno(pPos);
	static SINT32	as32ValPiezas[15]	=
		{
			0,
			100,300,300,500,1000,30000,	// 100,325,345,530,1100,INF
			0,0,
			100,300,300,500,1000,30000
		};

	//
	// Determinar pieza en origen y destino
	//
	u32Desde = Jug_GetDesde(jug);
	u32Hasta = Jug_GetHasta(jug);
	if (u32Hasta == pPos->u8AlPaso)
		InvertirPeonAlPaso(pPos,u32Hasta);
	u32PiezaEnOrigen = PiezaEnCasilla(pPos,u32Desde);
	u32PiezaEnDestino = PiezaEnCasilla(pPos,u32Hasta);
	assert(u32PiezaEnOrigen > 0);

	//
	// Determinar ataques sobre la posición de destino (menos la pieza que muevo)
	//
	u64Ataques = AtaquesHacia(pPos,u32Hasta);
	BB_ClearBit(&u64Ataques,u32Desde); // No vale "invertir" pq esto puede ser una promoción de peón

	//
	// Comprobamos si la primera pieza oculta un ataque descubierto
	//
	switch (as8Direccion[u32Desde][u32Hasta])
	{
		case 1:
		case -1:
		case 8:
		case -8:
			u64Temp = au64HaciaElBorde[u32Desde][u32Hasta] &
				(pPos->u64TorresB | pPos->u64DamasB
				| pPos->u64TorresN | pPos->u64DamasN);
			while (u64Temp)
			{
				//u32Temp = PrimerUno(u64Temp);
				u32Temp = BB_GetBitYQuitar(&u64Temp);
				if (!(au64Entre[u32Temp][u32Desde] & (pPos->u64TodasB | pPos->u64TodasN)))
				{
					BB_SetBit(&u64Ataques,u32Temp);
					break;
				}
				//BB_InvertirBit(&u64Temp,u32Temp);
			}
			break;
		case 7:
		case -7:
		case 9:
		case -9:
			u64Temp = au64HaciaElBorde[u32Desde][u32Hasta] &
				(pPos->u64AlfilesB | pPos->u64DamasB
				| pPos->u64AlfilesN | pPos->u64DamasN);
			while (u64Temp)
			{
				//u32Temp = PrimerUno(u64Temp);
				u32Temp = BB_GetBitYQuitar(&u64Temp);
				if (!(au64Entre[u32Temp][u32Desde] & (pPos->u64TodasB | pPos->u64TodasN)))
				{
					BB_SetBit(&u64Ataques,u32Temp);
					break;
				}
				//BB_InvertirBit(&u64Temp,u32Temp);
			}
			break;
	}

	//
	// Inicializar array de evaluaciones
	//
	as32Evals[0] = as32ValPiezas[u32PiezaEnDestino];
	if (Jug_GetPromo(jug))
	{
		u32PiezaEnOrigen = Jug_GetPromo(jug);
		as32Evals[0] += as32ValPiezas[u32PiezaEnOrigen];
	}

	//
	// Iteración sobre los ataques sobre la casilla
	//
	while (u64Ataques)
	{
		//
		// Siguiente atacante
		//
		if (u32Turno == BLANCAS)
		{
			if (u64Ataques & pPos->u64PeonesB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64PeonesB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = PB;
			}
			else if (u64Ataques & pPos->u64CaballosB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64CaballosB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = CB;
			}
			else if (u64Ataques & pPos->u64AlfilesB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64AlfilesB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = AB;
			}
			else if (u64Ataques & pPos->u64TorresB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64TorresB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = TB;
			}
			else if (u64Ataques & pPos->u64DamasB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64DamasB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = DB;
			}
			else if (u64Ataques & BB_Mask(pPos->u8PosReyB))
			{
				u32Desde = pPos->u8PosReyB;
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = RB;
			}
			else
				break;
		} // if (u32Turno == BLANCAS)
		else
		{
			if (u64Ataques & pPos->u64PeonesN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64PeonesN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = PN;
			}
			else if (u64Ataques & pPos->u64CaballosN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64CaballosN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = CN;
			}
			else if (u64Ataques & pPos->u64AlfilesN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64AlfilesN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = AN;
			}
			else if (u64Ataques & pPos->u64TorresN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64TorresN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = TN;
			}
			else if (u64Ataques & pPos->u64DamasN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64DamasN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = DN;
			}
			else if (u64Ataques & BB_Mask(pPos->u8PosReyN))
			{
				u32Desde = pPos->u8PosReyN;
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = RN;
			}
			else
				break;
		} // else [if (u32Turno == BLANCAS)]

		//
		// Si el bando al que no toca mover pierde material, salimos
		//
		if (as32Evals[u32NumEvals] < 0 && u32Turno == Pos_GetTurno(pPos))
		{
			//
			// Si era casilla al paso, antes de salir pongo las cosas en su sitio
			//
			if (u32Hasta == pPos->u8AlPaso)
				InvertirPeonAlPaso(pPos,u32Hasta);
			return(as32Evals[u32NumEvals]);
		}

		//
		// Comprobamos si hay ataque descubierto
		//
		switch (as8Direccion[u32Desde][u32Hasta])
		{
			case 1:
			case -1:
			case 8:
			case -8:
				u64Temp = au64HaciaElBorde[u32Desde][u32Hasta] &
					(pPos->u64TorresB | pPos->u64DamasB
					| pPos->u64TorresN | pPos->u64DamasN);
				while (u64Temp)
				{
					u32Temp = PrimerUno(u64Temp);
					if (!(au64Entre[u32Temp][u32Desde] & (pPos->u64TodasB | pPos->u64TodasN)))
					{
						BB_SetBit(&u64Ataques,u32Temp);
						break;
					}
					BB_InvertirBit(&u64Temp,u32Temp);
				}
				break;
			case 7:
			case -7:
			case 9:
			case -9:
				u64Temp = au64HaciaElBorde[u32Desde][u32Hasta] &
					(pPos->u64AlfilesB | pPos->u64DamasB
					| pPos->u64AlfilesN | pPos->u64DamasN);
				while (u64Temp)
				{
					u32Temp = PrimerUno(u64Temp);
					if (!(au64Entre[u32Temp][u32Desde] & (pPos->u64TodasB | pPos->u64TodasN)))
					{
						BB_SetBit(&u64Ataques,u32Temp);
						break;
					}
					BB_InvertirBit(&u64Temp,u32Temp);
				}
				break;
		}

		u32NumEvals++;
		u32Turno = !u32Turno;
		BB_InvertirBit(&u64Ataques,u32Desde);
	} // while (u64Ataques)

	//
	// Si era captura al paso, vuelvo a poner las cosas en su sitio
	//
	if (u32Hasta == pPos->u8AlPaso)
		InvertirPeonAlPaso(pPos,u32Hasta);

	//
	// Hemos agotado las capturas, hacemos recuento
	//
	while (--u32NumEvals)
	{
		if (as32Evals[u32NumEvals] > -as32Evals[u32NumEvals - 1])
			as32Evals[u32NumEvals - 1] = -as32Evals[u32NumEvals];
	}
	return(as32Evals[0]);
}

// 20/04/2012 - Añado parámetro u32PiezaQuePongo, que es el valor de la pieza que pongo en esa casilla (y que, por tanto, me pueden
//	zampar), que usaré para las llamadas donde se pregunte ¿qué pasaría si pusiera ahí un alfil?
//	Dejar a cero si se quiere usar la pieza que realmente hay en la casilla
SINT32 SEE_Amenaza(TPosicion * pPos, UINT32 u32Hasta, UINT32 u32PiezaQuePongo)
{
	UINT64			u64Ataques;
	UINT64			u64Temp;
	UINT32			u32Temp;
	SINT32			as32Evals[32];
	UINT32			u32NumEvals = 1;
	UINT32			u32PiezaEnOrigen;
	UINT32			u32Turno = !Pos_GetTurno(pPos);
	UINT32			u32Desde;
	static SINT32	as32ValPiezas[15]	=
		{
			0,
			100,300,300,500,1000,30000,	// 100,325,345,530,1100,INF
			0,0,
			100,300,300,500,1000,30000,
		};

	if (u32PiezaQuePongo != VACIO)
		u32PiezaEnOrigen = u32PiezaQuePongo;
	else
		u32PiezaEnOrigen = PiezaEnCasilla(pPos,u32Hasta);

	//
	// Determinar ataques sobre la posición de destino
	//
	u64Ataques = AtaquesHacia(pPos,u32Hasta);

	//
	// Inicializar array de evaluaciones
	//
	as32Evals[0] = 0;

	//
	// Iteración sobre los ataques sobre la casilla
	//
	while (u64Ataques)
	{
		//
		// Siguiente atacante
		//
		if (u32Turno == BLANCAS)
		{
			if (u64Ataques & pPos->u64PeonesB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64PeonesB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = PB;
			}
			else if (u64Ataques & pPos->u64CaballosB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64CaballosB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = CB;
			}
			else if (u64Ataques & pPos->u64AlfilesB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64AlfilesB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = AB;
			}
			else if (u64Ataques & pPos->u64TorresB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64TorresB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = TB;
			}
			else if (u64Ataques & pPos->u64DamasB)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64DamasB);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = DB;
			}
			else if (u64Ataques & BB_Mask(pPos->u8PosReyB))
			{
				u32Desde = pPos->u8PosReyB;
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = RB;
			}
			else
				break;
		} // if (u32Turno == BLANCAS)
		else
		{
			if (u64Ataques & pPos->u64PeonesN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64PeonesN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = PN;
			}
			else if (u64Ataques & pPos->u64CaballosN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64CaballosN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = CN;
			}
			else if (u64Ataques & pPos->u64AlfilesN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64AlfilesN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = AN;
			}
			else if (u64Ataques & pPos->u64TorresN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64TorresN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = TN;
			}
			else if (u64Ataques & pPos->u64DamasN)
			{
				u32Desde = PrimerUno(u64Ataques & pPos->u64DamasN);
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = DN;
			}
			else if (u64Ataques & BB_Mask(pPos->u8PosReyN))
			{
				u32Desde = pPos->u8PosReyN;
				as32Evals[u32NumEvals] = -as32Evals[u32NumEvals - 1] + as32ValPiezas[u32PiezaEnOrigen];
				u32PiezaEnOrigen = RN;
			}
			else
				break;
		} // else [if (u32Turno == BLANCAS)]

		//
		// Si el bando al que no toca mover pierde material, salimos
		//
		if (as32Evals[u32NumEvals] < 0 && u32Turno != Pos_GetTurno(pPos))
			return(0);


		//
		// Comprobamos si hay ataque descubierto
		//
		switch (as8Direccion[u32Desde][u32Hasta])
		{
			case 1:
			case -1:
			case 8:
			case -8:
				u64Temp = au64HaciaElBorde[u32Desde][u32Hasta] & (pPos->u64TorresB | pPos->u64DamasB | pPos->u64TorresN | pPos->u64DamasN);
				while (u64Temp)
				{
					u32Temp = PrimerUno(u64Temp);
					if (!(au64Entre[u32Temp][u32Desde] & (pPos->u64TodasB | pPos->u64TodasN)))
					{
						BB_SetBit(&u64Ataques,u32Temp);
						break;
					}
					BB_InvertirBit(&u64Temp,u32Temp);
				}
				break;
			case 7:
			case -7:
			case 9:
			case -9:
				u64Temp = au64HaciaElBorde[u32Desde][u32Hasta] &
					(pPos->u64AlfilesB | pPos->u64DamasB
					| pPos->u64AlfilesN | pPos->u64DamasN);
				while (u64Temp)
				{
					u32Temp = PrimerUno(u64Temp);
					if (!(au64Entre[u32Temp][u32Desde] & (pPos->u64TodasB | pPos->u64TodasN)))
					{
						BB_SetBit(&u64Ataques,u32Temp);
						break;
					}
					BB_InvertirBit(&u64Temp,u32Temp);
				}
				break;
		}

		u32NumEvals++;
		u32Turno = !u32Turno;
		BB_InvertirBit(&u64Ataques,u32Desde);
	} // while (u64Ataques)

	//
	// Hemos agotado las capturas, hacemos recuento
	//
	while (--u32NumEvals)
	{
		if (as32Evals[u32NumEvals] > -as32Evals[u32NumEvals - 1])
			as32Evals[u32NumEvals - 1] = -as32Evals[u32NumEvals];
	}
	// Nunca puede ser cero, pues no nos comemos nada en la primera jugada nula, pero lo pongo
	//	aquí por si acaso.
	// Luego devuelve en negativo, pues es lo que nos amenaza comernos el contrario sobre esta
	//	casilla
	//
	if (as32Evals[0] > 0)
		return(0);
	else
		return(-as32Evals[0]);
}
