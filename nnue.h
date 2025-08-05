#pragma once

#include "Preprocesador.h"
#include "Tipos.h"
#include "Constantes.h"
#include <immintrin.h>

// Alineación para AVX
#if defined (AVX512)
#define ALIGN_NNUE __declspec(align(64))
#else
#define ALIGN_NNUE __declspec(align(32))
#endif

#define INPUT_SIZE 773
#define HIDDEN1 256
#define HIDDEN2 64
#define OUTPUT_SIZE 1

// Estructura para los pesos de la red NNUE
ALIGN_NNUE typedef struct
{
    ALIGN_NNUE float W1[HIDDEN1][INPUT_SIZE];  // Pesos de la primera capa
    ALIGN_NNUE float b1[HIDDEN1];              // Bias de la primera capa
    ALIGN_NNUE float W2[HIDDEN2][HIDDEN1];     // Pesos de la segunda capa
    ALIGN_NNUE float b2[HIDDEN2];              // Bias de la segunda capa 
    ALIGN_NNUE float W3[1][HIDDEN2];           // Pesos de la capa de salida
    ALIGN_NNUE float b3[1];                    // Bias de la capa de salida
} NNUEWeights;

extern NNUEWeights nnue_weights_T0;
extern NNUEWeights nnue_weights_T1;
extern NNUEWeights nnue_weights_T2;
extern NNUEWeights nnue_weights_T3;
extern NNUEWeights nnue_weights_T4;
extern NNUEWeights nnue_weights_T5;
extern NNUEWeights nnue_weights_T6;
extern NNUEWeights nnue_weights_T7;
extern NNUEWeights nnue_weights_T8;
extern NNUEWeights nnue_weights_T9;

extern int nFeaturesActivas[37]; // 32 piezas + 5 para turno y enroques
extern int nNumFeaturesActivas;

#define FEAT_TURNO 768
#define FEAT_ENROQUE_K 769
#define FEAT_ENROQUE_Q 770
#define FEAT_ENROQUE_k 771
#define FEAT_ENROQUE_q 772

#define FEAT_PB 0
#define FEAT_CB 1
#define FEAT_AB 2
#define FEAT_TB 3
#define FEAT_DB 4
#define FEAT_RB 5
#define FEAT_PN 6
#define FEAT_CN 7
#define FEAT_AN 8
#define FEAT_TN 9
#define FEAT_DN 10
#define FEAT_RN 11

#define FEAT_PIEZACASILLA(piece, sq) piece * 64 + sq

void RecalcularFeatures(TPosicion * pPos);
int nnue_evaluate(TPosicion* pPos, NNUEWeights* pnnue_weights);
int load_weights(const char* filename, NNUEWeights* pnnue_weights);