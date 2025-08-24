
#pragma once

#include "Preprocesador.h"
#include "Tipos.h"

// Ataques
UINT32	Atacado(TPosicion * pPos, UINT32 u32Casilla, UINT32 u32Color);
UINT64	AtaquesHacia(TPosicion * pPos, UINT32 u32Casilla);
SINT32	SEE(TPosicion * pPos, TJugada jug);
SINT32	SEE_Amenaza(TPosicion * pPos, UINT32 u32Hasta, UINT32 u32PiezaQuePongo);
void	AsignarValorSEE(TPosicion * pPos, TJugada * pJug1, TJugada * pJug2);
UINT64	AtaquesB_ZonaN(TPosicion * pPos, UINT64 u64Zona);
UINT64	AtaquesN_ZonaB(TPosicion * pPos, UINT64 u64Zona);
UINT64	AtaquesB_Hacia(TPosicion * pPos, UINT32 u32Casilla);
UINT64	AtaquesN_Hacia(TPosicion * pPos, UINT32 u32Casilla);
UINT64	AtaquesDeslizB_Hacia(TPosicion * pPos, UINT32 u32Casilla);
UINT64	AtaquesDeslizN_Hacia(TPosicion * pPos, UINT32 u32Casilla);
UINT64	MovimientosB_Hacia(TPosicion * pPos, UINT32 u32Casilla);
UINT64	MovimientosN_Hacia(TPosicion * pPos, UINT32 u32Casilla);

// Bitboard_Inicializar
void InicializarDatosPrecomputados(void);

// Generación de movimientos
TJugada * GenerarCapturas(TPosicion * pPos, TJugada * pJugada, BOOL bSubPromociones);
TJugada * GenerarMovimientos(TPosicion * pPos, TJugada * pJugada);
TJugada * GenerarJaquesNoCapturas(TPosicion * pPos, TJugada * pJugada);

// Hashing
void InicializarRandomHash(void);
BOOL DimensionarTablasHash(void);
void BorrarTablasHash(void);
void CalcularHash(TPosicion * pPos);
BOOL ConsultarHash(TPosicion * pPos, SINT32 s32Prof, SINT32 s32Alfa, SINT32 s32Beta, TNodoHash ** pNodo);
void GrabarHash(TPosicion * pPos, SINT32 s32Prof, SINT32 s32Val, TJugada jug, SINT32 s32Alfa, SINT32 s32Beta);
BOOL ConsultarHashEval(TPosicion * pPos);
void GrabarHashEval(TPosicion * pPos, SINT32 s32Eval);
void GrabarHashEvalSoloEval(TPosicion* pPos, SINT32 s32Eval);
#if defined(QSJ_HASH)
BOOL ConsultarHashQSCJ(TPosicion * pPos, SINT32 s32Prof, SINT32 s32Alfa, SINT32 s32Beta, SINT32 * ps32ValPoda, TJugada * pJug);
void GrabarHashQSCJ(TPosicion * pPos, SINT32 s32Prof, SINT32 s32Val, TJugada jug, SINT32 s32Alfa, SINT32 s32Beta);
#endif
BOOL IntentarFailLow(UINT64 u64HashSignature,UINT8 u8Turno,SINT32 s32Prof,SINT32 s32Alfa);

// General
void Salir(void);
void ComputarBB(TPosicion * pPos);

void AbrirFicheroLog(void);

void InicioWB(int argc,char * argv[]);

// Tablero
BOOL SetBoard(TPosicion * pPos, char * szFEN);

// Notación
TJugada	Algebraica2Jug(char * szJugada);
void Jug2Algebraica(TJugada jug, char * szJugada);
void Jug2SAN(TPosicion* pPos, TJugada jug, char* szJugada);
void ExtraerPVdeArray(TPosicion * pPos, char * szPV);

// Mover
BOOL Mover(TPosicion * pPos, TJugada * pJug);
void MoverNull(TPosicion * pPos);
#if defined(BUS_ETC)
	UINT64 CalcularNuevoHash(TPosicion * pPos, TJugada * pJug);
#endif

// Búsqueda
void	Pensar(TPosicion * pPos);
void	DeterminarTiempoBusqueda(void);
void	InicializarBusqueda(TPosicion * pPos);
SINT32	QSearch(TPosicion * pPos, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Ply);
SINT32	QSConJaques(TPosicion * pPos, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Ply, SINT32 s32Prof);
void	Inicializar_Historia();

void Aspiracion(TPosicion* pPos, TJugada* pJugPV, SINT32 s32Prof);
SINT32 Raiz(TPosicion* pPos, TJugada* pJugPV, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Prof);

SINT32	AlfaBeta(TPosicion * pPos, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Prof, SINT32 s32Ply, BOOL bEsCutNode);
SINT32	AlfaBetaPV(TPosicion * pPos, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Prof, SINT32 s32Ply);
BOOL	JugadaCorrecta(TPosicion * pPos, TJugada jug);
void	Ordenar(TPosicion * pPos, TJugada * pJugada);
SINT32	Extensiones(TPosicion * pPos, TJugada jug, SINT32 s32Prof, SINT32 s32Ply);
SINT32	Reducciones(TPosicion * pPos, TJugada jug, SINT32 s32Prof, SINT32 s32Alfa, UINT32 u32Legales, BOOL bEsCutNode, BOOL	bMejorando);
BOOL	SegundaRepeticion(TPosicion * pPos);
BOOL	PodarContraBeta(TPosicion * pPos, SINT32 s32Ply, SINT32 s32Prof, SINT32 s32Beta, BOOL bMejorando);
#if defined(PODA_FUTIL)
BOOL	PodarFutility(TPosicion * pPos, SINT32 s32Prof, SINT32 s32Alfa);
#endif
#if defined(PODA_LMP)
BOOL PodarLMP(SINT32 s32Prof, UINT32 u32Legales, TPosicion * pPos);
#endif
BOOL PodarRazoring(TPosicion * pPos, SINT32 s32Alfa, SINT32 s32Beta, SINT32 s32Prof, SINT32 s32Ply);

// Eval
SINT32 Evaluar(TPosicion * pPos);
SINT32 EvaluarMaterial(TPosicion * pPos);
BOOL InsuficienteMaterial(TPosicion * pPos);
SINT32 DeterminarAmenaza(TPosicion* pPos);

// Thread del motor
#ifdef _MSC_VER
UINT32 __stdcall MotorThread(void);
#else
void* MotorThread(void* arg);
#endif
BOOL ProcesarComando(void);
void EnviarComandoAMotor(UINT32 u32Codigo, TPosicion * u32Param1, UINT32 u32Param2, char * szParam);

// Pantalla
void __cdecl	ImprimirAPantalla(const char * szFmt, ...);
#if (FICHERO_LOG	== AV_LOG)
	void __cdecl	ImprimirALog(const char * szFmt, ...);
#endif
void ImprimirCadenaGUI(SINT32 s32Prof,SINT32 s32Eval,char * szPV);
