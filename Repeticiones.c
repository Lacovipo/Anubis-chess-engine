/*
	Anubis

	José Carlos Martínez Galán
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Inline.h"

/*
	*********************************************************************************
	*																				*
	*	SegundaRepeticion															*
	*																				*
	*																				*
	*	Recibe: Puntero a la posición actual										*
	*																				*
	*	Descripción: Recorre la lista de posiciones hacia atrás, en busca de una	*
	*				 coincidencia con la clave hash actual, en cuyo caso devuelve	*
	*				 TRUE.															*
	*																				*
	*********************************************************************************
*/
BOOL SegundaRepeticion(TPosicion * pPos)
{
	TPosicion * pPos2;

	for (pPos2 = pPos - 2;pPos2 >= pPos->pPunteroRepeticion;pPos2--)
	{
		if (pPos->u64HashSignature == pPos2->u64HashSignature
			&& Pos_GetTurno(pPos) == Pos_GetTurno(pPos2))
		{
			//assert(pPos->s32Eval == pPos2->s32Eval);
			assert(pPos->u64PeonesB == pPos2->u64PeonesB);
			assert(pPos->u64PeonesN == pPos2->u64PeonesN);
			assert(pPos->u64CaballosB == pPos2->u64CaballosB);
			assert(pPos->u64CaballosN == pPos2->u64CaballosN);
			assert(pPos->u64AlfilesB == pPos2->u64AlfilesB);
			assert(pPos->u64AlfilesN == pPos2->u64AlfilesN);
			assert(pPos->u64TorresB == pPos2->u64TorresB);
			assert(pPos->u64TorresN == pPos2->u64TorresN);
			assert(pPos->u64DamasB == pPos2->u64DamasB);
			assert(pPos->u64DamasN == pPos2->u64DamasN);
			assert(pPos->u8PosReyB == pPos2->u8PosReyB);
			assert(pPos->u8PosReyN == pPos2->u8PosReyN);
			assert(pPos->u64TodasB == pPos2->u64TodasB);
			assert(pPos->u64TodasN == pPos2->u64TodasN);
			return(TRUE);
		}
	}
	return(FALSE);
}

