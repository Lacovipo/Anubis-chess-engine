/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación del generador de
	movimientos que den jaque y no sean capturas
	ni promociones
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include <assert.h>
#include "Bitboards_inline.h"
#include "Inline.h"

/*
	*********************************************************************
	*																	*
	*	GenerarJugadasCasilla											*
	*																	*
	*																	*
	*	Recibe: puntero a la posición actual, puntero a la lista de		*
	*			de juegadas, índice de la casilla donde está la pieza	*
	*			que queremos mover, zona de exclusión					*
	*																	*
	*	Devuelve: el puntero a la jugada (por si lo modificamos dentro	*
	*			  de la función											*
	*																	*
	*	Descripción: en la casilla que pasamos hay una pieza cuyo tipo	*
	*				 desconocemos, y que queremos mover. Generamos		*
	*				 todas sus jugadas pseudo legales salvo las que se	*
	*				 dirijan a la zona de exclusión. Esto es porque		*
	*				 estamos generando jugadas que descubren un jaque,	*
	*				 y no queremos seguir tapándolo						*
	*																	*
	*	Problemas conocidos: si una jugada es jaque descubierto y		*
	*						 directo a un tiempo, será generada dos		*
	*						 veces. Por el momento, lo dejo así			*
	*																	*
	*********************************************************************
*/
static TJugada * GenerarJugadasCasilla(TPosicion * pPos,
									   TJugada * pJugada,
									   UINT32 u32Casilla,
									   UINT64 u64Excluir)
{
	UINT64 u64ZonaPermitida = ~u64Excluir;
	UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	UINT64 u64Vacias = ~u64Todas;
	UINT64 u64Destinos;
	UINT32 u32Hasta;

	assert(PiezaEnCasilla(pPos,u32Casilla) == PB || PiezaEnCasilla(pPos,u32Casilla) == CB
		|| PiezaEnCasilla(pPos,u32Casilla) == AB || PiezaEnCasilla(pPos,u32Casilla) == TB
		|| PiezaEnCasilla(pPos,u32Casilla) == DB || PiezaEnCasilla(pPos,u32Casilla) == RB
		|| PiezaEnCasilla(pPos,u32Casilla) == PN || PiezaEnCasilla(pPos,u32Casilla) == CN
		|| PiezaEnCasilla(pPos,u32Casilla) == AN || PiezaEnCasilla(pPos,u32Casilla) == TN
		|| PiezaEnCasilla(pPos,u32Casilla) == DN || PiezaEnCasilla(pPos,u32Casilla) == RN);
	switch(PiezaEnCasilla(pPos,u32Casilla))
	{
		case RB:
			//
			// Rey blanco (no es necesario considerar los enroques, pues no pueden decubrir un
			// jaque, y el jaque con la torre que enroca es tan raro que lo voy a ignorar)
			//
			u64Destinos = BB_ATAQUES_REY[u32Casilla] & u64Vacias & u64ZonaPermitida & ~pPos->u64AtaquesN;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Hasta,0);
			}
			break;
		case RN:
			//
			// Rey negro (no es necesario considerar los enroques, pues no pueden decubrir un
			// jaque, y el jaque con la torre que enroca es tan raro que lo voy a ignorar)
			//
			u64Destinos = BB_ATAQUES_REY[u32Casilla] & u64Vacias & u64ZonaPermitida & ~pPos->u64AtaquesB;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Hasta,0);
			}
			break;
		case DB:
		case DN:
			//
			// Damas (ambos colores llevan el mismo código)
			//
			u64Destinos = (au64AtaquesTorre[u32Casilla] | BB_ATAQUES_ALFIL[u32Casilla])
				& u64Vacias & u64ZonaPermitida;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				// Si hay alguna pieza entre origen y destino, pasamos a otra
				if (au64Entre[u32Casilla][u32Hasta] & u64Todas) 
					continue;
				Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Hasta,0);
			}
			break;
		case TB:
		case TN:
			//
			// Torres (ambos colores llevan el mismo código)
			//
			u64Destinos = au64AtaquesTorre[u32Casilla] & u64Vacias & u64ZonaPermitida;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				// Si hay alguna pieza entre origen y destino, pasamos a otra
				if (au64Entre[u32Casilla][u32Hasta] & u64Todas) 
					continue;
				Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Hasta,0);
			}
			break;
		case AB:
		case AN:
			//
			// Alfiles (ambos colores llevan el mismo código)
			//
			u64Destinos = BB_ATAQUES_ALFIL[u32Casilla] & u64Vacias & u64ZonaPermitida;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				// Si hay alguna pieza entre origen y destino, pasamos a otra
				if (au64Entre[u32Casilla][u32Hasta] & u64Todas) 
					continue;
				Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Hasta,0);
			}
			break;
		case CB:
		case CN:
			//
			// Caballos (ambos colores llevan el mismo código)
			//
			u64Destinos = BB_ATAQUES_CABALLO[u32Casilla] & u64Vacias & u64ZonaPermitida;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Hasta,0);
			}
			break;
		case PB:
			//
			// Peones blancos (las promociones van en GenerarCapturas)
			//
			if (Tab_GetFila(u32Casilla) > 1)
			{
				u64Destinos = BB_Mask(u32Casilla) << 8;
				if (u64Destinos & ~u64Todas & u64ZonaPermitida)
				{
					Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Casilla - 8,0);
					if (Tab_GetFila(u32Casilla) == 7)
					{
						u64Destinos <<= 8;
						if (u64Destinos & ~u64Todas & u64ZonaPermitida)
							Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Casilla - 16,0);
					}
				}
			}
			break;
		case PN:
			//
			// Peones negros (las promociones van en GenerarCapturas)
			//
			if (Tab_GetFila(u32Casilla) < 6)
			{
				u64Destinos = BB_Mask(u32Casilla) >> 8;
				if (u64Destinos & ~u64Todas & u64ZonaPermitida)
				{
					Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Casilla + 8,0);
					if (Tab_GetFila(u32Casilla) == 1)
					{
						u64Destinos >>= 8;
						if (u64Destinos & ~u64Todas & u64ZonaPermitida)
							Jug_SetMovEvalCero(pJugada++,u32Casilla,u32Casilla + 16,0);
					}
				}
			}
			break;
		default:
			__assume(0);
	}
	return(pJugada);
}

/*
	*********************************************************************
	*																	*
	*	GenerarJaquesNoCapturas											*
	*																	*
	*																	*
	*	Recibe: puntero a la posición actual, puntero a la lista de		*
	*			jugadas													*
	*																	*
	*	Devuelve: puntero a la última jugada generada					*
	*																	*
	*	Descripción: Esta función está escrita para ser llamada	desde	*
	*				 la QSearch, tras haber intentado las capturas		*
	*				 ganadoras. Por ello, las capturas y promociones	*
	*				 son excluidas aquí	y sólo generamos jaques			*
	*				 asestados desde casillas vacías					*
	*				 Igualmente, ignoramos el caso de estar en jaque	*
	*				 nuestro rey, pues en tal situación generaríamos	*
	*				 todos los escapes									*
	*				 Tenemos dos tipos de jaque a considerar:			*
	*					- ataques directos al rey enemigo				*
	*						+ Movimientos de peón						*
	*						+ Movimientos de caballo					*
	*						+ Piezas deslizantes a líneas abiertas		*
	*						  sobre el rey (generado por Kogge-Stone)	*
	*					- descubiertas									*
	*						+ Piezas deslizantes a casillas de ataque	*
	*						  en segundo grado, es decir, genero las	*
	*						  casillas que atacan directamente al rey,	*
	*						  "and" con las piezas enemigas, y continúo	*
	*						  hasta hallar otra pieza enemiga que		*
	*						  corresponda al tipo adecuado. Quitar la	*
	*						  pieza de enmedio _en otra dirección_		*
	*						  distinta de la del rey es una descubierta	*
	*																	*
	*********************************************************************
*/
TJugada * GenerarJaquesNoCapturas(TPosicion * pPos,TJugada * pJugada)
{
	UINT64		u64Pieza;
	UINT64		u64Destinos;
	UINT64		u64Vacias;
	UINT64		u64Todas;
	UINT64		u64EntreB;
	UINT64		u64EntreN;
	UINT64		u64Candidatos;
	UINT32		u32Desde;
	UINT32		u32Hasta;

	u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	u64Vacias = ~u64Todas;

	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		/*
			+---------------+
			|				|
			|	BLANCAS		|
			|				|
			+---------------+
		*/

		//
		// Peones (ataque directo)
		//
		u64Destinos = au64AtaquesPeonN[pPos->u8PosReyN] & u64Vacias;
		// Un paso
		u64Pieza = BB_GetShiftAbajo(u64Destinos) & pPos->u64PeonesB;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			Jug_SetMovEvalCero(pJugada++,u32Desde,u32Desde - 8,0);
		}
		// Dos pasos
		if (Tab_GetFila(pPos->u8PosReyN) == 3)
		{
			u64Pieza = BB_GetShiftAbajo(u64Destinos) & u64Vacias;
			u64Pieza = BB_GetShiftAbajo(u64Pieza) & pPos->u64PeonesB;
			while (u64Pieza)
			{
				u32Desde = BB_GetBitYQuitar(&u64Pieza);
				Jug_SetMovEvalCero(pJugada++,u32Desde,u32Desde - 16,0);
			}
		}

		//
		// Caballos (ataque directo)
		//
		if (pPos->u64CaballosB)
		{
			// Posibles destinos: casillas en el perímetro caballar del RN atacadas por CB's
			u64Destinos = pPos->u64AtaquesCB & BB_ATAQUES_CABALLO[pPos->u8PosReyN] & u64Vacias;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				u64Pieza = pPos->u64CaballosB & BB_ATAQUES_CABALLO[u32Hasta];
				while (u64Pieza)
				{
					u32Desde = BB_GetBitYQuitar(&u64Pieza);
					Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
			}
		}

		//
		// Alfiles (ataque directo)
		//
		if (pPos->u64AlfilesB)
		{
			// Posibles destinos: casillas libres que atacan diagonalmente al rey negro...
			u64Destinos = AtaquesAlfil(BB_Mask(pPos->u8PosReyN),u64Vacias) & u64Vacias;
			// ...que son accesibles por alfiles blancos
			u64Destinos &= pPos->u64AtaquesAB;
			while (u64Destinos)
			{
				// Para cada destino debo buscar todas las piezas blancas que pueden ir allí
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				u64Pieza = BB_ATAQUES_ALFIL[u32Hasta] & pPos->u64AlfilesB;
				// Ya tengo todas los alfiles blancos candidatos (¡ojo a "entre"!) a dar jaque
				while (u64Pieza)
				{
					u32Desde = BB_GetBitYQuitar(&u64Pieza);
					if (au64Entre[u32Desde][u32Hasta] & u64Todas) 
						continue;
					Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
			}
		}

		//
		// Torres (ataque directo)
		//
		if (pPos->u64TorresB)
		{
			// Posibles destinos: casillas libres que atacan horz / vert al rey negro...
			u64Destinos = AtaquesTorre(BB_Mask(pPos->u8PosReyN),u64Vacias) & u64Vacias;
			// ...que son accesibles por torres blancas
			u64Destinos &= pPos->u64AtaquesTB;
			while (u64Destinos)
			{
				// Para cada destino debo buscar todas las torres blancas que pueden ir allí
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				u64Pieza = au64AtaquesTorre[u32Hasta] & pPos->u64TorresB;
				// Ya tengo todas las torres blancas candidatas (¡ojo a "entre"!) a dar jaque
				while (u64Pieza)
				{
					u32Desde = BB_GetBitYQuitar(&u64Pieza);
					if (au64Entre[u32Desde][u32Hasta] & u64Todas) 
						continue;
					Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
			}
		}

		//
		// Damas (ataque directo)
		//
		if (pPos->u64DamasB)
		{
			// Posibles casillas libres que atacan (dama) al rey negro...
			u64Destinos = u64Vacias & 
				(
					AtaquesAlfil(BB_Mask(pPos->u8PosReyN),u64Vacias)
					|
					AtaquesTorre(BB_Mask(pPos->u8PosReyN),u64Vacias)
				);
			// ...que son accesibles por damas blancas
			u64Destinos &= pPos->u64AtaquesDB;
			while (u64Destinos)
			{
				// Para cada destino debo buscar todas las damas blancas que pueden ir allí
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				u64Pieza = (BB_ATAQUES_ALFIL[u32Hasta] | au64AtaquesTorre[u32Hasta]) & pPos->u64DamasB;
				// Ya tengo todas las damas blancas candidatas (¡ojo a "entre"!) a dar jaque
				while (u64Pieza)
				{
					u32Desde = BB_GetBitYQuitar(&u64Pieza);
					if (au64Entre[u32Desde][u32Hasta] & u64Todas) 
						continue;
					Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
			}
		}

		//
		// Descubiertas diagonales
		//
		if (pPos->u64AlfilesB | pPos->u64DamasB)
		{
			u64Candidatos = BB_ATAQUES_ALFIL[pPos->u8PosReyN] & 
				(pPos->u64AlfilesB | pPos->u64DamasB);
			while (u64Candidatos)
			{
				u32Desde = BB_GetBitYQuitar(&u64Candidatos);
				u64EntreN = au64Entre[u32Desde][pPos->u8PosReyN] & pPos->u64TodasN;
				// Si hay piezas negras en el camino no hay descubierta posible
				if (!u64EntreN)
				{
					u64EntreB = au64Entre[u32Desde][pPos->u8PosReyN] & pPos->u64TodasB;
					// Tiene que haber al menos una, pq si no hay ni blancas ni negras nos
					// estaríamos comiendo el rey negro
					u32Hasta = BB_GetBitYQuitar(&u64EntreB);
					// He quitado una pieza blanca (en u32Hasta); si no quedan más, genero jugadas
					// para ella
					if (!u64EntreB)
					{
						// Generamos jugadas para esa pieza
						pJugada = GenerarJugadasCasilla(pPos,pJugada,u32Hasta,
							au64Entre[u32Desde][pPos->u8PosReyN]);
					}
				}
			}
		}

		//
		// Descubiertas horz / vert
		//
		if (pPos->u64TorresB | pPos->u64DamasB)
		{
			u64Candidatos = au64AtaquesTorre[pPos->u8PosReyN] & 
				(pPos->u64TorresB | pPos->u64DamasB);
			while (u64Candidatos)
			{
				u32Desde = BB_GetBitYQuitar(&u64Candidatos);
				u64EntreN = au64Entre[u32Desde][pPos->u8PosReyN] & pPos->u64TodasN;
				// Si hay piezas negras en el camino no hay descubierta posible
				if (!u64EntreN)
				{
					u64EntreB = au64Entre[u32Desde][pPos->u8PosReyN] & pPos->u64TodasB;
					// Tiene que haber al menos una, pq si no hay ni blancas ni negras nos
					// estaríamos comiendo el rey negro
					u32Hasta = BB_GetBitYQuitar(&u64EntreB);
					// He quitado una pieza blanca (en u32Hasta); si no quedan más, genero jugadas
					// para ella
					if (!u64EntreB)
					{
						// Generamos jugadas para esa pieza
						pJugada = GenerarJugadasCasilla(pPos,pJugada,u32Hasta,
							au64Entre[u32Desde][pPos->u8PosReyN]);
					}
				}
			}
		}
	} // if (POS_TURNO(pPos) == BLANCAS)
	else
	{
		/*
			+---------------+
			|				|
			|	NEGRAS		|
			|				|
			+---------------+
		*/

		//
		// Peones (ataque directo)
		//
		u64Destinos = au64AtaquesPeonB[pPos->u8PosReyB] & u64Vacias;
		// Un paso
		u64Pieza = BB_GetShiftArriba(u64Destinos) & pPos->u64PeonesN;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			Jug_SetMovEvalCero(pJugada++,u32Desde,u32Desde + 8,0);
		}
		// Dos pasos
		if (Tab_GetFila(pPos->u8PosReyB) == 4)
		{
			u64Pieza = BB_GetShiftArriba(u64Destinos) & u64Vacias;
			u64Pieza = BB_GetShiftArriba(u64Pieza) & pPos->u64PeonesN;
			while (u64Pieza)
			{
				u32Desde = BB_GetBitYQuitar(&u64Pieza);
				Jug_SetMovEvalCero(pJugada++,u32Desde,u32Desde + 16,0);
			}
		}

		//
		// Caballos (ataque directo)
		//
		if (pPos->u64CaballosN)
		{
			// Posibles destinos: casillas en el perímetro caballar del RB atacadas por CN's
			u64Destinos = pPos->u64AtaquesCN & BB_ATAQUES_CABALLO[pPos->u8PosReyB] & u64Vacias;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				u64Pieza = pPos->u64CaballosN & BB_ATAQUES_CABALLO[u32Hasta];
				while (u64Pieza)
				{
					u32Desde = BB_GetBitYQuitar(&u64Pieza);
					Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
			}
		}

		//
		// Alfiles (ataque directo)
		//
		if (pPos->u64AlfilesN)
		{
			// Posibles destinos: casillas libres que atacan diagonalmente al rey blanco...
			u64Destinos = AtaquesAlfil(BB_Mask(pPos->u8PosReyB),u64Vacias) & u64Vacias;
			// ...que son accesibles por alfiles negros
			u64Destinos &= pPos->u64AtaquesAN;
			while (u64Destinos)
			{
				// Para cada destino debo buscar todos los alfiles negros que pueden ir allí
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				u64Pieza = BB_ATAQUES_ALFIL[u32Hasta] & pPos->u64AlfilesN;
				// Ya tengo todas los alfiles negros candidatos (¡ojo a "entre"!) a dar jaque
				while (u64Pieza)
				{
					u32Desde = BB_GetBitYQuitar(&u64Pieza);
					if (au64Entre[u32Desde][u32Hasta] & u64Todas) 
						continue;
					Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
			}
		}

		//
		// Torres (ataque directo)
		//
		if (pPos->u64TorresN)
		{
			// Posibles destinos: casillas libres que atacan horz / vert al rey blanco...
			u64Destinos = AtaquesTorre(BB_Mask(pPos->u8PosReyB),u64Vacias) & u64Vacias;
			// ...que son accesibles por torres negras
			u64Destinos &= pPos->u64AtaquesTN;
			while (u64Destinos)
			{
				// Para cada destino debo buscar todas las torres negras que pueden ir allí
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				u64Pieza = au64AtaquesTorre[u32Hasta] & pPos->u64TorresN;
				// Ya tengo todas las torres negras candidatas (¡ojo a "entre"!) a dar jaque
				while (u64Pieza)
				{
					u32Desde = BB_GetBitYQuitar(&u64Pieza);
					if (au64Entre[u32Desde][u32Hasta] & u64Todas) 
						continue;
					Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
			}
		}

		//
		// Damas (ataque directo)
		//
		if (pPos->u64DamasN)
		{
			// Posibles casillas libres que atacan (dama) al rey blanco...
			u64Destinos = u64Vacias & 
				(
					AtaquesAlfil(BB_Mask(pPos->u8PosReyB),u64Vacias)
					|
					AtaquesTorre(BB_Mask(pPos->u8PosReyB),u64Vacias)
				);
			// ...que son accesibles por damas negras
			u64Destinos &= pPos->u64AtaquesDN;
			while (u64Destinos)
			{
				// Para cada destino debo buscar todas las damas negras que pueden ir allí
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				u64Pieza = (BB_ATAQUES_ALFIL[u32Hasta] | au64AtaquesTorre[u32Hasta]) & pPos->u64DamasN;
				// Ya tengo todas las damas negras candidatas (¡ojo a "entre"!) a dar jaque
				while (u64Pieza)
				{
					u32Desde = BB_GetBitYQuitar(&u64Pieza);
					if (au64Entre[u32Desde][u32Hasta] & u64Todas) 
						continue;
					Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
			}
		}

		//
		// Descubiertas diagonales
		//
		if (pPos->u64AlfilesN | pPos->u64DamasN)
		{
			u64Candidatos = BB_ATAQUES_ALFIL[pPos->u8PosReyB] & 
				(pPos->u64AlfilesN | pPos->u64DamasN);
			while (u64Candidatos)
			{
				u32Desde = BB_GetBitYQuitar(&u64Candidatos);
				u64EntreB = au64Entre[u32Desde][pPos->u8PosReyB] & pPos->u64TodasB;
				// Si hay piezas blancas en el camino no hay descubierta posible
				if (!u64EntreB)
				{
					u64EntreN = au64Entre[u32Desde][pPos->u8PosReyB] & pPos->u64TodasN;
					// Tiene que haber al menos una, pq si no hay ni blancas ni negras nos
					// estaríamos comiendo el rey blanco
					u32Hasta = BB_GetBitYQuitar(&u64EntreN);
					// He quitado una pieza negra (en u32Hasta); si no quedan más, genero jugadas
					// para ella
					if (!u64EntreN)
					{
						// Generamos jugadas para esa pieza
						pJugada = GenerarJugadasCasilla(pPos,pJugada,u32Hasta,
							au64Entre[u32Desde][pPos->u8PosReyB]);
					}
				}
			}
		}

		//
		// Descubiertas horz / vert
		//
		if (pPos->u64TorresN | pPos->u64DamasN)
		{
			u64Candidatos = au64AtaquesTorre[pPos->u8PosReyB] & 
				(pPos->u64TorresN | pPos->u64DamasN);
			while (u64Candidatos)
			{
				u32Desde = BB_GetBitYQuitar(&u64Candidatos);
				u64EntreB = au64Entre[u32Desde][pPos->u8PosReyB] & pPos->u64TodasB;
				// Si hay piezas blancas en el camino no hay descubierta posible
				if (!u64EntreB)
				{
					u64EntreN = au64Entre[u32Desde][pPos->u8PosReyB] & pPos->u64TodasN;
					// Tiene que haber al menos una, pq si no hay ni blancas ni negras nos
					// estaríamos comiendo el rey blanco
					u32Hasta = BB_GetBitYQuitar(&u64EntreN);
					// He quitado una pieza negra (en u32Hasta); si no quedan más, genero jugadas
					// para ella
					if (!u64EntreN)
					{
						// Generamos jugadas para esa pieza
						pJugada = GenerarJugadasCasilla(pPos,pJugada,u32Hasta,
							au64Entre[u32Desde][pPos->u8PosReyB]);
					}
				}
			}
		}
	} // else [if (POS_TURNO(pPos) == BLANCAS)]

	// Al final del proceso, el puntero está justo detrás de la última
	// jugada, así que retrocedemos
	return(pJugada-1);
}




