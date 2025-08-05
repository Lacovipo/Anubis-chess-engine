// MagicBitboards.h
#pragma once

#include "Tipos.h"

// Inicializa los tableros mágicos al arrancar el motor
void InitMagicBitboards(void);

// Genera movimientos deslizantes (alfiles, torres, damas)
// Si bCapturas = TRUE: solo capturas
// Si bCapturas = FALSE: solo no-capturas
int GenerarMovimientosDeslizantes(TPosicion* pPos, TJugada* pJugada, BOOL bCapturas);
UINT64 GetRookAttacks(UINT32 sq, UINT64 occupied);
UINT64 GetBishopAttacks(UINT32 sq, UINT64 occupied);