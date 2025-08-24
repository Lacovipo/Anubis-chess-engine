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


#if defined(SOFT_INTRINSICS)
#ifndef _pext_u64
#include <stdint.h>

inline uint64_t my_blsr_u64(uint64_t x) {
    return x & (x - 1);
}



inline uint64_t generic_pext_u64(uint64_t src, uint64_t mask) {
    uint64_t res = 0;
    int bit_index = 0;
    for (int i = 0; i < 64; ++i) {
        if ((mask >> i) & 1) {
            if ((src >> i) & 1) {
                res |= (1ULL << bit_index);
            }
            bit_index++;
        }
    }
    return res;
}

#define _pext_u64(src, mask) generic_pext_u64(src, mask)

#endif
#endif