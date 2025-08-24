
#pragma once

#include "Preprocesador.h"
#include "Tipos.h"
#include <assert.h>
#include "Variables.h"

#if defined(_MSC_VER)
	#if defined(__AVX2__)
		#include <intrin.h>
	#endif
#endif

#if defined(__AVX2__) 
	#include <immintrin.h>  // Para pext y tzcnt
#endif

static INLINE UINT64 BB_Mask(UINT32 u32)
{
	assert(u32 < 64);
	assert(au64Mask[u32] == 0x8000000000000000 >> u32);
	return(0x8000000000000000 >> u32);
}

static INLINE UINT64 BB_NoMask(UINT32 u32)
{
	return(~BB_Mask(u32));
}

/*
 * Primer uno
 */
static INLINE UINT32 PrimerUnoD(UINT64 bb)
{
#ifdef _MSC_VER
	return bb ? 63 - (UINT32)_tzcnt_u64(bb) : 64;
#else
	return bb ? 63 - (UINT32)__builtin_ctzll(bb) : 64;
#endif
}

static INLINE UINT32 PrimerUnoI(UINT64 bb)
{
#ifdef _MSC_VER
	return bb ? (UINT32)_lzcnt_u64(bb) : 64;
#else
	return bb ? (UINT32)__builtin_clzll(bb) : 64;
#endif
}

static INLINE UINT32 CuentaUnos(UINT64 bb)
{
#ifdef _MSC_VER
	return((UINT32)__popcnt64(bb));
#else
	return((UINT32)__builtin_popcountll(bb));
#endif
}

static INLINE UINT32 PrimerUno(UINT64 bb)
{
	return(PrimerUnoD(bb));
}

/*
 *
 * PiezaEnCasilla
 *
 *
 * Recibe: Puntero a la posición actual y casilla que queremos comprobar
 *
 * Descripción: Comprueba los bitboards de piezas para determinar qué hay en la casilla especificada
 *
 */
static INLINE UINT32 PiezaEnCasilla(TPosicion* pPos, UINT32 u32Casilla)
{
	register UINT64 u64Casilla = BB_Mask(u32Casilla);

	if (u64Casilla & pPos->u64TodasB)
	{
		if (pPos->u64PeonesB & u64Casilla)
			return(PB);
		if (pPos->u64CaballosB & u64Casilla)
			return(CB);
		if (pPos->u64AlfilesB & u64Casilla)
			return(AB);
		if (pPos->u64TorresB & u64Casilla)
			return(TB);
		if (pPos->u64DamasB & u64Casilla)
			return(DB);
		return(RB);
	}
	else if (u64Casilla & pPos->u64TodasN)
	{
		if (pPos->u64PeonesN & u64Casilla)
			return(PN);
		if (pPos->u64CaballosN & u64Casilla)
			return(CN);
		if (pPos->u64AlfilesN & u64Casilla)
			return(AN);
		if (pPos->u64TorresN & u64Casilla)
			return(TN);
		if (pPos->u64DamasN & u64Casilla)
			return(DN);
		return(RN);
	}
	else
		return(VACIO);
}

static INLINE void BB_InvertirBit(UINT64 * pu64BB, UINT32 u32Bit)
{
	assert(u32Bit < 64);
	*pu64BB ^= BB_Mask(u32Bit);
}
static INLINE void BB_SetBit(UINT64 * pu64BB, UINT32 u32Bit)
{
	assert(u32Bit < 64);
	*pu64BB |= BB_Mask(u32Bit);
}
static INLINE void BB_ClearBit(UINT64 * pu64BB, UINT32 u32Bit)
{
	assert(u32Bit < 64);
	*pu64BB &= BB_NoMask(u32Bit);
}
static inline BOOL MasDeUnUno(UINT64 u64)
{
	return (u64 & (u64 - 1)) != 0;
}

// Rotaciones y desplazamientos
static INLINE UINT64 BB_GetShiftIzda(UINT64 u64BB)
{
	assert(!(((u64BB & BB_SINCOLIZDA) << 1) & BB_COLH1H8));
	return((u64BB & BB_SINCOLIZDA) << 1);
}
static INLINE UINT64 BB_GetShiftDcha(UINT64 u64BB)
{
	assert(!(((u64BB & BB_SINCOLDCHA) >> 1) & BB_COLA1A8));
	return((u64BB & BB_SINCOLDCHA) >> 1);
}
static INLINE UINT64 BB_GetShiftArriba(UINT64 u64BB)
{
	assert(!((u64BB << 8) & BB_FILA1H1));
	return(u64BB << 8);
}
static INLINE UINT64 BB_GetShiftAbajo(UINT64 u64BB)
{
	assert(!((u64BB >> 8) & BB_FILA8H8));
	return(u64BB >> 8);
}

static INLINE UINT32 BB_GetBitYQuitar(UINT64 * pu64)
{
	const UINT32 u32 = PrimerUno(*pu64);
	BB_ClearBit(pu64, u32);
	return(u32);
}

/*
 * Otras derivadas de instrinsics que me podrían venir bien
 */
//http://msdn.microsoft.com/en-US/library/h65k4tze(v=vs.80)
//http://msdn.microsoft.com/en-us/library/bb385231.aspx