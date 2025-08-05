// MagicBitboards.c
#include "GPT_Magic.h"
#include <immintrin.h>  // Para pext y tzcnt
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "Tipos.h"
#include "Constantes.h"
#include <intrin.h>
#include "Bitboards_inline.h"
#include "Inline.h"

// Declaraciones de funciones internas
static UINT64 RookMask(int sq);
static UINT64 BishopMask(int sq);
static UINT64 RookAttacksOnTheFly(int sq, UINT64 block);
static UINT64 BishopAttacksOnTheFly(int sq, UINT64 block);
static UINT64 GenerateBlockers(int index, UINT64 mask);

// Tablas globales
UINT64 au64MagicRook[64];
UINT64 au64MagicBishop[64];
UINT64 au64MaskRook[64];
UINT64 au64MaskBishop[64];
UINT64 au64RookMoves[64][4096];
UINT64 au64BishopMoves[64][512];

UINT64 GetRookAttacks(UINT32 sq, UINT64 occupied)
{
    UINT64 masked = occupied & au64MaskRook[sq];
    UINT64 key = _pext_u64(masked, au64MaskRook[sq]);
    return au64RookMoves[sq][key];
}

UINT64 GetBishopAttacks(UINT32 sq, UINT64 occupied)
{
    UINT64 masked = occupied & au64MaskBishop[sq];
    UINT64 key = _pext_u64(masked, au64MaskBishop[sq]);
    return au64BishopMoves[sq][key];
}

int GenerarMovimientosDeslizantes(TPosicion* pPos, TJugada* pJugada, BOOL bCapturas)
{
    UINT64 u64Todas = pPos->u64TodasB | pPos->u64TodasN;
    UINT64 u64Piezas, u64Destinos;
    UINT32 u32Desde, u32Hasta;
    int iCuenta = 0;

    if (pPos->u8Turno == BLANCAS)
    {
        u64Piezas = pPos->u64TorresB | pPos->u64DamasB;
        while (u64Piezas)
        {
            UINT32 bitIdx = _tzcnt_u64(u64Piezas);
            u32Desde = 63 - bitIdx;
            assert(PiezaEnCasilla(pPos, u32Desde) == TB || PiezaEnCasilla(pPos, u32Desde) == DB);
            u64Piezas = _blsr_u64(u64Piezas);
            u64Destinos = GetRookAttacks(bitIdx, u64Todas) & ~pPos->u64TodasB;
            if (bCapturas)
                u64Destinos &= pPos->u64TodasN;
            else
                u64Destinos &= ~pPos->u64TodasN;
            while (u64Destinos)
            {
                u32Hasta = 63 - _tzcnt_u64(u64Destinos);
                u64Destinos = _blsr_u64(u64Destinos);
                Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
                iCuenta++;
            }
        }
        u64Piezas = pPos->u64AlfilesB | pPos->u64DamasB;
        while (u64Piezas)
        {
            UINT32 bitIdx = _tzcnt_u64(u64Piezas);
            u32Desde = 63 - bitIdx;
            assert(PiezaEnCasilla(pPos, u32Desde) == AB || PiezaEnCasilla(pPos, u32Desde) == DB);
            u64Piezas = _blsr_u64(u64Piezas);
            u64Destinos = GetBishopAttacks(bitIdx, u64Todas) & ~pPos->u64TodasB;
            if (bCapturas)
                u64Destinos &= pPos->u64TodasN;
            else
                u64Destinos &= ~pPos->u64TodasN;
            while (u64Destinos)
            {
                u32Hasta = 63 - _tzcnt_u64(u64Destinos);
                u64Destinos = _blsr_u64(u64Destinos);
                Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
                iCuenta++;
            }
        }
    }
    else
    {
        u64Piezas = pPos->u64TorresN | pPos->u64DamasN;
        while (u64Piezas)
        {
            UINT32 bitIdx = _tzcnt_u64(u64Piezas);
            u32Desde = 63 - bitIdx;
            assert(PiezaEnCasilla(pPos, u32Desde) == TN || PiezaEnCasilla(pPos, u32Desde) == DN);
            u64Piezas = _blsr_u64(u64Piezas);
            u64Destinos = GetRookAttacks(bitIdx, u64Todas) & ~pPos->u64TodasN;
            if (bCapturas)
                u64Destinos &= pPos->u64TodasB;
            else
                u64Destinos &= ~pPos->u64TodasB;
            while (u64Destinos)
            {
                u32Hasta = 63 - _tzcnt_u64(u64Destinos);
                u64Destinos = _blsr_u64(u64Destinos);
                Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
                iCuenta++;
            }
        }
        u64Piezas = pPos->u64AlfilesN | pPos->u64DamasN;
        while (u64Piezas)
        {
            UINT32 bitIdx = _tzcnt_u64(u64Piezas);
            u32Desde = 63 - bitIdx;
            assert(PiezaEnCasilla(pPos, u32Desde) == AN || PiezaEnCasilla(pPos, u32Desde) == DN);
            u64Piezas = _blsr_u64(u64Piezas);
            u64Destinos = GetBishopAttacks(bitIdx, u64Todas) & ~pPos->u64TodasN;
            if (bCapturas)
                u64Destinos &= pPos->u64TodasB;
            else
                u64Destinos &= ~pPos->u64TodasB;
            while (u64Destinos)
            {
                u32Hasta = 63 - _tzcnt_u64(u64Destinos);
                u64Destinos = _blsr_u64(u64Destinos);
                Jug_SetMovConHistoria(pJugada++, u32Desde, u32Hasta, 0);
                iCuenta++;
            }
        }
    }

    return(iCuenta);
}

void InitMagicBitboards(void)
{
    for (int sq = 0; sq < 64; ++sq)
    {
        au64MaskRook[sq] = RookMask(sq);
        au64MaskBishop[sq] = BishopMask(sq);

        int rookBits = __popcnt64(au64MaskRook[sq]);
        int bishopBits = __popcnt64(au64MaskBishop[sq]);

        for (int i = 0; i < (1 << rookBits); ++i)
        {
            UINT64 blockers = GenerateBlockers(i, au64MaskRook[sq]);
            UINT64 key = _pext_u64(blockers, au64MaskRook[sq]);
            au64RookMoves[sq][key] = RookAttacksOnTheFly(sq, blockers);
        }
        for (int i = 0; i < (1 << bishopBits); ++i)
        {
            UINT64 blockers = GenerateBlockers(i, au64MaskBishop[sq]);
            UINT64 key = _pext_u64(blockers, au64MaskBishop[sq]);
            au64BishopMoves[sq][key] = BishopAttacksOnTheFly(sq, blockers);
        }
    }
}

// Devuelve máscara de casillas para movimientos tipo torre desde una casilla
static UINT64 RookMask(int sq)
{
    int rank = sq / 8;
    int file = sq % 8;
    UINT64 mask = 0;

    for (int r = rank + 1; r <= 6; r++) mask |= 1ULL << (r * 8 + file);
    for (int r = rank - 1; r >= 1; r--) mask |= 1ULL << (r * 8 + file);
    for (int f = file + 1; f <= 6; f++) mask |= 1ULL << (rank * 8 + f);
    for (int f = file - 1; f >= 1; f--) mask |= 1ULL << (rank * 8 + f);

    return mask;
}

// Devuelve máscara de casillas para movimientos tipo alfil desde una casilla
static UINT64 BishopMask(int sq)
{
    int rank = sq / 8;
    int file = sq % 8;
    UINT64 mask = 0;

    for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++) mask |= 1ULL << (r * 8 + f);
    for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--) mask |= 1ULL << (r * 8 + f);
    for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++) mask |= 1ULL << (r * 8 + f);
    for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--) mask |= 1ULL << (r * 8 + f);

    return mask;
}

// Genera movimientos de torre teniendo en cuenta bloqueos
static UINT64 RookAttacksOnTheFly(int sq, UINT64 block)
{
    UINT64 attacks = 0;
    int rk = sq / 8;
    int fl = sq % 8;

    for (int r = rk + 1; r <= 7; r++)
    {
        int to = r * 8 + fl;
        attacks |= 1ULL << to;
        if (block & (1ULL << to))
            break;
    }
    for (int r = rk - 1; r >= 0; r--)
    {
        int to = r * 8 + fl;
        attacks |= 1ULL << to;
        if (block & (1ULL << to))
            break;
    }
    for (int f = fl + 1; f <= 7; f++)
    {
        int to = rk * 8 + f;
        attacks |= 1ULL << to;
        if (block & (1ULL << to))
            break;
    }
    for (int f = fl - 1; f >= 0; f--)
    {
        int to = rk * 8 + f;
        attacks |= 1ULL << to;
        if (block & (1ULL << to))
            break;
    }

    return attacks;
}

// Genera movimientos de alfil teniendo en cuenta bloqueos
static UINT64 BishopAttacksOnTheFly(int sq, UINT64 block)
{
    UINT64 attacks = 0;
    int rk = sq / 8;
    int fl = sq % 8;

    for (int r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++)
    {
        int to = r * 8 + f;
        attacks |= 1ULL << to;
        if (block & (1ULL << to))
            break;
    }
    for (int r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--)
    {
        int to = r * 8 + f;
        attacks |= 1ULL << to;
        if (block & (1ULL << to))
            break;
    }
    for (int r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++)
    {
        int to = r * 8 + f;
        attacks |= 1ULL << to;
        if (block & (1ULL << to))
            break;
    }
    for (int r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--)
    {
        int to = r * 8 + f;
        attacks |= 1ULL << to;
        if (block & (1ULL << to))
            break;
    }

    return attacks;
}

// Dado un índice y una máscara, devuelve una combinación de bloqueadores
static UINT64 GenerateBlockers(int index, UINT64 mask)
{
    UINT64 result = 0;
    int bits = __popcnt64(mask);
    int i = 0;

    for (int b = 0; b < 64 && i < bits; ++b)
    {
        if (mask & (1ULL << b))
        {
            if (index & (1 << i)) result |= (1ULL << b);
            ++i;
        }
    }
    return result;
}
