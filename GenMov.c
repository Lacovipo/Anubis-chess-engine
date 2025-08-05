/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de implementación del generador de
	movimientos que no sean capturas
*/
#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"
#include "Bitboards_inline.h"
#include "Funciones.h"
#include "GPT_Magic.h"

static const SINT8 as8MovReyMJ[] =
{
	0,  2, 1, 0, 0, 0, 2, 0,
	0,  0,-1,-2,-2,-2, 0, 0,
	-1,-2,-3,-4,-4,-3,-2,-1,
	-2,-3,-4,-5,-5,-4,-3,-2,
	-2,-3,-4,-5,-5,-4,-3,-2,
	-1,-2,-3,-4,-4,-3,-2,-1,
	0,  0,-1,-2,-2,-2, 0, 0,
	0,  2, 1, 0, 0, 0, 2, 0
};
static const SINT8 as8MovReyF[] =
{
	-4,-3,-2,-1,-1,-2,-3,-4,
	-3,-2,-1, 0, 0,-1,-2,-3,
	-2,-1, 0, 1, 1, 0,-1,-2,
	-1, 0, 1, 2, 2, 0, 1,-1,
	-1, 0, 1, 2, 2, 0, 1,-1,
	-2,-1, 0, 1, 1, 0,-1,-2,
	-3,-2,-1, 0, 0,-1,-2,-3,
	-4,-3,-2,-1,-1,-2,-3,-4
};
static const SINT8 as8MovCaballo[] =
{
	-9,-3,-2,-1,-1,-2,-3,-9,
	-5, 0, 0, 1, 1, 0, 0,-5,
	-2, 2, 3, 2, 2, 3, 2,-2,
	-1, 2, 2, 4, 4, 2, 2,-1,
	-1, 2, 2, 4, 4, 2, 2,-1,
	-2, 2, 3, 2, 2, 3, 2,-2,
	-5, 0, 0, 1, 1, 0, 0,-5,
	-9,-3,-2,-1,-1,-2,-3,-9
};
static const SINT8 as8MovAlfilB[] =
{
	-5,-5,-5,-5,-5,-5,-5,-5,
	-4,-4,-4,-4,-4,-4,-4,-4,
	-3,-3,-3,-3,-3,-3,-3,-3,
	-2, 5, 2, 2, 2, 2, 5,-2
	-2, 1, 5, 3, 3, 5, 1,-2,
	 0, 3, 4, 5, 5, 4, 2, 0,
	-2, 5, 1, 3, 3, 1, 5,-2,
	 0,-2,-2,-3,-3,-2,-2, 0
};
static const SINT8 as8MovAlfilN[] =
{
	 0,-2,-2,-3,-3,-2,-2, 0,
	-2, 5, 1, 3, 3, 1, 5,-2,
	 0, 3, 4, 5, 5, 4, 2, 0,
	-2, 1, 5, 3, 3, 5, 1,-2,
	-2, 5, 2, 2, 2, 2, 5,-2
	-3,-3,-3,-3,-3,-3,-3,-3,
	-4,-4,-4,-4,-4,-4,-4,-4,
	-5,-5,-5,-5,-5,-5,-5,-5
};

/*
	*************************************************************
	*															*
	*	GenerarMovimientosEnJaque								*
	*															*
	*															*
	*	Recibe: un puntero a la posición actual y otro a la		*
	*			jugada donde debe empezar a introducir			*
	*															*
	*	Devuelve: un puntero a la última jugada	generada (por	*
	*			  la que empezaremos a analizar hacia atrás)	*
	*															*
	*	Descripción: Genera todas jugadas que no son capturas	*
	*				 ni promociones	en la posición dada por		*
	*				 pPos. Empieza a almacenar en la dirección	*
	*				 apuntada por pJugada, que constituye un	*
	*				 puntero a	una posición de la pila global.	*
	*				 Esta función se aplica sólo en posiciones	*
	*				 jaqueadas, y genera las no-capturas que	*
	*				 liberan del jaque							*
	*				 Casuística:								*
	*					- Movimientos de rey a casillas vacías	*
	*					  no atacadas por el enemigo			*
	*					- Si sólo hay un jaqueador, y es pieza	*
	*					  deslizante, ponemos algo en medio		*
	*															*
	*************************************************************
*/
TJugada * GenerarMovimientosEnJaque(TPosicion * pPos,TJugada * pJugada)
{
	UINT64 u64CasillasAccesibles;
	UINT64 u64Jaqueadores;
	UINT64 u64Entre;
	UINT64 u64Vacias = ~(pPos->u64TodasB | pPos->u64TodasN);
	UINT64 u64Temp;
	UINT32 u32Desde;
	UINT32 u32Hasta;

	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		//
		// Jugadas de rey a casillas libres
		// 14/06/2003 - No debo permitir mover el rey a lo largo de una fila o diagonal tapada por
		//				él mismo, y que siga siendo jaque, como en
		//				8/6p1/P1p2k1p/2p4P/8/8/2K4P/1q6 w - - 0 1
		//				Por ahora, está cutremente controlado en QsearchConJaques, pero eso falsea
		//				las extensiones basadas en el nùmero de jugadas legales
		// 15/06/2003 - Resuelvo la nota anterior con el switch
		//
		u64CasillasAccesibles = BB_ATAQUES_REY[pPos->u8PosReyB] & ~pPos->u64AtaquesN & u64Vacias;
		while (u64CasillasAccesibles)
		{
			u32Hasta = BB_GetBitYQuitar(&u64CasillasAccesibles);

			u64Temp = u64Vacias;
			BB_SetBit(&u64Temp,pPos->u8PosReyB);
			switch(u32Hasta-pPos->u8PosReyB)
			{
				case -9:
					u64Entre = AtaquesAbajoDcha(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case -8:
					u64Entre = AtaquesAbajo(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case -7:
					u64Entre = AtaquesAbajoIzda(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case -1:
					u64Entre = AtaquesDerecha(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case 1:
					u64Entre = AtaquesIzquierda(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case 7:
					u64Entre = AtaquesArribaDcha(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case 8:
					u64Entre = AtaquesArriba(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case 9:
					u64Entre = AtaquesArribaIzda(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
			}

			Jug_SetMovEvalCero(pJugada++,pPos->u8PosReyB,u32Hasta,0);
			assert(JugadaCorrecta(pPos,*(pJugada - 1)));
otra_while: ;
		}
		//
		// Veamos si podemos interponer algo
		//
		u64Jaqueadores = AtaquesN_Hacia(pPos,pPos->u8PosReyB);
		if (CuentaUnos(u64Jaqueadores) == 1)
		{
			u64Jaqueadores = AtaquesDeslizN_Hacia(pPos,pPos->u8PosReyB);
			if (u64Jaqueadores)
			{
				//
				// Ya sabemos que sólo hay una pieza, y que es deslizante, así que
				// intentamos tapar la línea
				// Ojo: no nos limitamos a casillas atacadas por las blancas porque puede
				// haber una avance de peón que tape la fila
				//
				u64Entre = au64Entre[PrimerUno(u64Jaqueadores)][pPos->u8PosReyB];
				//
				// En u64Entre tenemos las casillas que están entre el jaqueador y
				// nuestro rey, y que podemos ocupar con una pieza nuestra. Iteramos
				// sobre dicho bitmap, localizando qué piezas nuestras atacan esas
				// casillas, y generando las jugadas correspondientes, haciendo excepción
				// de nuestro rey, claro está
				//
				while (u64Entre)
				{
					u32Hasta = BB_GetBitYQuitar(&u64Entre);
					u64Jaqueadores = MovimientosB_Hacia(pPos,u32Hasta);
					while (u64Jaqueadores)
					{
						u32Desde = BB_GetBitYQuitar(&u64Jaqueadores);
						if (u32Desde != pPos->u8PosReyB)
						{
							//
							// Antes que nada, me aseguro de que el bloqueador no está clavado
							//
							if (!(BB_Mask(u32Desde) & pPos->u64Clavadas))
							{
								//
								// El único caso particular a tener
								// en cuenta es que el bloqueador sea un peón y que corone
								//
								if (BB_Mask(u32Desde) & pPos->u64PeonesB)
								{
									//
									// Sabemos que estamos bloqueando con un peón, así que
									// comprobamos si está en la penúltima fila, en cuyo 
									// caso generamos promociones
									//
									if (u32Hasta < 8)
									{
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,CB);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,AB);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,TB);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,DB);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
									}
									else
									{
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
									}
								} // if (BB_MASK(u32Desde) & pPos->u64PeonesB)
								else
								{
									Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
									assert(JugadaCorrecta(pPos,*(pJugada - 1)));
								}
							}
						} // if (u32Desde != pPos->u8PosReyB)
					} // while (u64Jaqueadores)
				} // while (u64Entre)
			} // if (u64Jaqueadores)
		} // if (CuentaUnos(u64Jaqueadores) == 1)
	} // if (Pos_GetTurno(pPos) == BLANCAS)
	else ///////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		//
		// Jugadas de rey a casillas libres
		//
		u64CasillasAccesibles = BB_ATAQUES_REY[pPos->u8PosReyN] & ~pPos->u64AtaquesB & u64Vacias;
		while (u64CasillasAccesibles)
		{
			u32Hasta = BB_GetBitYQuitar(&u64CasillasAccesibles);

			u64Temp = u64Vacias;
			BB_SetBit(&u64Temp,pPos->u8PosReyN);
			switch(u32Hasta-pPos->u8PosReyN)
			{
				case -9:
					u64Entre = AtaquesAbajoDcha(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case -8:
					u64Entre = AtaquesAbajo(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case -7:
					u64Entre = AtaquesAbajoIzda(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case -1:
					u64Entre = AtaquesDerecha(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case 1:
					u64Entre = AtaquesIzquierda(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case 7:
					u64Entre = AtaquesArribaDcha(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case 8:
					u64Entre = AtaquesArriba(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case 9:
					u64Entre = AtaquesArribaIzda(BB_Mask(u32Hasta),u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_while2;
					break;
			}

			Jug_SetMovEvalCero(pJugada++,pPos->u8PosReyN,u32Hasta,0);
			assert(JugadaCorrecta(pPos,*(pJugada - 1)));
otra_while2: ;
		}
		//
		// Veamos si podemos interponer algo
		//
		u64Jaqueadores = AtaquesB_Hacia(pPos,pPos->u8PosReyN);
		if (CuentaUnos(u64Jaqueadores) == 1)
		{
			u64Jaqueadores = AtaquesDeslizB_Hacia(pPos,pPos->u8PosReyN);
			if (u64Jaqueadores)
			{
				//
				// Ya sabemos que sólo hay una pieza, y que es deslizante, así que
				// intentamos tapar la línea
				//
				u64Entre = au64Entre[PrimerUno(u64Jaqueadores)][pPos->u8PosReyN];
				//
				// En u64Entre tenemos las casillas que están entre el jaqueador y
				// nuestro rey, y que podemos ocupar con una pieza nuestra. Iteramos
				// sobre dicho bitmap, localizando qué piezas nuestras atacan esas
				// casillas, y generando las jugadas correspondientes, haciendo excepción
				// de nuestro rey, claro está
				//
				while (u64Entre)
				{
					u32Hasta = BB_GetBitYQuitar(&u64Entre);
					u64Jaqueadores = MovimientosN_Hacia(pPos,u32Hasta);
					while (u64Jaqueadores)
					{
						u32Desde = BB_GetBitYQuitar(&u64Jaqueadores);
						if (u32Desde != pPos->u8PosReyN)
						{
							//
							// Antes que nada, me aseguro de que el bloqueador no está clavado
							//
							if (!(BB_Mask(u32Desde) & pPos->u64Clavadas))
							{
								//
								// El único caso particular a tener
								// en cuenta es que el bloqueador sea un peón y que corone
								//
								if (BB_Mask(u32Desde) & pPos->u64PeonesN)
								{
									//
									// Sabemos que estamos bloqueando con un peón, así que
									// comprobamos si está en la penúltima fila, en cuyo 
									// caso generamos promociones
									//
									if (u32Hasta > 55)
									{
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,CB);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,AB);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,TB);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,DB);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
									}
									else
									{
										Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
										assert(JugadaCorrecta(pPos,*(pJugada - 1)));
									}
								} // if (BB_MASK(u32Desde) & pPos->u64PeonesN)
								else
								{
									Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
									assert(JugadaCorrecta(pPos,*(pJugada - 1)));
								}
							}
						} // if (u32Desde != pPos->u8PosReyN)
					} // while (u64Jaqueadores)
				} // while (u64Entre)
			} // if (u64Jaqueadores)
		} // if (CuentaUnos(u64Jaqueadores) == 1)
	} // else [if (Pos_GetTurno(pPos) == BLANCAS)]

	// Al final del proceso, el puntero está justo detrás de la última
	// jugada, así que retrocedemos
	return(pJugada-1);
}

/*
	*************************************************************
	*															*
	*	GenerarMovimientos										*
	*															*
	*															*
	*	Recibe: un puntero a la posición actual y otro a la		*
	*			jugada donde debe empezar a introducir			*
	*															*
	*	Devuelve: un puntero a la última jugada	generada (por	*
	*			  la que empezaremos a analizar hacia atrás)	*
	*															*
	*	Descripción: Genera todas jugadas que no son capturas	*
	*				 ni promociones	en la posición dada por		*
	*				 pPos. Empieza a almacenar en la dirección	*
	*				 apuntada por pJugada, que constituye un	*
	*				 puntero a	una posición de la pila global	*
	*															*
	*	Nota: Utilizo la macro BB_INVERTIRBIT en lugar de la	*
	*		  lógica BB_CLEARBIT porque es más rápida y, en		*
	*		  este caso, perfectamente segura porque sabemos	*
	*		  lo que hay										*
	*															*
	*************************************************************
*/
TJugada * GenerarMovimientos(TPosicion * pPos,TJugada * pJugada)
{
	UINT64		u64Pieza;
	UINT64		u64Destinos;
	UINT64		u64Vacias;
	UINT64		u64Todas;
	UINT32		u32Desde;
	UINT32		u32Hasta;

	if (Pos_GetJaqueado(pPos))
		return(GenerarMovimientosEnJaque(pPos,pJugada));

	u64Todas = pPos->u64TodasB | pPos->u64TodasN;
	u64Vacias = ~u64Todas;

	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		//
		// BLANCAS
		//

		// Rey
		u32Desde = (UINT32)pPos->u8PosReyB;
		u64Destinos = BB_ATAQUES_REY[u32Desde] & u64Vacias;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);

			Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
		}
		if (pPos->u8PosReyB == TAB_E1)
		{
			if (Pos_GetPuedeCortoB(pPos) && !(u64Todas & BB_F1G1) && (pPos->u64TorresB & BB_Mask(TAB_H1)))
			{
				if (!Atacado(pPos, TAB_F1, NEGRAS) && !Atacado(pPos, TAB_G1, NEGRAS))
					Jug_SetMovConHistoria(pJugada++, TAB_E1, TAB_G1, 0);
			}
			if (Pos_GetPuedeLargoB(pPos) && !(u64Todas & BB_B1C1D1) && (pPos->u64TorresB & BB_Mask(TAB_A1)))
			{
				if (!Atacado(pPos, TAB_C1, NEGRAS) && !Atacado(pPos, TAB_D1, NEGRAS))
					Jug_SetMovConHistoria(pJugada++, TAB_E1, TAB_C1, 0);
			}
		}

		// Caballos
		u64Pieza = pPos->u64CaballosB;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = BB_ATAQUES_CABALLO[u32Desde] & u64Vacias;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
			}
		}

	#if defined(_DEBUG)
		int Cuenta = GenerarMovimientosDeslizantes(pPos, pJugada, FALSE);
		int CuentaNew = 0;
		// Torres y damas
		u64Pieza = pPos->u64TorresB | pPos->u64DamasB;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = au64AtaquesTorre[u32Desde] & u64Vacias;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				// Si hay alguna pieza entre origen y destino, pasamos a otra
				if (au64Entre[u32Desde][u32Hasta] & u64Todas)
					continue;
				CuentaNew++;
			}
		}

		// Alfiles y damas
		u64Pieza = pPos->u64AlfilesB | pPos->u64DamasB;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = BB_ATAQUES_ALFIL[u32Desde] & u64Vacias;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				// Si hay alguna pieza entre origen y destino, pasamos a otra
				if (au64Entre[u32Desde][u32Hasta] & u64Todas)
					continue;
				CuentaNew++;
			}
		}
		assert(Cuenta == CuentaNew);
		if (Cuenta)
			pJugada += Cuenta;
	#else
		pJugada += GenerarMovimientosDeslizantes(pPos, pJugada, FALSE);
	#endif

		// Peones (las promociones van en GenerarCapturas)
		u64Pieza = pPos->u64PeonesB & (~BB_FILA7H7);
		// P.1: Un paso (-8)
		u64Destinos = (u64Pieza << 8) & u64Vacias;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta + 8;
			Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
		}
		// P.2: Dos pasos (-16)
		u64Destinos = ((((u64Pieza & BB_FILA2H2) << 8) & u64Vacias) << 8) & u64Vacias;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta + 16;
			Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
		}
	} // if (POS_TURNO(pPos) == BLANCAS)
	else
	{
		//
		// NEGRAS
		//

		// Rey
		u32Desde = (UINT32)pPos->u8PosReyN;
		u64Destinos = BB_ATAQUES_REY[u32Desde] & u64Vacias;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
		}
		if (pPos->u8PosReyN == TAB_E8)
		{
			if (Pos_GetPuedeCortoN(pPos) && !(u64Todas & BB_F8G8) && (pPos->u64TorresN & BB_Mask(TAB_H8)))
			{
				if (!Atacado(pPos, TAB_F8, BLANCAS) && !Atacado(pPos, TAB_G8, BLANCAS))
				{
					Jug_SetMovConHistoria(pJugada++, TAB_E8, TAB_G8, 0);
				}
			}
			if (Pos_GetPuedeLargoN(pPos) && !(u64Todas & BB_B8C8D8) && (pPos->u64TorresN & BB_Mask(TAB_A8)))
			{
				if (!Atacado(pPos, TAB_C8, BLANCAS) && !Atacado(pPos, TAB_D8, BLANCAS))
				{
					Jug_SetMovConHistoria(pJugada++, TAB_E8, TAB_C8, 0);
				}
			}
		}

		// Caballos
		u64Pieza = pPos->u64CaballosN;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = BB_ATAQUES_CABALLO[u32Desde] & u64Vacias;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
			}
		}

	#if defined(_DEBUG)
		int Cuenta = GenerarMovimientosDeslizantes(pPos, pJugada, FALSE);
		int CuentaNew = 0;
		// Torres y damas
		u64Pieza = pPos->u64TorresN | pPos->u64DamasN;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = au64AtaquesTorre[u32Desde] & u64Vacias;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				// Si hay alguna pieza entre origen y destino, pasamos a otra
				if (au64Entre[u32Desde][u32Hasta] & u64Todas)
					continue;
				CuentaNew++;
			}
		}

		// Alfiles y damas
		u64Pieza = pPos->u64AlfilesN | pPos->u64DamasN;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = BB_ATAQUES_ALFIL[u32Desde] & u64Vacias;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				// Si hay alguna pieza entre origen y destino, pasamos a otra
				if (au64Entre[u32Desde][u32Hasta] & u64Todas)
					continue;
				CuentaNew++;
			}
		}
		assert(Cuenta == CuentaNew);
		if (Cuenta)
			pJugada += Cuenta;
	#else
		pJugada += GenerarMovimientosDeslizantes(pPos, pJugada, FALSE);
	#endif

		// Peones (las promociones van en GenerarCapturas)
		u64Pieza = pPos->u64PeonesN & (~BB_FILA2H2);
		// P.1: Un paso (+8)
		u64Destinos = (u64Pieza >> 8) & u64Vacias;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta - 8;
			Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
		}
		// P.2: Dos pasos (+16)
		u64Destinos = ((((u64Pieza & BB_FILA7H7) >> 8) & u64Vacias) >> 8) & u64Vacias;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta - 16;
			Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
		}
	} // else [if (POS_TURNO(pPos) == BLANCAS)]

	// Al final del proceso, el puntero está justo detrás de la última
	// jugada, así que retrocedemos
	return(pJugada-1);
}