#include "Preprocesador.h"
#include "Tipos.h"
#include "Variables.h"
#include "Constantes.h"

void InicializarBusqueda(TPosicion * pPos)
{
	UINT32 u32;

	dbDatosBusqueda.bAbortar = FALSE;
	//dbDatosBusqueda.u32NumPlyPartida++;
	dbDatosBusqueda.u32NumPlyPartida = 1 + (pPos - (aPilaPosiciones + 4));
	dbDatosBusqueda.u32ContadorOrdenRoot = CONTADOR_ORDEN_ROOT_INI;

	dbDatosBusqueda.u32NumNodos = 0;
	dbDatosBusqueda.u32NumNodosQ = 0;
	dbDatosBusqueda.u32NumNodosQJ = 0;
	dbDatosBusqueda.u32NumNulos = 0;
	dbDatosBusqueda.u32NumJugHash = 0;
	dbDatosBusqueda.s32TotalMaterialInicial = pPos->u8NumPeonesB + pPos->u8NumPeonesN + pPos->u8NumPiezasB + pPos->u8NumPiezasN; // OJO: ¿el número de piezas y peones?

	if (dbDatosBusqueda.u32NumPlyPartida >= 2)
		dbDatosBusqueda.s32EvalRoot = as32PilaEvaluaciones[dbDatosBusqueda.u32NumPlyPartida - 2];
	else
		dbDatosBusqueda.s32EvalRoot = 0;

	dbDatosBusqueda.jugJugadaRoot = JUGADA_NULA;
	if (dbDatosBusqueda.eTipoBusqueda != TBU_PONDER)
		dbDatosBusqueda.jugJugadaPonder = JUGADA_NULA;

	//
	// Aprovechar killers de la última búsqueda
	//
	for (u32 = 2; u32 < MAX_PLIES; u32++)
	{
		ajugKillers[u32 - 2][0] = ajugKillers[u32][0];
		ajugKillers[u32 - 2][1] = ajugKillers[u32][1];
	}
}

void Inicializar_Historia()
{
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++)
			as32Historia[i][j] = 0;
}
