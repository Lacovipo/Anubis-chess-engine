/*
	Anubis

	José Carlos Martínez Galán
*/

#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"
#include "Bitboards_inline.h"
#include "Inline.h"

/*
	*****************************************************
	*													*
	*	BorrarTablasHash								*
	*													*
	*													*
	*	Deja a cero el contenido de las tablas hash		*
	*													*
	*	Última modificación: 02/07/2003					*
	*													*
	*****************************************************
*/
void BorrarTablasHash(void)
{
	memset(aTablaHash, 0, sizeof(TNodoHash) * (u32TamTablaHash + 1));
	memset(aHashEval, 0, sizeof(TNodoHashEval) * (u32TamHashEval + 1));
	memset(aHashQSCJ, 0, sizeof(TNodoHash) * u32TamHashQSCJ);
}

/*
	*************************************************
	*												*
	*	CalcularHash								*
	*												*
	*												*
	*	Recibe: Un puntero a la posición actual		*
	*												*
	*	Descripción: Calcula las claves hash para	*
	*				 la posición dada, y las		*
	*				 almacena en el registro		*
	*												*
	*************************************************
*/
void CalcularHash(TPosicion * pPos)
{
	UINT64	u64Pieza;
	UINT32	i;

	pPos->u64HashSignature = 0;

	u64Pieza = pPos->u64PeonesB;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64PeonB_Random[i];
	}
	u64Pieza = pPos->u64PeonesN;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64PeonN_Random[i];
	}
	u64Pieza = pPos->u64CaballosB;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64CaballoB_Random[i];
	}
	u64Pieza = pPos->u64CaballosN;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64CaballoN_Random[i];
	}
	u64Pieza = pPos->u64AlfilesB;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64AlfilB_Random[i];
	}
	u64Pieza = pPos->u64AlfilesN;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64AlfilN_Random[i];
	}
	u64Pieza = pPos->u64TorresB;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64TorreB_Random[i];
	}
	u64Pieza = pPos->u64TorresN;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64TorreN_Random[i];
	}
	u64Pieza = pPos->u64DamasB;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64DamaB_Random[i];
	}
	u64Pieza = pPos->u64DamasN;
	while (u64Pieza)
	{
		i = BB_GetBitYQuitar(&u64Pieza);
		pPos->u64HashSignature ^= au64DamaN_Random[i];
	}
	pPos->u64HashSignature ^= au64ReyB_Random[pPos->u8PosReyB];
	pPos->u64HashSignature ^= au64ReyN_Random[pPos->u8PosReyN];

	pPos->u64HashSignature ^= au64AlPaso_Random[pPos->u8AlPaso];
	pPos->u64HashSignature ^= au64Enroques_Random[pPos->u8Enroques];
}

BOOL ConsultarHash(TPosicion* pPos, SINT32 s32Prof, SINT32 s32Alfa, SINT32 s32Beta, TNodoHash ** pNodo)
{
	TNodoHash * pTablaHash = &aTablaHash[pPos->u64HashSignature & u32TamTablaHash];

	for (*pNodo = pTablaHash; *pNodo < pTablaHash + HASH_RANURAS; (*pNodo)++)
	{
		if (((*pNodo)->u64HashSignature == pPos->u64HashSignature) && (Hash_GetTurno(*pNodo) == Pos_GetTurno(pPos)))
		{
			// Tenemos coincidencia de signature y turno, comprobamos la prof para ver si podemos podar directamente
			if (s32Prof <= (SINT32)Hash_GetProf(*pNodo))
			{
				// La profundidad es buena, así que sólo queda comprobar la eval
				if (Hash_GetEsExacto(*pNodo))
					return(TRUE);
				else if (Hash_GetEsUBound(*pNodo))
				{
					if (Jug_GetVal((*pNodo)->jug) <= s32Alfa)
						return(TRUE);
				}
				else
				{
					assert(Hash_GetEsLBound(*pNodo));
					if (Jug_GetVal((*pNodo)->jug) >= s32Beta)
						return(TRUE);
				}
			}

			// Si llegamos hasta aquí es porque no podemos podar
			if (!Hash_GetIntentarNull(*pNodo))
				Pos_SetNoIntentarNull(pPos);
			return(FALSE);
		}
	}

	*pNodo = NULL;
	return(FALSE);
}

/*
 * GrabarHash
 *
 * Recibe: Puntero a la posición actual, profundidad (draft), val, jugada a almacenar, alfa y beta (para saber si es FH of FL)
 *
 */
void GrabarHash(TPosicion * pPos,
                SINT32 s32Prof,
                SINT32 s32Val,
                TJugada jug,
                SINT32 s32Alfa,
                SINT32 s32Beta)
{
	#if defined(HASH_NO_GRABAR_MATES)
		if (Bus_DoyMatePronto(s32Val))
			if (Bus_DoyMatePronto(s32Beta))
				return;
			else
				s32Val = s32Beta;
		if (Bus_ReciboMatePronto(s32Val))
			if (Bus_ReciboMatePronto(s32Alfa))
				return;
			else
				s32Val = s32Alfa;
	#endif

	TNodoHash * pTablaHash = &aTablaHash[pPos->u64HashSignature & u32TamTablaHash];
	TNodoHash * pNodo;
	BOOL		bGrabar = FALSE;

	//
	// Primero, decido en cual de las ranuras voy a grabar
	//
	for (pNodo = pTablaHash; pNodo < pTablaHash + HASH_RANURAS; pNodo++)
	{
		const BOOL bEsMismaPos = pNodo->u64HashSignature == pPos->u64HashSignature && Hash_GetTurno(pNodo) == Pos_GetTurno(pPos);

		if (bEsMismaPos)
			break;
		if (pNodo->u64HashSignature == 0)													// La ranura está vacía
			break;
		if (Hash_GetMaterial(pNodo) > (UINT32)dbDatosBusqueda.s32TotalMaterialInicial)		// El material es obsoleto
			break;
		if (pNodo == pTablaHash + HASH_RANURAS-1)											// No he podido grabar en las primeras
			break;
	}

	//
	// Vamos a ver si machacamos lo que hay o lo respetamos
	//
	if (pNodo->u64HashSignature == 0)		
		bGrabar = TRUE;										// La tabla hash está vacía
	else if (s32Prof > (SINT32)Hash_GetProf(pNodo))		
		bGrabar = TRUE;										// Tengo más profundidad en la pos actual
	else if (s32Prof == (SINT32)Hash_GetProf(pNodo))
	{
															// Tengo igual profundidad
		if (s32Val > s32Alfa && s32Val < s32Beta)
			bGrabar = TRUE;										// Valor exacto en la pos actual
		else if (Hash_GetEsExacto(pNodo))
			bGrabar = FALSE;									// Cota en la prof actual y valor exacto en hash
		else
			bGrabar = TRUE;										// Cota en ambos
	}
	else
		bGrabar = FALSE;									// Tengo más profundidad en hash

	if (bGrabar)
	{
		pNodo->u64HashSignature = pPos->u64HashSignature;

		Hash_SetProf(pNodo, s32Prof);
		Hash_SetMaterial(pNodo, dbDatosBusqueda.s32TotalMaterialInicial);
		Hash_SetTurno(pNodo, Pos_GetTurno(pPos));
		if (s32Val > s32Alfa)
		{
			if (s32Val >= s32Beta)
				Hash_SetLBound(pNodo);		// Cota inferior (LBound, FH)
			else
				Hash_SetExacto(pNodo);		// Valor exacto

			if (Jug_GetEsNula(jug))
				Jug_SetNula(&(pNodo->jug));
			else
				pNodo->jug = jug;
			Jug_SetVal(&(pNodo->jug), s32Val);
		}
		else
		{
			// Cota superior (UBound, FL)
			Hash_SetUBound(pNodo);
			Jug_SetNula(&(pNodo->jug));
			Jug_SetVal(&(pNodo->jug), s32Val);
		}
		assert((UINT32)(pNodo->u8Flags & (UINT8)0x0E) == 2 || (UINT32)(pNodo->u8Flags & (UINT8)0x0E) == 4 || (UINT32)(pNodo->u8Flags & (UINT8)0x0E) == 8); // Tiene bound

		if (Pos_GetIntentarNull(pPos))
			Hash_SetIntentarNull(pNodo);
	}
}

/*
	*************************************************
	*												*
	*	ConsultarHashEval							*
	*												*
	*												*
	*	Recibe: Un puntero a la posición actual		*
	*												*
	*	Devuelve: TRUE si la encuentra, FALSE en	*
	*			  caso contrario					*
	*												*
	*	Descripción: Busca en la tabla hash de		*
	*				 evaluaciones. Si encuentra la	*
	*				 posición actual, copia todos	*
	*				 los valores de evaluación a	*
	*				 la estructura de pPos y		*
	*				 devuelve TRUE.					*
	*												*
	*************************************************
*/
BOOL ConsultarHashEval(TPosicion * pPos)
{
	TNodoHashEval * pNodo = &aHashEval[pPos->u64HashSignature & u32TamHashEval];

	if (pPos->u64HashSignature == pNodo->u64HashSignature && pNodo->u32Turno == Pos_GetTurno(pPos))
	{
		pPos->s32Eval =			pNodo->s32Eval;
		pPos->s32EvalAmenaza =	pNodo->s32EvalAmenaza;

		pPos->u64AtaquesPB = pNodo->u64AtaquesPB;
		pPos->u64AtaquesPN = pNodo->u64AtaquesPN;
		pPos->u64AtaquesCB = pNodo->u64AtaquesCB;
		pPos->u64AtaquesCN = pNodo->u64AtaquesCN;
		pPos->u64AtaquesAB = pNodo->u64AtaquesAB;
		pPos->u64AtaquesAN = pNodo->u64AtaquesAN;
		pPos->u64AtaquesTB = pNodo->u64AtaquesTB;
		pPos->u64AtaquesTN = pNodo->u64AtaquesTN;
		pPos->u64AtaquesDB = pNodo->u64AtaquesDB;
		pPos->u64AtaquesDN = pNodo->u64AtaquesDN;
		pPos->u64AtaquesRB = pNodo->u64AtaquesRB;
		pPos->u64AtaquesRN = pNodo->u64AtaquesRN;
		pPos->u64AtaquesB = pNodo->u64AtaquesB;
		pPos->u64AtaquesN = pNodo->u64AtaquesN;
		pPos->u64Clavadas = pNodo->u64Clavadas;

#if defined(MI_DEBUG)
		assert(pPos->u64AtaquesPB == AtaquesPeonB(pPos->u64PeonesB));
		assert(pPos->u64AtaquesPN == AtaquesPeonN(pPos->u64PeonesN));
		assert(pPos->u64AtaquesCB == AtaquesCaballo(pPos->u64CaballosB));
		assert(pPos->u64AtaquesCN == AtaquesCaballo(pPos->u64CaballosN));
		assert(pPos->u64AtaquesAB == AtaquesAlfil(pPos->u64AlfilesB,~(pPos->u64TodasB | pPos->u64TodasN)));
		assert(pPos->u64AtaquesAN == AtaquesAlfil(pPos->u64AlfilesN,~(pPos->u64TodasB | pPos->u64TodasN)));
		assert(pPos->u64AtaquesTB == AtaquesTorre(pPos->u64TorresB,~(pPos->u64TodasB | pPos->u64TodasN)));
		assert(pPos->u64AtaquesTN == AtaquesTorre(pPos->u64TorresN,~(pPos->u64TodasB | pPos->u64TodasN)));
		assert(pPos->u64AtaquesDB == AtaquesAlfil(pPos->u64DamasB,~(pPos->u64TodasB | pPos->u64TodasN)) | AtaquesTorre(pPos->u64DamasB,~(pPos->u64TodasB | pPos->u64TodasN)));
		assert(pPos->u64AtaquesDN == AtaquesAlfil(pPos->u64DamasN,~(pPos->u64TodasB | pPos->u64TodasN)) | AtaquesTorre(pPos->u64DamasN,~(pPos->u64TodasB | pPos->u64TodasN)));
		assert(pPos->u64AtaquesRB == BB_ATAQUES_REY[pPos->u8PosReyB]);
		assert(pPos->u64AtaquesRN == BB_ATAQUES_REY[pPos->u8PosReyN]);
		assert(pPos->u64AtaquesB == pPos->u64AtaquesPB | pPos->u64AtaquesCB | pPos->u64AtaquesAB | pPos->u64AtaquesTB | pPos->u64AtaquesDB | pPos->u64AtaquesRB);
		assert(pPos->u64AtaquesN == pPos->u64AtaquesPN | pPos->u64AtaquesCN | pPos->u64AtaquesAN | pPos->u64AtaquesTN | pPos->u64AtaquesDN | pPos->u64AtaquesRN);
		{
			UINT64 u64Clavadores;
			UINT64 u64Clavadas = BB_TABLEROVACIO;
			UINT32 u32Desde;

			u64Clavadores = BB_ATAQUES_ALFIL[pPos->u8PosReyB] & (pPos->u64AlfilesN | pPos->u64DamasN);
			u64Clavadores |= (au64AtaquesTorre[pPos->u8PosReyB] & (pPos->u64TorresN | pPos->u64DamasN));
			while (u64Clavadores)
			{
				u32Desde = BB_GetBitYQuitar(&u64Clavadores);
				// Si sólo hay una pieza entre el rey y el "clavador"
				if (CuentaUnos(au64Entre[u32Desde][pPos->u8PosReyB] & (pPos->u64TodasB | pPos->u64TodasN)) == 1)
				{
					// Si esa única pieza es blanca
					if (au64Entre[u32Desde][pPos->u8PosReyB] & pPos->u64TodasB)
						u64Clavadas |= (au64Entre[u32Desde][pPos->u8PosReyB] & pPos->u64TodasB);
				}
			}
			u64Clavadores = BB_ATAQUES_ALFIL[pPos->u8PosReyN] & (pPos->u64AlfilesB | pPos->u64DamasB);
			u64Clavadores |= (au64AtaquesTorre[pPos->u8PosReyN] & (pPos->u64TorresB | pPos->u64DamasB));
			while (u64Clavadores)
			{
				u32Desde = BB_GetBitYQuitar(&u64Clavadores);
				// Si sólo hay una pieza entre el rey y el "clavador"
				if (CuentaUnos(au64Entre[u32Desde][pPos->u8PosReyN] & (pPos->u64TodasB | pPos->u64TodasN)) == 1)
				{
					// Si esa única pieza es negra
					if (au64Entre[u32Desde][pPos->u8PosReyN] & pPos->u64TodasN)
						u64Clavadas |= (au64Entre[u32Desde][pPos->u8PosReyN] & pPos->u64TodasN);
				}
			}
			assert(u64Clavadas == pNodo->u64Clavadas);
		}
#endif
		return(TRUE);
	}
	else // if (pPos->u64HashSignature == pNodo->u64HashSignature)
		return(FALSE);
}

/*
	*********************************************************************************************
	*																							*
	*	GrabarHashEval																			*
	*																							*
	*																							*
	*	Recibe: Un puntero a la posición actual													*
	*																							*
	*	Descripción: Copia los valores de evaluación de la pos actual a	la tabla hash.			*
	*				 Nótese que no hago comprobación de colisión, puesto que, en Evaluar(),		*
	*				 si	encuentra la posición, sale.											*
	*				 Por tanto, nunca entraremos aquí con una posición que ya está en la tabla	*
	*																							*
	*********************************************************************************************
*/
void GrabarHashEval(TPosicion * pPos,SINT32 s32Eval)
{
	TNodoHashEval * pNodo = &aHashEval[pPos->u64HashSignature & u32TamHashEval];

	pNodo->u64HashSignature =	pPos->u64HashSignature;
	pNodo->s32Eval =			s32Eval;
	pNodo->s32EvalAmenaza =		pPos->s32EvalAmenaza;

	pNodo->u64AtaquesPB = pPos->u64AtaquesPB;
	pNodo->u64AtaquesPN = pPos->u64AtaquesPN;
	pNodo->u64AtaquesCB = pPos->u64AtaquesCB;
	pNodo->u64AtaquesCN = pPos->u64AtaquesCN;
	pNodo->u64AtaquesAB = pPos->u64AtaquesAB;
	pNodo->u64AtaquesAN = pPos->u64AtaquesAN;
	pNodo->u64AtaquesTB = pPos->u64AtaquesTB;
	pNodo->u64AtaquesTN = pPos->u64AtaquesTN;
	pNodo->u64AtaquesDB = pPos->u64AtaquesDB;
	pNodo->u64AtaquesDN = pPos->u64AtaquesDN;
	pNodo->u64AtaquesRB = pPos->u64AtaquesRB;
	pNodo->u64AtaquesRN = pPos->u64AtaquesRN;
	pNodo->u64AtaquesB = pPos->u64AtaquesB;
	pNodo->u64AtaquesN = pPos->u64AtaquesN;
	pNodo->u64Clavadas = pPos->u64Clavadas;

	pNodo->u32Turno = Pos_GetTurno(pPos);
}

//
// Comprueba que es la misma signature y turno, y actualiza el valor de eval
//	(viene de cuando mejoramos eval con el valor de la tabla hash general)
//
void GrabarHashEvalSoloEval(TPosicion* pPos, SINT32 s32Eval)
{
	TNodoHashEval* pNodo = &aHashEval[pPos->u64HashSignature & u32TamHashEval];

	if (pNodo->u64HashSignature == pPos->u64HashSignature && pNodo->u32Turno == Pos_GetTurno(pPos))
		pNodo->s32Eval = s32Eval;	
}
