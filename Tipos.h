
#pragma once

#include "Preprocesador.h"
#include <sys/timeb.h>

// Compatibilidad
#define UINT8	unsigned __int8
#define UINT16	unsigned __int16
#define UINT32	unsigned __int32
#define UINT64	unsigned __int64
#define SINT8	signed __int8
#define SINT16	signed __int16
#define SINT32	signed __int32
#define SINT64	signed __int64
#define INLINE	__forceinline
#define BOOL	unsigned __int32

// Jugada
typedef struct _TJugada
{
	SINT32	s32Val;
	UINT32	u32Mov;				// PPPP DDDD DDHH HHHH
									// P = Promo (pieza a la que se promociona)
									// D = Desde
									// H = Hasta
} TJugada;

// Posición
__declspec(align(64)) typedef struct _TPosicion
{
	UINT64	u64PeonesB;				// Bitboards (piezas blancas)
	UINT64	u64CaballosB;
	UINT64	u64AlfilesB;
	UINT64	u64TorresB;
	UINT64	u64DamasB;
	UINT64	u64PeonesN;				// Bitboards (piezas negras)
	UINT64	u64CaballosN;
	UINT64	u64AlfilesN;
	UINT64	u64TorresN;
	UINT64	u64DamasN;
	UINT64	u64TodasB;				// Bitboards (mezcla de piezas)
	UINT64	u64TodasN;

	UINT64	u64HashSignature;		// Clave hash con los 64 dígitos
									// La sign. de hash eval es ésta misma

	struct _TPosicion	* pPunteroRepeticion;
	TJugada * pListaJug;			// Puntero al inicio (fin) de la lista de jugadas de la posición actual
	UINT8	u8Cincuenta;			// Son plies, así que son 100 en realidad
	UINT8	u8Enroques;				// 0000 LN(8),CN(4),LB(2),CB(1)
	UINT8	u8AlPaso;				// Casilla al paso
	UINT8	u8IntentarNull;
	UINT8	u8Turno;
	UINT8	u8Jaqueado;
	UINT8	u8PosReyB;				// Posición del rey blanco
	UINT8	u8PosReyN;				// Posición del rey negro

	UINT8	u8NumPiezasB;			// Número de piezas blancas							** Se actualiza en Mover()
	UINT8	u8NumPiezasN;			// Número de piezas negras
	UINT8	u8NumPeonesB;			// Número de peones blancos
	UINT8	u8NumPeonesN;			// Número de peones negros

	// Evaluación
	SINT32 s32Eval;					// Desde el punto de vista del que tiene el turno
	SINT32 s32EvalMaterial;			// Desde el punto de vista de las blancas			** Se actualiza en Mover()
	SINT32 s32EvalAmenaza;
	UINT32 u32Fase;

	// Ataques
	UINT64 u64AtaquesPB;
	UINT64 u64AtaquesPN;
	UINT64 u64AtaquesCB;
	UINT64 u64AtaquesCN;
	UINT64 u64AtaquesAB;
	UINT64 u64AtaquesAN;
	UINT64 u64AtaquesTB;
	UINT64 u64AtaquesTN;
	UINT64 u64AtaquesDB;
	UINT64 u64AtaquesDN;
	UINT64 u64AtaquesRB;
	UINT64 u64AtaquesRN;
	UINT64 u64AtaquesB;
	UINT64 u64AtaquesN;
	UINT64 u64Clavadas;

	// Jugada excluida -- para extensión singular
	UINT32 u32MovExclu;

	// Último movimiento realizado (para refutación)
	UINT8 UltMovPieza;
	UINT8 UltMovHasta;
} TPosicion;

// Hashing
typedef struct _TNodoHash
{
	UINT64	u64HashSignature;
	TJugada	jug;
	UINT16	u16Material;			// Total Material, para el criterio de reemplazo
	UINT8	u8Prof;
	UINT8	u8Flags;				// ???? ???x Turno
									// ???? ??1? Exacto
									// ???? ?1?? UBound
									// ???? 1??? LBound
									// ???x ???? IntentarNull
} TNodoHash;

// Hash eval
typedef struct _TNodoHashEval
{
	UINT64 u64HashSignature;
	SINT32 s32Eval;
	SINT32 s32EvalReyB;
	SINT32 s32EvalReyN;
	SINT32 s32EvalAmenaza;

	// Información general de ataques
	UINT64 u64AtaquesPB;
	UINT64 u64AtaquesPN;
	UINT64 u64AtaquesCB;
	UINT64 u64AtaquesCN;
	UINT64 u64AtaquesAB;
	UINT64 u64AtaquesAN;
	UINT64 u64AtaquesTB;
	UINT64 u64AtaquesTN;
	UINT64 u64AtaquesDB;
	UINT64 u64AtaquesDN;
	UINT64 u64AtaquesRB;
	UINT64 u64AtaquesRN;
	UINT64 u64AtaquesB;
	UINT64 u64AtaquesN;
	UINT64 u64Clavadas;

	// Incertidumbre
	UINT32 u32Turno;
} TNodoHashEval;

// Estados búsqueda
typedef enum
{
	BUS_ESPERANDO,	// Motor parado
	BUS_NORMAL		// Valor exacto
} enmEstadosBusqueda;

// Tipos de búsqueda
typedef enum
{
	TBU_PARTIDA,
	TBU_PONDER,
	TBU_ANALISIS,
	TBU_FORCE
} enmTiposBusqueda;

// Comunicación con el thread del motor
typedef struct _TComandoMotor
{
	UINT32		u32Codigo;
	TPosicion *	u32Param1;
	UINT32		u32Param2;
	char		szParam[1024];
} TComandoMotor;

// Datos generales de la búsqueda
typedef struct _TDatosBusqueda
{
	// Partida
	UINT32				u32ColorAnubis;
	UINT32				u32NumPlyPartida;
	UINT32				u32ContadorOrdenRoot;

	// Contadores
	UINT32				u32NumNodos;
	UINT32				u32NumNodosQ;
	UINT32				u32NumNodosQJ;
	UINT32				u32NumNulos;
	UINT32				u32NumJugHash;

	// Búsqueda
	BOOL				bAbortar;
	SINT32				s32TotalMaterialInicial;
	enmEstadosBusqueda	eEstadoBusqueda;
	enmTiposBusqueda	eTipoBusqueda;
	SINT32				s32EvalRoot;
	TJugada				jugJugadaRoot;
	TJugada				jugJugadaPonder;
	TJugada				ajugPV[150][150];		// MAX_PLIES (si incluyo Constantes.h se lía)
	SINT32				s32ProfRoot;
	TJugada				jugObvia;
	BOOL				bOtroCicloPonder;
	BOOL				bPonderar;
	BOOL				bAnalizandoPV;
	BOOL				bFailLowEnPV;
	SINT32				s32ProfSel;				// 30/01/25 0.60e Profundidad selectiva
	SINT32				s32CambiosJugMejor;		// 08/02/25 0.63 Para alargar el tiempo si cambiamos mucho de jugada principal
} TDatosBusqueda;

/*
 * RELOJ
 */
typedef struct _TReloj
{
	SINT32 m_s32NumJugadasControl;
	SINT32 m_s32NumSegundosControl;
	SINT32 m_s32NumSegundosIncremento;
	SINT32 m_s32MaxProf;					// Máxima profundidad permitida (dada por el comando "sd" de WBII). Lo pongo aquí porque está directamente relacionado con el control de tiempo
	SINT32 m_s32MilisegundosDisponibles;
	SINT64 m_s64MomentoInicioBusqueda;		// El momento en que se inicia la búsqueda
	SINT64 m_s64TiempoCorteObvia;			// El momento de cortar si estamos ante una jugada obvia
	SINT64 m_s64TiempoCorteNormal;			// El momento en el que hemos de cortar, si todo va normal
	SINT64 m_s64TiempoCorteExtendido;		// El momento en que hemos de cortar pase lo que pase
} TReloj;
INLINE SINT64 GetLecturaReloj(void)								{struct _timeb t; _ftime_s(&t); return(t.time * 1000 + t.millitm);}
INLINE void PonerRelojEnMarcha(TReloj * ptReloj)				{ptReloj->m_s64MomentoInicioBusqueda = GetLecturaReloj();}
#pragma warning(disable : 4244)
INLINE double GetTiempoEsperadoNormal(TReloj * ptReloj)			{return((double)(ptReloj->m_s64TiempoCorteNormal - ptReloj->m_s64MomentoInicioBusqueda) / 1000.0);}
INLINE double GetTiempoEsperadoExtendido(TReloj * ptReloj)		{return((double)(ptReloj->m_s64TiempoCorteExtendido - ptReloj->m_s64MomentoInicioBusqueda) / 1000.0);}
INLINE double GetTiempoDisponible(TReloj * ptReloj)				{return((double)(ptReloj->m_s64MomentoInicioBusqueda + ptReloj->m_s32MilisegundosDisponibles - GetLecturaReloj()) / 1000.0);}
INLINE void DetenerReloj(TReloj * ptReloj)
{
	SINT64 s64MilisegundosTranscurridos;
	s64MilisegundosTranscurridos = GetLecturaReloj() - ptReloj->m_s64MomentoInicioBusqueda;
	ptReloj->m_s32MilisegundosDisponibles -= s64MilisegundosTranscurridos;
}
INLINE double GetSegundosTranscurridos(TReloj * ptReloj)		{return((double)(GetLecturaReloj() - ptReloj->m_s64MomentoInicioBusqueda) / 1000);}
#pragma warning(default : 4244)
INLINE SINT64 GetCentisegundosTranscurridos(TReloj * ptReloj)	{return((GetLecturaReloj() - ptReloj->m_s64MomentoInicioBusqueda) / 10);}
INLINE BOOL GetSuperadoCorteObvia(TReloj * ptReloj)				{return(GetLecturaReloj() >= ptReloj->m_s64TiempoCorteObvia);}
INLINE BOOL GetSuperadoCorteMedNormal(TReloj * ptReloj)			{return(GetLecturaReloj() - ptReloj->m_s64MomentoInicioBusqueda >= (ptReloj->m_s64TiempoCorteNormal - ptReloj->m_s64MomentoInicioBusqueda) / 2);}
INLINE BOOL GetSuperadoCorteDosTerNormal(TReloj * ptReloj)		{return(GetLecturaReloj() - ptReloj->m_s64MomentoInicioBusqueda >= 2 * (ptReloj->m_s64TiempoCorteNormal - ptReloj->m_s64MomentoInicioBusqueda) / 3);}
INLINE BOOL GetSuperadoCorteNormal(TReloj * ptReloj)			{return(GetLecturaReloj() >= ptReloj->m_s64TiempoCorteNormal);}
INLINE BOOL GetSuperadoCorteExtendido(TReloj * ptReloj)			{return(GetLecturaReloj() >= ptReloj->m_s64TiempoCorteExtendido);}
INLINE BOOL GetEsHoraDeParar(TReloj * ptReloj, TDatosBusqueda * ptDatosBusqueda, SINT32 aPilaEval[])
{
	if (ptDatosBusqueda->eTipoBusqueda != TBU_PARTIDA)
		return(0);
	if (GetSuperadoCorteExtendido(ptReloj))
		return(1);
	if (GetSuperadoCorteNormal(ptReloj))
	{
		if (ptDatosBusqueda->bFailLowEnPV)
			return(0);
		if (ptDatosBusqueda->u32NumPlyPartida > 1)
			if (ptDatosBusqueda->s32EvalRoot <= aPilaEval[ptDatosBusqueda->u32NumPlyPartida - 2] - 30)
				return(0);
		if (ptDatosBusqueda->s32CambiosJugMejor > 0)		// 08/02/25 0.63 Tengo inestabilidad en la jugada principal
			return(0);
		if (!ptDatosBusqueda->bAnalizandoPV)
			return(0);

		return(1);
	}

	// La jugada obvia la comprobamos en Pensar(), tras completar cada iteración
	return(0);
}

// Datos de poco uso (que no me fastidien la caché)
typedef struct _TDatosPocoUso
{
	char	szNombreOponente[256];
	BOOL	bEnICS;
	BOOL	bComputerOponente;
	UINT32	u32RatingAnubis;
	UINT32	u32RatingOponente;
} TDatosPocoUso;
