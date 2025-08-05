/*
	Anubis

	Copyright Jos� Carlos Mart�nez Gal�n
	Todos los derechos reservados

	-------------------------------------

	M�dulo de implementaci�n de la funci�n
	MoverNull
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"
#include "Funciones.h"

void MoverNull(TPosicion * pPos)
{
	TPosicion *	pPosNueva = pPos + 1;

	*pPosNueva = *pPos;

	// No se pueden hacer dos null moves seguidos
	Pos_SetNoIntentarNull(pPosNueva);

	Pos_CambiarTurno(pPosNueva);
	pPosNueva->u8Cincuenta += 2;
	pPosNueva->s32Eval = NO_EVAL;
	pPosNueva->pListaJug++;
	dbDatosBusqueda.u32NumNulos++;
	pPosNueva->u64HashSignature = pPos->u64HashSignature ^ au64AlPaso_Random[pPos->u8AlPaso];
	pPosNueva->u64HashSignature ^= au64AlPaso_Random[TAB_ALPASOIMPOSIBLE];
	pPosNueva->u8AlPaso = TAB_ALPASOIMPOSIBLE;

	// Si no reseteo el puntero de repeticiones, el segundo null move no detecta zugzwang (la posici�n se considera tablas por la repetici�n)
	pPosNueva->pPunteroRepeticion = pPosNueva;

	pPos->UltMovPieza = pPos->UltMovHasta = 0;
}