
#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"
#include <math.h>

/*
 *
 * CalcularNumElemsPot2
 *
 *
 * Recibe: La cantidad de megas disponibles para ser asignados
 *
 * Devuelve: Los elementos que se pueden reservar
 *
 * Descripción: Calcula el espacio de memoria que se puede reservar respetando la estructura deseada de potencias de dos, para poder hacer uso de máscaras directas
 *
 * Seguiremos el siguiente esquema para repartir memoria:
 *	s: sizeof(TipoHash)
 *	M: Tamaño deseado en megas
 *	x: Número de bits usados (ha de ser un número entero)
 *	(2^x)-1: Número de registros
 *
 *	2^x <= (T * (1024^2) / s) + 1
 *
 */
static UINT32 CalcularNumElemsPot2(double dMegasDisponibles, UINT32 u32Size)
{
	SINT32 x;
	double M = dMegasDisponibles;

	for (x = 1; ; x++)
	{
		if (pow(2, x) > ((M * 1024 * 1024 / u32Size) + 1))
			break;
	}
	x--;

	return((UINT32)abs((SINT32)pow(2.0, (double)x)-1));
}

/*
 *
 * DimensionarTablasHash
 *
 *
 * Total de megas que queremos emplear en hashing: PER_MEMORIAHASH
 * Lo repartiremos entre las siguientes tablas:
 *	- Trans/Ref, por profundidad / Trans/Ref, siempre reemplazo
 *	- QSearchConJaques
 *	- Evaluación completa
 *
 */
BOOL DimensionarTablasHash(void)
{
	UINT32			u32Libre		= PER_MEMORIAHASH;
	const UINT32	u32DivisorMegas	= 1024 * 1024;

	// Calcular tamaños
	u32TamTablaHash = CalcularNumElemsPot2(u32Libre, sizeof(TNodoHash));
	u32Libre -= u32TamTablaHash * sizeof(TNodoHash) / u32DivisorMegas;

	u32TamHashEval = CalcularNumElemsPot2(u32Libre, sizeof(TNodoHashEval));

	// Reservar memoria
	aTablaHash = (TNodoHash *)calloc(u32TamTablaHash + 1, sizeof(TNodoHash));
	if (aTablaHash == NULL)
		return(FALSE);
	aHashEval = (TNodoHashEval *)calloc(u32TamHashEval + 1, sizeof(TNodoHashEval));
	if (aHashEval == NULL)
		return(FALSE);

	return(TRUE);
}