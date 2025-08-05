/*
	Anubis

	Copyright José Carlos Martínez Galán
	Todos los derechos reservados

	-------------------------------------

	Módulo de declaración e inicialización
	de constantes

	Última modificación: 04/07/2003
*/

#include "Preprocesador.h"
#include "Constantes.h"

// Incertidumbre
const UINT32 u32INC_DESCONOCIDA	= 0;
const UINT32 u32INC_BAJA		= 1;
const UINT32 u32INC_MEDIA		= 2;
const UINT32 u32INC_ALTA		= 3;

const UINT64 au64Fila[8] = 
{
	BB_FILA8H8,
	BB_FILA7H7,
	BB_FILA6H6,
	BB_FILA5H5,
	BB_FILA4H4,
	BB_FILA3H3,
	BB_FILA2H2,
	BB_FILA1H1
};

const UINT64 au64Columna[8] =
{
	BB_COLA1A8,
	BB_COLB1B8,
	BB_COLC1C8,
	BB_COLD1D8,
	BB_COLE1E8,
	BB_COLF1F8,
	BB_COLG1G8,
	BB_COLH1H8
};

const char *aszCuadros[64] =
{
	"a8","b8","c8","d8","e8","f8","g8","h8",
	"a7","b7","c7","d7","e7","f7","g7","h7",
	"a6","b6","c6","d6","e6","f6","g6","h6",
	"a5","b5","c5","d5","e5","f5","g5","h5",
	"a4","b4","c4","d4","e4","f4","g4","h4",
	"a3","b3","c3","d3","e3","f3","g3","h3",
	"a2","b2","c2","d2","e2","f2","g2","h2",
	"a1","b1","c1","d1","e1","f1","g1","h1"
};

const TJugada JUGADA_NULA = {0,0};

const UINT8 LSB_64_table[154] =
{
#define __  0
    40,__,__,__,32,__,__,24,44,__, 46,47,45,__,16,53,43,54,55,52,
    62,63,61, 6, 7, 5,60,51,__, 4, __,__,42,__,59,__,__, 3,__,__,
    __,__,__,50,__,__,__,__,__,__, 58,__,__, 2,__,__,__,__,__,__,
    __,__,__,__,41,__,__,__,33,__, __,25,__,__,__,49,__,__,17,__,
    __,__,57,__,__, 1,__,__,__, 9, __,__,__,__,__,__,__,__,__,__,
    34,__,__,26,__,__,__,__,__,__, 18,__,__,__,__,__,35,__,__,27,
    __,10,__,__,36,__,19,28,37,39, 38,29,31,30,20,23,22,11,21,48,
    __,13,15,14,__,12,56,__,__, 0, __,__,__, 8
#undef __
};

const SINT8 as32PSReyFinalPeonesAmbosFlancos[64] =
{
	-10,-8,-6,-5,-5,-6,-8,-10,
	 -8,-5,-3,-2,-2,-3,-5, -8,
	 -6,-2, 0, 2, 2, 0,-2, -6,
	 -5, 0, 2, 6, 6, 2, 0, -5,
	 -5, 0, 2, 6, 6, 2, 0, -5,
	 -6,-2, 0, 2, 2, 0,-2, -6,
	 -8,-5,-3,-2,-2,-3,-5, -8,
	-10,-8,-6,-5,-5,-6,-8,-10
};

const SINT8 as32PSReyFinalPeonesFlancoR[64] =
{
	-60,-40,-30,-15,-6,-5,-5,-6,
	-50,-35,-25,-10,-3,-2,-2,-3,
	-50,-35,-25,  0, 0, 2, 2, 0,
	-50,-30,-20,  0, 2, 6, 6, 2,
	-50,-30,-20,  0, 2, 6, 6, 2,
	-50,-35,-25,  0, 0, 2, 2, 0,
	-50,-35,-25,-10,-3,-2,-2,-3,
	-60,-40,-30,-15,-6,-5,-5,-6
};

const SINT8 as32PSReyFinalPeonesFlancoD[64] =
{
	-6,-5,-5,-6,-15,-30,-40,-60,
	-3,-2,-2,-3,-10,-25,-35,-50,
	 0, 2, 2, 0,  0,-25,-35,-50,
	 2, 6, 6, 2,  0,-20,-30,-50,
	 2, 6, 6, 2,  0,-20,-30,-50,
	 0, 2, 2, 0,  0,-25,-35,-50,
	-3,-2,-2,-3,-10,-25,-35,-50,
	-6,-5,-5,-6,-15,-30,-40,-60
};

const SINT8 as8PSPeonPasadoBlancoFP[64] =
{
	 0, 0, 0, 0, 0, 0, 0, 0,
	60,50,40,40,40,40,50,60,
	50,40,30,30,30,30,40,50,
	40,30,20,20,20,20,30,40,
	30,20,10,10,10,10,20,30,
	20,10, 0, 0, 0, 0,10,20,
	20,10, 0, 0, 0, 0,10,20,
	 0, 0, 0, 0, 0, 0, 0, 0
};

const SINT8 as8PSPeonPasadoNegroFP[64] = // Positivo pq luego se resta
{
	 0, 0, 0, 0, 0, 0, 0, 0,
	20,10, 0, 0, 0, 0,10,20,
	20,10, 0, 0, 0, 0,10,20,
	30,20,10,10,10,10,20,30,
	40,30,20,20,20,20,30,40,
	50,40,30,30,30,30,40,50,
	60,50,40,40,40,40,50,60,
	 0, 0, 0, 0, 0, 0, 0, 0
};

const SINT8 as8psColorCasilla[64] =
{
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1
};

const UINT64 BB_ATAQUES_REY[64] =
{
	BB_B8 | BB_A7 | BB_B7,																					//  0 == A8
	BB_A8 | BB_A7 | BB_B7 | BB_C7 | BB_C8,													//  1 == B8
	BB_B8 | BB_B7 | BB_C7 | BB_D7 | BB_D8,													//  2 == C8
	BB_C8 | BB_C7 | BB_D7 | BB_E7 | BB_E8,													//  3 == D8
	BB_D8 | BB_D7 | BB_E7 | BB_F7 | BB_F8,													//  4 == E8
	BB_E8 | BB_E7 | BB_F7 | BB_G7 | BB_G8,													//  5 == F8
	BB_F8 | BB_F7 | BB_G7 | BB_H7 | BB_H8,													//  6 == G8
	BB_G8 | BB_G7 | BB_H7,																					//  7 == H8
	BB_A8 | BB_B8 | BB_B7 | BB_B6 | BB_A6,													//  8 == A7
	BB_A8 | BB_A7 | BB_A6 | BB_B8 | BB_B6 | BB_C8 | BB_C7 | BB_C6,	//  9 == B7
	BB_B8 | BB_B7 | BB_B6 | BB_C8 | BB_C6 | BB_D8 | BB_D7 | BB_D6,	// 10 == C7
	BB_C8 | BB_C7 | BB_C6 | BB_D8 | BB_D6 | BB_E8 | BB_E7 | BB_E6,	// 11 == D7
	BB_D8 | BB_D7 | BB_D6 | BB_E8 | BB_E6 | BB_F8 | BB_F7 | BB_F6,	// 12 == E7
	BB_E8 | BB_E7 | BB_E6 | BB_F8 | BB_F6 | BB_G8 | BB_G7 | BB_G6,	// 13 == F7
	BB_F8 | BB_F7 | BB_F6 | BB_G8 | BB_G6 | BB_H8 | BB_H7 | BB_H6,	// 14 == G7
	BB_G8 | BB_G7 | BB_G6 | BB_H8 | BB_H6,													// 15 == H7
	BB_B7 | BB_B6 | BB_B5 | BB_A7 | BB_A5,													// 16 == A6
	BB_A7 | BB_A6 | BB_A5 | BB_B7 | BB_B5 | BB_C7 | BB_C6 | BB_C5,	// 17 == B6
	BB_B7 | BB_B6 | BB_B5 | BB_C7 | BB_C5 | BB_D7 | BB_D6 | BB_D5,	// 18 == C6
	BB_C7 | BB_C6 | BB_C5 | BB_D7 | BB_D5 | BB_E7 | BB_E6 | BB_E5,	// 19 == D6
	BB_D7 | BB_D6 | BB_D5 | BB_E7 | BB_E5 | BB_F7 | BB_F6 | BB_F5,	// 20 == E6
	BB_E7 | BB_E6 | BB_E5 | BB_F7 | BB_F5 | BB_G7 | BB_G6 | BB_G5,	// 21 == F6
	BB_F7 | BB_F6 | BB_F5 | BB_G7 | BB_G5 | BB_H7 | BB_H6 | BB_H5,	// 22 == G6
	BB_G7 | BB_G6 | BB_G5 | BB_H7 | BB_H5,													// 23 == H6
	BB_B6 | BB_B5 | BB_B4 | BB_A6 | BB_A4,													// 24 == A5
	BB_A6 | BB_A5 | BB_A4 | BB_B6 | BB_B4 | BB_C6 | BB_C5 | BB_C4,	// 25 == B5
	BB_B6 | BB_B5 | BB_B4 | BB_C6 | BB_C4 | BB_D6 | BB_D5 | BB_D4,	// 26 == C5
	BB_C6 | BB_C5 | BB_C4 | BB_D6 | BB_D4 | BB_E6 | BB_E5 | BB_E4,	// 27 == D5
	BB_D6 | BB_D5 | BB_D4 | BB_E6 | BB_E4 | BB_F6 | BB_F5 | BB_F4,	// 28 == E5
	BB_E6 | BB_E5 | BB_E4 | BB_F6 | BB_F4 | BB_G6 | BB_G5 | BB_G4,	// 29 == F5
	BB_F6 | BB_F5 | BB_F4 | BB_G6 | BB_G4 | BB_H6 | BB_H5 | BB_H4,	// 30 == G5
	BB_G6 | BB_G5 | BB_G4 | BB_H6 | BB_H4,													// 31 == H5
	BB_B5 | BB_B4 | BB_B3 | BB_A5 | BB_A3,													// 32 == A4
	BB_A5 | BB_A4 | BB_A3 | BB_B5 | BB_B3 | BB_C5 | BB_C4 | BB_C3,	// 33 == B4
	BB_B5 | BB_B4 | BB_B3 | BB_C5 | BB_C3 | BB_D5 | BB_D4 | BB_D3,	// 34 == C4
	BB_C5 | BB_C4 | BB_C3 | BB_D5 | BB_D3 | BB_E5 | BB_E4 | BB_E3,	// 35 == D4
	BB_D5 | BB_D4 | BB_D3 | BB_E5 | BB_E3 | BB_F5 | BB_F4 | BB_F3,	// 36 == E4
	BB_E5 | BB_E4 | BB_E3 | BB_F5 | BB_F3 | BB_G5 | BB_G4 | BB_G3,	// 37 == F4
	BB_F5 | BB_F4 | BB_F3 | BB_G5 | BB_G3 | BB_H5 | BB_H4 | BB_H3,	// 38 == G4
	BB_G5 | BB_G4 | BB_G3 | BB_H5 | BB_H3,													// 39 == H4
	BB_B4 | BB_B3 | BB_B2 | BB_A4 | BB_A2,													// 40 == A3
	BB_A4 | BB_A3 | BB_A2 | BB_B4 | BB_B2 | BB_C4 | BB_C3 | BB_C2,	// 41 == B3
	BB_B4 | BB_B3 | BB_B2 | BB_C4 | BB_C2 | BB_D4 | BB_D3 | BB_D2,	// 42 == C3
	BB_C4 | BB_C3 | BB_C2 | BB_D4 | BB_D2 | BB_E4 | BB_E3 | BB_E2,	// 43 == D3
	BB_D4 | BB_D3 | BB_D2 | BB_E4 | BB_E2 | BB_F4 | BB_F3 | BB_F2,	// 44 == E3
	BB_E4 | BB_E3 | BB_E2 | BB_F4 | BB_F2 | BB_G4 | BB_G3 | BB_G2,	// 45 == F3
	BB_F4 | BB_F3 | BB_F2 | BB_G4 | BB_G2 | BB_H4 | BB_H3 | BB_H2,	// 46 == G3
	BB_G4 | BB_G3 | BB_G2 | BB_H4 | BB_H2,													// 47 == H3
	BB_B3 | BB_B2 | BB_B1 | BB_A3 | BB_A1,													// 48 == A2
	BB_A3 | BB_A2 | BB_A1 | BB_B3 | BB_B1 | BB_C3 | BB_C2 | BB_C1,	// 49 == B2
	BB_B3 | BB_B2 | BB_B1 | BB_C3 | BB_C1 | BB_D3 | BB_D2 | BB_D1,	// 50 == C2
	BB_C3 | BB_C2 | BB_C1 | BB_D3 | BB_D1 | BB_E3 | BB_E2 | BB_E1,	// 51 == D2
	BB_D3 | BB_D2 | BB_D1 | BB_E3 | BB_E1 | BB_F3 | BB_F2 | BB_F1,	// 52 == E2
	BB_E3 | BB_E2 | BB_E1 | BB_F3 | BB_F1 | BB_G3 | BB_G2 | BB_G1,	// 53 == F2
	BB_F3 | BB_F2 | BB_F1 | BB_G3 | BB_G1 | BB_H3 | BB_H2 | BB_H1,	// 54 == G2
	BB_G3 | BB_G2 | BB_G1 | BB_H3 | BB_H1,													// 55 == H2
	BB_B2 | BB_B1 | BB_A2,																					// 56 == A1
	BB_A2 | BB_A1 | BB_B2 | BB_C2 | BB_C1,													// 57 == B1
	BB_B2 | BB_B1 | BB_C2 | BB_D2 | BB_D1,													// 58 == C1
	BB_C2 | BB_C1 | BB_D2 | BB_E2 | BB_E1,													// 59 == D1
	BB_D2 | BB_D1 | BB_E2 | BB_F2 | BB_F1,													// 60 == E1
	BB_E2 | BB_E1 | BB_F2 | BB_G2 | BB_G1,													// 61 == F1
	BB_F2 | BB_F1 | BB_G2 | BB_H2 | BB_H1,													// 62 == G1
	BB_G2 | BB_G1 | BB_H2																						// 63 == H1
};

const UINT64 BB_ATAQUES_CABALLO[64] =
{
	BB_B6 | BB_C7,																									//  0 == A8
	BB_A6 | BB_C6 | BB_D7,																					//  1 == B8
	BB_A7 | BB_B6 | BB_D6 | BB_E7,																	//  2 == C8
	BB_B7 | BB_C6 | BB_E6 | BB_F7,																	//  3 == D8
	BB_C7 | BB_D6 | BB_F6 | BB_G7,																	//  4 == E8
	BB_D7 | BB_E6 | BB_G6 | BB_H7,																	//  5 == F8
	BB_E7 | BB_F6 | BB_H6,																					//  6 == G8
	BB_F7 | BB_G6,																									//  7 == H8
	BB_B5 | BB_C6 | BB_C8,																					//  8 == A7
	BB_A5 | BB_C5 | BB_D6 | BB_D8,																	//  9 == B7
	BB_A8 | BB_A6 | BB_B5 | BB_D5 | BB_E6 | BB_E8,									// 10 == C7
	BB_B8 | BB_B6 | BB_C5 | BB_E5 | BB_F6 | BB_F8,									// 11 == D7
	BB_C8 | BB_C6 | BB_D5 | BB_F5 | BB_G6 | BB_G8,									// 12 == E7
	BB_D8 | BB_D6 | BB_E5 | BB_G5 | BB_H6 | BB_H8,									// 13 == F7
	BB_E8 | BB_E6 | BB_F5 | BB_H5,																	// 14 == G7
	BB_F8 | BB_F6 | BB_G5,																					// 15 == H7
	BB_B8 | BB_B4 | BB_C5 | BB_C7,																	// 16 == A6
	BB_A8 | BB_A4 | BB_C8 | BB_C4 | BB_D7 | BB_D5,									// 17 == B6
	BB_A7 | BB_A5 | BB_B8 | BB_B4 | BB_D8 | BB_D4 | BB_E7 | BB_E5,	// 18 == C6
	BB_B7 | BB_B5 | BB_C8 | BB_C4 | BB_E8 | BB_E4 | BB_F7 | BB_F5,	// 19 == D6
	BB_C7 | BB_C5 | BB_D8 | BB_D4 | BB_F8 | BB_F4 | BB_G7 | BB_G5,	// 20 == E6
	BB_D7 | BB_D5 | BB_E8 | BB_E4 | BB_G8 | BB_G4 | BB_H7 | BB_H5,	// 21 == F6
	BB_E7 | BB_E5 | BB_F8 | BB_F4 | BB_H8 | BB_H4,									// 22 == G6
	BB_F7 | BB_F5 | BB_G8 | BB_G4,																	// 23 == H6
	BB_B7 | BB_B3 | BB_C4 | BB_C6,																	// 24 == A5
	BB_A7 | BB_A3 | BB_C7 | BB_C3 | BB_D6 | BB_D4,									// 25 == B5
	BB_A6 | BB_A4 | BB_B7 | BB_B3 | BB_D7 | BB_D3 | BB_E6 | BB_E4,	// 26 == C5
	BB_B6 | BB_B4 | BB_C7 | BB_C3 | BB_E7 | BB_E3 | BB_F6 | BB_F4,	// 27 == D5
	BB_C6 | BB_C4 | BB_D7 | BB_D3 | BB_F7 | BB_F3 | BB_G6 | BB_G4,	// 28 == E5
	BB_D6 | BB_D4 | BB_E7 | BB_E3 | BB_G7 | BB_G3 | BB_H6 | BB_H4,	// 29 == F5
	BB_E6 | BB_E4 | BB_F7 | BB_F3 | BB_H7 | BB_H3,									// 30 == G5
	BB_F6 | BB_F4 | BB_G7 | BB_G3,																	// 31 == H5
	BB_B6 | BB_B2 | BB_C3 | BB_C5,																	// 32 == A4
	BB_A6 | BB_A2 | BB_C6 | BB_C2 | BB_D5 | BB_D3,									// 33 == B4
	BB_A5 | BB_A3 | BB_B6 | BB_B2 | BB_D6 | BB_D2 | BB_E5 | BB_E3,	// 34 == C4
	BB_B5 | BB_B3 | BB_C6 | BB_C2 | BB_E6 | BB_E2 | BB_F5 | BB_F3,	// 35 == D4
	BB_C5 | BB_C3 | BB_D6 | BB_D2 | BB_F6 | BB_F2 | BB_G5 | BB_G3,	// 36 == E4
	BB_D5 | BB_D3 | BB_E6 | BB_E2 | BB_G6 | BB_G2 | BB_H5 | BB_H3,	// 37 == F4
	BB_E5 | BB_E3 | BB_F6 | BB_F2 | BB_H6 | BB_H2,									// 38 == G4
	BB_F5 | BB_F3 | BB_G6 | BB_G2,																	// 39 == H4
	BB_B5 | BB_B1 | BB_C2 | BB_C4,																	// 40 == A3
	BB_A5 | BB_A1 | BB_C5 | BB_C1 | BB_D4 | BB_D2,									// 41 == B3
	BB_A4 | BB_A2 | BB_B5 | BB_B1 | BB_D5 | BB_D1 | BB_E4 | BB_E2,	// 42 == C3
	BB_B4 | BB_B2 | BB_C5 | BB_C1 | BB_E5 | BB_E1 | BB_F4 | BB_F2,	// 43 == D3
	BB_C4 | BB_C2 | BB_D5 | BB_D1 | BB_F5 | BB_F1 | BB_G4 | BB_G2,	// 44 == E3
	BB_D4 | BB_D2 | BB_E5 | BB_E1 | BB_G5 | BB_G1 | BB_H4 | BB_H2,	// 45 == F3
	BB_E4 | BB_E2 | BB_F5 | BB_F1 | BB_H5 | BB_H1,									// 46 == G3
	BB_F4 | BB_F2 | BB_G5 | BB_G1,																	// 47 == H3
	BB_B4 | BB_C1 | BB_C3,																					// 48 == A2
	BB_A4 | BB_C4 | BB_D3 | BB_D1,																	// 49 == B2
	BB_A3 | BB_A1 | BB_B4 | BB_D4 |BB_E3 | BB_E1,										// 50 == C2
	BB_B3 | BB_B1 | BB_C4 | BB_E4 |BB_F3 | BB_F1,										// 51 == D2
	BB_C3 | BB_C1 | BB_D4 | BB_F4 |BB_G3 | BB_G1,										// 52 == E2
	BB_D3 | BB_D1 | BB_E4 | BB_G4 |BB_H3 | BB_H1,										// 53 == F2
	BB_E3 | BB_E1 | BB_F4 | BB_H4,																	// 54 == G2
	BB_F3 | BB_F1 | BB_G4,																					// 55 == H2
	BB_B3 | BB_C2,																									// 56 == A1
	BB_A3 | BB_C3 | BB_D2,																					// 57 == B1
	BB_A2 | BB_B3 | BB_D3 |BB_E2 ,																	// 58 == C1
	BB_B2 | BB_C3 | BB_E3 |BB_F2 ,																	// 59 == D1
	BB_C2 | BB_D3 | BB_F3 |BB_G2 ,																	// 60 == E1
	BB_D2 | BB_E3 | BB_G3 |BB_H2 ,																	// 61 == F1
	BB_E2 | BB_F3 | BB_H3,																					// 62 == G1
	BB_F2 | BB_G3																										// 63 == H1
};

const UINT64 BB_ATAQUES_ALFIL[64] =
{
	  18049651735527937,    45053622886727936,   22667548931719168,   11334324221640704,    5667164249915392,    2833579985862656,    1416240237150208,     567382630219904,
	4611756524879479810, 11529391036782871041, 5764696068147249408, 2882348036221108224, 1441174018118909952,  720587009051099136,  360293502378066048,  144117404414255168,
	2323857683139004420,  1197958188344280066, 9822351133174399489, 4911175566595588352, 2455587783297826816, 1227793891648880768,  577868148797087808,  288793334762704928,
	1161999073681608712,   581140276476643332,  326598935265674242, 9386671504487645697, 4693335752243822976, 2310639079102947392, 1155178802063085600,  577588851267340304,
	 580999811184992272,   290500455356698632,  145390965166737412,  108724279602332802, 9241705379636978241, 4620711952330133792, 2310355426409252880, 1155177711057110024,
	 290499906664153120,   145249955479592976,   72625527495610504,     424704217196612,   36100411639206946, 9241421692918565393, 4620710844311799048, 2310355422147510788,
	 145249953336262720,    72624976676520096,     283693466779728,       1659000848424,     141017232965652,   36099303487963146, 9241421688590368773, 4620710844295151618,
	  72624976668147712,      283691315142656,       1108177604608,          6480472064,        550848566272,     141012904249856,   36099303471056128, 9241421688590303744
};
