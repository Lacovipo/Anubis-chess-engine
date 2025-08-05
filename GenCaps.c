
#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"
#include "Funciones.h"
#include "Bitboards_inline.h"
#include "GPT_Magic.h"

/*
 *
 * GenerarCapturasEnJaque
 *
 * Recibe: un puntero a la posición actual, y otro a la	jugada por donde ha de empezar a incluir
 *
 * Devuelve: un puntero a la última jugada generada (por la que empezaremos a analizar hacia atrás)
 *
 * Descripción: Genera todas las capturas y promociones en la posición dada por pPos. Empieza a	almacenar en la
 *  dirección apuntada por pJugada y que constituye un puntero a una posición de la pila global. Esta función se
 *  aplica sólo en posiciones jaqueadas, y genera las capturas que liberan del jaque
 *
 */
static TJugada * GenerarCapturasEnJaque(TPosicion * pPos, TJugada * pJugada)
{
	UINT64 u64Jaqueadores;
	UINT64 u64Temp;
	UINT64 u64Entre;
	UINT64 u64Vacias = ~(pPos->u64TodasB | pPos->u64TodasN);
	UINT32 u32Desde;
	UINT32 u32Hasta;

	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		//
		// Blancas
		//
		u64Jaqueadores = AtaquesN_Hacia(pPos, pPos->u8PosReyB);
		u64Temp = u64Jaqueadores;

		// 1. Capturar pieza jaqueadora
		if (MasDeUnUno(u64Jaqueadores))
		{
			assert(CuentaUnos(u64Jaqueadores) > 1);

			// Si tenemos más de una pieza jaqueándonos, las únicas capturas legales posibles son a piezas enemigas
			//  jaqueadoras que no estén defendidas, y que la captura sea con nuestro rey
			u64Jaqueadores &= (BB_ATAQUES_REY[pPos->u8PosReyB] & ~pPos->u64AtaquesN);
			while(u64Jaqueadores)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Jaqueadores);
				Jug_SetMovEvalCero(pJugada++, pPos->u8PosReyB, u32Hasta, 0);
			}
		}
		else
		{
			// Sólo hay una pieza jaqueando, así que busco todas las mías que la atacan y me la zampo
			u32Hasta = PrimerUno(u64Jaqueadores);
			u64Jaqueadores = AtaquesB_Hacia(pPos,u32Hasta);
			while (u64Jaqueadores)
			{
				// Itero sobre los atacantes de la pieza jaqueadora
				u32Desde = BB_GetBitYQuitar(&u64Jaqueadores);
				if (u32Desde == pPos->u8PosReyB)
				{
					// Si el atacante es mi rey, sólo puedo comérmela si no está defendida
					if (BB_Mask(u32Hasta) & ~pPos->u64AtaquesN)
						Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
				else
				{
					// El atacante no es mi rey

					// Antes que nada, me aseguro de que el atacante no está clavado
					if (!(pPos->u64Clavadas & BB_Mask(u32Desde)))
					{
						// El único caso particular a tener en cuenta es que el capturador sea un peón:
						//  a) al paso (ya contemplado en AtaquesHacia)
						//  b) promoción
						if (BB_Mask(u32Desde) & pPos->u64PeonesB)
						{
							// Sabemos que estamos capturando con un peón, así que comprobamos si está en la penúltima fila,
							//  en cuyo caso generamos promociones
							if (u32Hasta < 8)
							{
								Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, AB);
								Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, TB);
								Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, CB);
								Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, DB);
							}
							else
							{
								if (u32Hasta == (UINT32)pPos->u8AlPaso + TAB_ABAJO && (u32Desde == (UINT32)pPos->u8AlPaso + TAB_ABAJO_IZDA || u32Desde == (UINT32)pPos->u8AlPaso + TAB_ABAJO_DCHA))
									Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta + TAB_ARRIBA, 0);
								else
									Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
							}
						}
						else
							Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
					}
				}
			}
		}

		// 2. Otra opción es que el rey se zampe todo lo que tenga alrededor que esté desprotegido. En u64Temp conservo
		//  los jaqueadores originales para quitarlos ahora (porque ahora sólo considero piezas enemigas que no me
		//  jaquean)
		u64Jaqueadores = BB_ATAQUES_REY[pPos->u8PosReyB] & pPos->u64TodasN & ~pPos->u64AtaquesN & ~u64Temp;
		while(u64Jaqueadores)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Jaqueadores);
			u64Temp = u64Vacias;
			BB_SetBit(&u64Temp, pPos->u8PosReyB);
			// El siguiente switch detecta si la pieza que me voy a zampar está protegida a través de mi rey, es decir,
			//  que un alfil, torre o dama, que me da jaque, apunta a esa pieza a través de mí
			// OJO: esto se puede optimizar; primero, antes del costoso "Ataques...()", había que ver si hay algún jaqueador
			//  enemigo en esa fila/columna/diagonal (debo conservar los jaqueadores originales, porque ese cálculo
			//  también es costoso); después, comprobamos el "Entre[]", que ha de ser vacío para que me esté jaqueando
			switch(u32Hasta - pPos->u8PosReyB)
			{
				case TAB_ARRIBA_IZDA:
					u64Entre = AtaquesAbajoDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ARRIBA:
					u64Entre = AtaquesAbajo(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ARRIBA_DCHA:
					u64Entre = AtaquesAbajoIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_IZQUIERDA:
					u64Entre = AtaquesDerecha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_DERECHA:
					u64Entre = AtaquesIzquierda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ABAJO_IZDA:
					u64Entre = AtaquesArribaDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ABAJO:
					u64Entre = AtaquesArriba(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresN | pPos->u64DamasN))
						goto otra_while;
					break;
				case TAB_ABAJO_DCHA:
					u64Entre = AtaquesArribaIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesN | pPos->u64DamasN))
						goto otra_while;
					break;
			}

			Jug_SetMovEvalCero(pJugada++, pPos->u8PosReyB, u32Hasta, 0);
			assert(JugadaCorrecta(pPos, *(pJugada - 1)));
otra_while: ;
		}
	}
	else
	{
		//
		// Negras
		//
		u64Jaqueadores = AtaquesB_Hacia(pPos, pPos->u8PosReyN);
		u64Temp = u64Jaqueadores;

		// 1. Capturar pieza jaqueadora
		if (MasDeUnUno(u64Jaqueadores))
		{
			assert(CuentaUnos(u64Jaqueadores) > 1);

			// Si tenemos más de una pieza jaqueándonos, las únicas capturas legales posibles son a piezas enemigas
			//  jaqueadoras que no estén defendidas, y que la captura sea con nuestro rey
			u64Jaqueadores &= (BB_ATAQUES_REY[pPos->u8PosReyN] & ~pPos->u64AtaquesB);
			while(u64Jaqueadores)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Jaqueadores);
				Jug_SetMovEvalCero(pJugada++, pPos->u8PosReyN, u32Hasta, 0);
			}
		}
		else
		{
			// Sólo hay una pieza jaqueando, así que busco todas las mías que la atacan y me la zampo
			u32Hasta = PrimerUno(u64Jaqueadores);
			u64Jaqueadores = AtaquesN_Hacia(pPos,u32Hasta);
			while (u64Jaqueadores)
			{
				// Itero sobre los atacantes de la pieza jaqueadora
				u32Desde = BB_GetBitYQuitar(&u64Jaqueadores);
				if (u32Desde == pPos->u8PosReyN)
				{
					// Si el atacante es mi rey, sólo puedo comérmela si no está defendida
					if (BB_Mask(u32Hasta) & ~pPos->u64AtaquesB)
						Jug_SetMovEvalCero(pJugada++,u32Desde,u32Hasta,0);
				}
				else
				{
					// El atacante no es mi rey
					
					// Antes que nada, me aseguro de que el atacante no está clavado
					if (!(BB_Mask(u32Desde) & pPos->u64Clavadas))
					{
						// El único caso particular a tener en cuenta es que el capturador sea un peón:
						//  a) al paso (ya contemplado en AtaquesHacia)
						//  b) promoción
						if (BB_Mask(u32Desde) & pPos->u64PeonesN)
						{
							// Sabemos que estamos capturando con un peón, así que comprobamos si está en la penúltima fila,
							//  en cuyo caso generamos promociones
							if (u32Hasta > 55)
							{
								Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, AB);
								Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, TB);
								Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, CB);
								Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, DB);
							}
							else
							{
								if (u32Hasta == (UINT32)pPos->u8AlPaso + TAB_ARRIBA && (u32Desde == (UINT32)pPos->u8AlPaso + TAB_ARRIBA_DCHA || u32Desde == (UINT32)pPos->u8AlPaso + TAB_ARRIBA_IZDA))
									Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta + TAB_ABAJO, 0);
								else
									Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta,0);
							}
						}
						else
							Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
					}
				}
			}
		}

		// 2. Otra opción es que el rey se zampe todo lo que tenga alrededor que esté desprotegido. En u64Temp conservo
		//  los jaqueadores originales para quitarlos ahora (porque ahora sólo considero piezas enemigas que no me
		//  jaquean)
		u64Jaqueadores = BB_ATAQUES_REY[pPos->u8PosReyN] & pPos->u64TodasB & ~pPos->u64AtaquesB & ~u64Temp;
		while(u64Jaqueadores)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Jaqueadores);
			u64Temp = u64Vacias;
			BB_SetBit(&u64Temp, pPos->u8PosReyN);
			// El siguiente switch detecta si la pieza que me voy a zampar está protegida a través de mi rey, es decir,
			//  que un alfil, torre o dama, que me da jaque, apunta a esa pieza a través de mí
			// OJO: esto se puede optimizar; primero, antes del costoso "Ataques...()", había que ver si hay algún jaqueador
			//  enemigo en esa fila/columna/diagonal (debo conservar los jaqueadores originales, porque ese cálculo
			//  también es costoso); después, comprobamos el "Entre[]", que ha de ser vacío para que me esté jaqueando
			switch(u32Hasta - pPos->u8PosReyN)
			{
				case TAB_ARRIBA_IZDA:
					u64Entre = AtaquesAbajoDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case TAB_ARRIBA:
					u64Entre = AtaquesAbajo(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case TAB_ARRIBA_DCHA:
					u64Entre = AtaquesAbajoIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case TAB_IZQUIERDA:
					u64Entre = AtaquesDerecha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case TAB_DERECHA:
					u64Entre = AtaquesIzquierda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case TAB_ABAJO_IZDA:
					u64Entre = AtaquesArribaDcha(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case TAB_ABAJO:
					u64Entre = AtaquesArriba(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64TorresB | pPos->u64DamasB))
						goto otra_while2;
					break;
				case TAB_ABAJO_DCHA:
					u64Entre = AtaquesArribaIzda(BB_Mask(u32Hasta), u64Temp);
					if (u64Entre & (pPos->u64AlfilesB | pPos->u64DamasB))
						goto otra_while2;
					break;
			}

			Jug_SetMovEvalCero(pJugada++, pPos->u8PosReyN, u32Hasta, 0);
			assert(JugadaCorrecta(pPos, *(pJugada - 1)));
otra_while2: ;
		}
	}

	// Al final del proceso, el puntero está justo detrás de la última jugada, así que retrocedemos
	return(pJugada-1);
}

/*
 * GenerarCapturas
 *
 * Recibe: un puntero a la posición actual, y otro a la jugada por donde ha de empezar a incluir
 *
 * Devuelve: un puntero a la última jugada generada (por la que empezaremos a analizar hacia atrás)
 *
 * Descripción: Genera todas las capturas y promociones en la posición dada por pPos. Empieza a	almacenar en la
 *  dirección apuntada por pJugada y que constituye un puntero a una posición de la pila global
 *
 */
TJugada * GenerarCapturas(TPosicion * pPos, TJugada * pJugada, BOOL bSubPromociones)
{
	UINT64 u64Pieza;
	UINT64 u64Destinos;
	UINT64 u64TodasRival;
	UINT64 u64Todas;
	UINT32 u32Desde;
	UINT32 u32Hasta;

	if (Pos_GetJaqueado(pPos))
		return(GenerarCapturasEnJaque(pPos, pJugada));

	if (Pos_GetTurno(pPos) == BLANCAS)
	{
		//
		// BLANCAS
		//

		u64TodasRival = pPos->u64TodasN;
		u64Todas = pPos->u64TodasB | u64TodasRival;

		// Rey
		u32Desde = (UINT32)pPos->u8PosReyB;
		u64Destinos = BB_ATAQUES_REY[u32Desde] & u64TodasRival;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
		}

		// Caballos
		u64Pieza = pPos->u64CaballosB;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = BB_ATAQUES_CABALLO[u32Desde] & u64TodasRival;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
			}
		}

	#if defined(_DEBUG)
		int Cuenta = GenerarMovimientosDeslizantes(pPos, pJugada, TRUE);
		int CuentaNew = 0;
		// Torres y damas
		u64Pieza = pPos->u64TorresB | pPos->u64DamasB;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = au64AtaquesTorre[u32Desde] & u64TodasRival;
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
			u64Destinos = BB_ATAQUES_ALFIL[u32Desde] & u64TodasRival;
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
		pJugada += Cuenta;
	#else
		pJugada += GenerarMovimientosDeslizantes(pPos, pJugada, TRUE);
	#endif

		// Peones
		u64Pieza = pPos->u64PeonesB;
		// P.1: Promociones
		u64Destinos = ((u64Pieza & BB_FILA7H7) << 8) & (~u64Todas);
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta + TAB_ABAJO;
			if (bSubPromociones)
			{
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, AB);
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, TB);
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, CB);
			}
			Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, DB);
		}
		// P.2: Al paso
		if (pPos->u8AlPaso != TAB_ALPASOIMPOSIBLE)
		{
			u32Hasta = (UINT32)pPos->u8AlPaso;
			u64Destinos = au64AdyacenteH[u32Hasta + TAB_ABAJO] & u64Pieza;
			while (u64Destinos)
			{
				u32Desde = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
			}
		}
		// P.3: Capturas derecha (-7)
		u64Destinos = ((u64Pieza & BB_SINCOLDCHA) << 7) & u64TodasRival;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta + TAB_ABAJO_IZDA;
			if (u32Hasta < 8)
			{
				// Promoción
				if (bSubPromociones)
				{
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, AB);
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, TB);
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, CB);
				}
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, DB);
			}
			else
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
		}
		// P.4: Capturas izquierda (-9)
		u64Destinos = ((u64Pieza & BB_SINCOLIZDA) << 9) & u64TodasRival;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta + TAB_ABAJO_DCHA;
			if (u32Hasta < 8)
			{
				// Promoción
				if (bSubPromociones)
				{
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, AB);
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, TB);
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, CB);
				}
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, DB);
			}
			else
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
		}
	} // if (POS_TURNO(pPos) == BLANCAS)
	else
	{
		//
		// NEGRAS
		//

		u64TodasRival = pPos->u64TodasB;
		u64Todas = pPos->u64TodasN | u64TodasRival;

		// Rey
		u32Desde = (UINT32)pPos->u8PosReyN;
		u64Destinos = BB_ATAQUES_REY[u32Desde] & u64TodasRival;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
		}

		// Caballos
		u64Pieza = pPos->u64CaballosN;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = BB_ATAQUES_CABALLO[u32Desde] & u64TodasRival;
			while (u64Destinos)
			{
				u32Hasta = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
			}
		}

	#if defined(_DEBUG)
		int Cuenta = GenerarMovimientosDeslizantes(pPos, pJugada, TRUE);
		int CuentaNew = 0;
		// Torres y damas
		u64Pieza = pPos->u64TorresN | pPos->u64DamasN;
		while (u64Pieza)
		{
			u32Desde = BB_GetBitYQuitar(&u64Pieza);
			u64Destinos = au64AtaquesTorre[u32Desde] & u64TodasRival;
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
			u64Destinos = BB_ATAQUES_ALFIL[u32Desde] & u64TodasRival;
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
		pJugada += Cuenta;
	#else
		pJugada += GenerarMovimientosDeslizantes(pPos, pJugada, TRUE);
	#endif

		// Peones
		u64Pieza = pPos->u64PeonesN;
		// P.1: Promociones
		u64Destinos = ((u64Pieza & BB_FILA2H2) >> 8) & (~u64Todas);
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta + TAB_ARRIBA;
			if (bSubPromociones)
			{
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, AB);	// Son blancos a propósito
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, TB);	// para mantener la coherencia
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, CB);	// en todo el programa
			}
			Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, DB);
		}
		// P.2: Al paso
		if (pPos->u8AlPaso != TAB_ALPASOIMPOSIBLE)
		{
			u32Hasta = (UINT32)pPos->u8AlPaso;
			u64Destinos = au64AdyacenteH[u32Hasta + TAB_ARRIBA] & u64Pieza;
			while (u64Destinos)
			{
				u32Desde = BB_GetBitYQuitar(&u64Destinos);
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
			}
		}
		// P.3: Capturas derecha (+9)
		u64Destinos = ((u64Pieza & BB_SINCOLDCHA) >> 9) & u64TodasRival;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta + TAB_ARRIBA_IZDA;
			if (u32Hasta > 55)
			{
				// Promoción
				if (bSubPromociones)
				{
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, AB);
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, TB);
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, CB);
				}
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, DB);
			}
			else
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
		}
		// P.4: Capturas izquierda (+7)
		u64Destinos = ((u64Pieza & BB_SINCOLIZDA) >> 7) & u64TodasRival;
		while (u64Destinos)
		{
			u32Hasta = BB_GetBitYQuitar(&u64Destinos);
			u32Desde = u32Hasta + TAB_ARRIBA_DCHA;
			if (u32Hasta > 55)
			{
				// Promoción
				if (bSubPromociones)
				{
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, AB);
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, TB);
					Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, CB);
				}
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, DB);
			}
			else
				Jug_SetMovEvalCero(pJugada++, u32Desde, u32Hasta, 0);
		}
	} // else [if (POS_TURNO(pPos) == BLANCAS)]

	// Al final del proceso, el puntero está justo detrás de la última jugada, así que retrocedemos
	return(pJugada-1);
}

