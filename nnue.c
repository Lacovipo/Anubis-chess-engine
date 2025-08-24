#include "Preprocesador.h"
#include "nnue.h"
#include <math.h>
#include <string.h>
#ifdef __ANDROID__
    #include <arm_neon.h>
#else
    #include <immintrin.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "Tipos.h"
#include "Inline.h"
#include "Bitboards_inline.h"

int nFeaturesActivas[37]; // 32 piezas + 5 para turno y enroques
int nNumFeaturesActivas;

NNUEWeights nnue_weights_T0;
NNUEWeights nnue_weights_T1;
NNUEWeights nnue_weights_T2;
NNUEWeights nnue_weights_T3;
NNUEWeights nnue_weights_T4;
NNUEWeights nnue_weights_T5;
NNUEWeights nnue_weights_T6;
NNUEWeights nnue_weights_T7;
NNUEWeights nnue_weights_T8;
NNUEWeights nnue_weights_T9;

static ALIGN_NNUE float h1[HIDDEN1];
static ALIGN_NNUE float h2[HIDDEN2];

// Función optimizada para ReLU
static inline float relu_f(float x)
{
    return x > 0.0f ? x : 0.0f;
}

void AplicarRelu(float * const hin, float * const hout)
{
#if defined(AVX512)
    for (int i = 0; i < HIDDEN1; i += 16)
    {
        __m512 v = _mm512_load_ps(&hin[i]);
        __m512 zero = _mm512_setzero_ps();
        v = _mm512_max_ps(v, zero);
        _mm512_store_ps(&hout[i], v);
    }
#elif defined(AVX2)
    for (int i = 0; i < HIDDEN1; i += 8)
    {
        __m256 v = _mm256_load_ps(&hin[i]);
        __m256 zero = _mm256_setzero_ps();
        v = _mm256_max_ps(v, zero);
        _mm256_store_ps(&hout[i], v);
    }
#else
    for (int i = 0; i < HIDDEN1; ++i)
        hout[i] = relu_f(hin[i]);
#endif
}
#if defined(AVX2)
// Suma horizontal de un __m256 (AVX)
static inline float horizontal_sum_avx(__m256 v)
{
    // Extrae los dos vectores de 128 bits
    __m128 vlow = _mm256_castps256_ps128(v);
    __m128 vhigh = _mm256_extractf128_ps(v, 1);
    // Suma los dos vectores de 128 bits
    __m128 sum128 = _mm_add_ps(vlow, vhigh);
    // Suma horizontal en 128 bits
    __m128 shuf = _mm_movehdup_ps(sum128);        // (3,2,1,0) -> (2,2,0,0)
    __m128 sums = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    return _mm_cvtss_f32(sums);
}
#endif

void RecalcularFeatures(TPosicion * pPos)
{
    nNumFeaturesActivas = 0;

    for (int pieza = PB; pieza <= RN; pieza++)
    {
        UINT64 u64Temp;
        int piece = -1;

        switch (pieza)
        {
            case PB: u64Temp = pPos->u64PeonesB; piece = FEAT_PB; break;
            case CB: u64Temp = pPos->u64CaballosB; piece = FEAT_CB; break;
            case AB: u64Temp = pPos->u64AlfilesB; piece = FEAT_AB; break;
            case TB: u64Temp = pPos->u64TorresB; piece = FEAT_TB; break;
            case DB: u64Temp = pPos->u64DamasB; piece = FEAT_DB; break;
            case RB: u64Temp = 0; nFeaturesActivas[nNumFeaturesActivas++] = FEAT_PIEZACASILLA(FEAT_RB, pPos->u8PosReyB); break;
            case PN: u64Temp = pPos->u64PeonesN; piece = FEAT_PN; break;
            case CN: u64Temp = pPos->u64CaballosN; piece = FEAT_CN; break;
            case AN: u64Temp = pPos->u64AlfilesN; piece = FEAT_AN; break;
            case TN: u64Temp = pPos->u64TorresN; piece = FEAT_TN; break;
            case DN: u64Temp = pPos->u64DamasN; piece = FEAT_DN; break;
            case RN: u64Temp = 0; nFeaturesActivas[nNumFeaturesActivas++] = FEAT_PIEZACASILLA(FEAT_RN, pPos->u8PosReyN); break;
            default: continue;
        }

        while (u64Temp)
        {
            int sq = BB_GetBitYQuitar(&u64Temp);

            assert(piece >= 0 && piece <= 10);
            nFeaturesActivas[nNumFeaturesActivas++] = FEAT_PIEZACASILLA(piece, sq); // piece * 64 + sq;
        }
    } // for (int pieza = PB; pieza <= RN; pieza++)

    // Turno
    if (Pos_GetTurno(pPos) == BLANCAS)
        nFeaturesActivas[nNumFeaturesActivas++] = FEAT_TURNO;

    // Enroques
    if (Pos_GetPuedeCortoB(pPos))
        nFeaturesActivas[nNumFeaturesActivas++] = FEAT_ENROQUE_K;	// K
    if (Pos_GetPuedeLargoB(pPos))
        nFeaturesActivas[nNumFeaturesActivas++] = FEAT_ENROQUE_Q;	// Q	
    if (Pos_GetPuedeCortoN(pPos))
        nFeaturesActivas[nNumFeaturesActivas++] = FEAT_ENROQUE_k;	// k
    if (Pos_GetPuedeLargoN(pPos))
        nFeaturesActivas[nNumFeaturesActivas++] = FEAT_ENROQUE_q;	// q
}

// Added new function to load weights from memory
int load_weights_from_memory(const unsigned char* data, size_t size, NNUEWeights* pnnue_weights)
{
    size_t total_params = (INPUT_SIZE * HIDDEN1) + HIDDEN1 + (HIDDEN1 * HIDDEN2) + HIDDEN2 + (HIDDEN2 * OUTPUT_SIZE) + OUTPUT_SIZE;
    if (size != total_params * sizeof(float))
        return 0;

    const float* weights = (const float*)data;

    // Distribuimos los pesos en la estructura
    const float* w = weights;
    const float* b = w + (INPUT_SIZE * HIDDEN1);
    const float* w2 = b + HIDDEN1;
    const float* b2 = w2 + (HIDDEN1 * HIDDEN2);
    const float* w3 = b2 + HIDDEN2;
    const float* b3 = w3 + (HIDDEN2 * OUTPUT_SIZE);

    // Copiamos los pesos a nuestra estructura
    for (int i = 0; i < HIDDEN1; i++)
    {
        pnnue_weights->b1[i] = b[i];
        for (int j = 0; j < INPUT_SIZE; j++)
            pnnue_weights->W1[i][j] = w[i * INPUT_SIZE + j];
    }
    for (int i = 0; i < HIDDEN2; i++)
    {
        pnnue_weights->b2[i] = b2[i];
        for (int j = 0; j < HIDDEN1; j++)
            pnnue_weights->W2[i][j] = w2[i * HIDDEN1 + j];
    }
    pnnue_weights->b3[0] = b3[0];
    for (int i = 0; i < HIDDEN2; i++)
        pnnue_weights->W3[0][i] = w3[i];

    // Transponer W1 y W2 para máxima eficiencia AVX
    float (*W1_transposed)[HIDDEN1] = malloc(INPUT_SIZE * HIDDEN1 * sizeof(float));
    for (int i = 0; i < HIDDEN1; i++)
    {
        for (int j = 0; j < INPUT_SIZE; j++)
            W1_transposed[j][i] = w[i * INPUT_SIZE + j]; // Transponer
    }
    memcpy(pnnue_weights->W1, W1_transposed, INPUT_SIZE * HIDDEN1 * sizeof(float));
    free(W1_transposed);

    float (*W2_transposed)[HIDDEN2] = malloc(HIDDEN1 * HIDDEN2 * sizeof(float));
    for (int i = 0; i < HIDDEN2; i++)
    {
        for (int j = 0; j < HIDDEN1; j++)
        {
            W2_transposed[j][i] = w2[i * HIDDEN1 + j];
        }
    }
    memcpy(pnnue_weights->W2, W2_transposed, HIDDEN1 * HIDDEN2 * sizeof(float));
    free(W2_transposed);

    return 1;
}

int load_weights(const char* filename, NNUEWeights* pnnue_weights)
{
    FILE* f = fopen(filename, "rb");
    if (!f)
        return 0;

    size_t total_params = (INPUT_SIZE * HIDDEN1) + HIDDEN1 + (HIDDEN1 * HIDDEN2) + HIDDEN2 + (HIDDEN2 * OUTPUT_SIZE) + OUTPUT_SIZE;
    float* weights = (float*)malloc(total_params * sizeof(float));
    if (!weights)
        return 0;

    size_t read = fread(weights, sizeof(float), total_params, f);
    fclose(f);

    if (read != total_params)
    {
        free(weights);
        return 0;
    }

    // Distribuimos los pesos en la estructura
    float* w = weights;
    float* b = w + (INPUT_SIZE * HIDDEN1);
    float* w2 = b + HIDDEN1;
    float* b2 = w2 + (HIDDEN1 * HIDDEN2);
    float* w3 = b2 + HIDDEN2;
    float* b3 = w3 + (HIDDEN2 * OUTPUT_SIZE);

    // Copiamos los pesos a nuestra estructura
    for (int i = 0; i < HIDDEN1; i++)
    {
        pnnue_weights->b1[i] = b[i];
        for (int j = 0; j < INPUT_SIZE; j++)
            pnnue_weights->W1[i][j] = w[i * INPUT_SIZE + j];
    }

    for (int i = 0; i < HIDDEN2; i++)
    {
        pnnue_weights->b2[i] = b2[i];
        for (int j = 0; j < HIDDEN1; j++)
            pnnue_weights->W2[i][j] = w2[i * HIDDEN1 + j];
    }

    pnnue_weights->b3[0] = b3[0];
    for (int i = 0; i < HIDDEN2; i++)
        pnnue_weights->W3[0][i] = w3[i];

    // Transponer W1 y W2 para máxima eficiencia AVX
    float (*W1_transposed)[HIDDEN1] = malloc(INPUT_SIZE * HIDDEN1 * sizeof(float));
    for (int i = 0; i < HIDDEN1; i++)
    {
        for (int j = 0; j < INPUT_SIZE; j++)
            W1_transposed[j][i] = w[i * INPUT_SIZE + j];  // Transponer
    }
    memcpy(pnnue_weights->W1, W1_transposed, INPUT_SIZE * HIDDEN1 * sizeof(float));
    free(W1_transposed);

    float (*W2_transposed)[HIDDEN2] = malloc(HIDDEN1 * HIDDEN2 * sizeof(float));
    for (int i = 0; i < HIDDEN2; i++)
    {
        for (int j = 0; j < HIDDEN1; j++)
        {
            W2_transposed[j][i] = w2[i * HIDDEN1 + j];
        }
    }
    memcpy(pnnue_weights->W2, W2_transposed, HIDDEN1 * HIDDEN2 * sizeof(float));
    free(W2_transposed);

    free(weights);
    return 1;
}

static void capa1_avx(const int* features, int num_features, float* salida, float * b1, float * W1)
{
#if defined(AVX512)
    for (int j = 0; j < HIDDEN1; j += 16)
    {        
        assert(((uintptr_t)&b1[j] & 63) == 0);  // Si falla: usar __m512 sum = _mm512_loadu_ps(&b1[j]);
        __m512 sum = _mm512_load_ps(&b1[j]);    // HIDDEN1 tiene que ser múltiplo de 16
        for (int i = 0; i < num_features; ++i)
        {
            int idx = features[i];
            assert(((uintptr_t)&W1[idx * HIDDEN1 + j] & 63) == 0);  // Si falla alguna vez, usar _mm512_loadu_ps (con alineación)
			__m512 w = _mm512_load_ps(&W1[idx * HIDDEN1 + j]);     // HIDDEN1 tiene que ser múltiplo de 16
            sum = _mm512_add_ps(sum, w);
        }
        assert(((uintptr_t)&salida[j] & 63) == 0);
        _mm512_store_ps(&salida[j], sum);
    }
#elif defined(AVX2)
    for (int j = 0; j < HIDDEN1; j += 8)
    {
        assert(((uintptr_t)&b1[j] & 31) == 0);
        __m256 sum = _mm256_load_ps(&b1[j]);
        for (int i = 0; i < num_features; ++i)
        {
            int idx = features[i];
            assert(((uintptr_t)&W1[idx * HIDDEN1 + j] & 31) == 0);
            __m256 w = _mm256_load_ps(&W1[idx * HIDDEN1 + j]);
            sum = _mm256_add_ps(sum, w);
        }
        _mm256_store_ps(&salida[j], sum);
    }
#else
    for (int i = 0; i < HIDDEN1; i++)
    {
        float sum = b1[i];
        int k = 0;
        for (; k < num_features; k++)
        {
            sum += W1[features[k] * HIDDEN1 + i];
        }
        
        salida[i] = sum;
    }
#endif
}

static void capa_densa_avx(const float* entrada, const float* W, const float* b, float* salida, int in_size, int out_size)
{
#if defined(AVX512)
    for (int j = 0; j < out_size; j += 16)
    {
        assert(((uintptr_t)&b[j] & 63) == 0);
        __m512 sum = _mm512_load_ps(&b[j]);
        for (int i = 0; i < in_size; ++i)
        {
            assert(((uintptr_t)&W[i * out_size + j] & 63) == 0);
            __m512 w = _mm512_load_ps(&W[i * out_size + j]);
            __m512 x = _mm512_set1_ps(entrada[i]);
            sum = _mm512_fmadd_ps(x, w, sum);
        }
        assert(((uintptr_t)&salida[j] & 63) == 0);
        _mm512_store_ps(&salida[j], sum);
    }
#elif defined(AVX2)
    for (int j = 0; j < out_size; j += 8)
    {
        assert(((uintptr_t)&b[j] & 31) == 0);
        __m256 sum = _mm256_load_ps(&b[j]);
        for (int i = 0; i < in_size; ++i)
        {
            assert(((uintptr_t)&W[i * out_size + j] & 31) == 0);
            __m256 w = _mm256_load_ps(&W[i * out_size + j]);
            __m256 x = _mm256_set1_ps(entrada[i]);
            sum = _mm256_fmadd_ps(x, w, sum);
        }
        assert(((uintptr_t)&salida[j] & 31) == 0);
        _mm256_store_ps(&salida[j], sum);
    }
#else
    for (int j = 0; j < out_size; ++j)
    {
        float sum = b[j];
        for (int i = 0; i < in_size; ++i)
            sum += entrada[i] * W[i * out_size + j];
        salida[j] = sum;
    }
#endif
}

int nnue_evaluate(TPosicion* pPos, NNUEWeights* pnnue_weights)
{
    //
    // Capa 1
    //
    RecalcularFeatures(pPos);
    capa1_avx(nFeaturesActivas, nNumFeaturesActivas, h1, pnnue_weights->b1, (float*)pnnue_weights->W1);
    AplicarRelu(h1, h1);

    //
    // Capa 2
    //
    capa_densa_avx(h1, (float*)pnnue_weights->W2, pnnue_weights->b2, h2, HIDDEN1, HIDDEN2);
    AplicarRelu(h2, h2);

    //
    // Bucle de suma final
    //
    float sum = pnnue_weights->b3[0];
    float* W3 = pnnue_weights->W3[0];

#if defined(AVX512)
    __m512 acc = _mm512_setzero_ps();
    int i = 0;
    for (; i <= HIDDEN2 - 16; i += 16)
    {
        __m512 v1 = _mm512_load_ps(&h2[i]);
        __m512 v2 = _mm512_load_ps(&W3[i]);
        acc = _mm512_fmadd_ps(v1, v2, acc);
    }
    sum += _mm512_reduce_add_ps(acc);
    for (; i < HIDDEN2; ++i)
        sum += h2[i] * W3[i];
#elif defined(AVX2)
    __m256 acc = _mm256_setzero_ps();
    int i = 0;
    for (; i <= HIDDEN2 - 8; i += 8)
    {
        __m256 v1 = _mm256_load_ps(&h2[i]);
        __m256 v2 = _mm256_load_ps(&W3[i]);
        acc = _mm256_fmadd_ps(v1, v2, acc);
    }
    sum += horizontal_sum_avx(acc);
    for (; i < HIDDEN2; ++i)
        sum += h2[i] * W3[i];
#else
    for (int i = 0; i < HIDDEN2; ++i)
        sum += h2[i] * W3[i];
#endif

    const int i1 = (int)roundf(tanhf(sum) * 1000.0f);

    return i1;
}
