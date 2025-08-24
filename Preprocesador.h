/*
	Anubis

	José Carlos Martínez Galán
*/

#pragma once
#if !defined(WIN64) && !defined(__linux__)
#define WIN64
#endif
#pragma warning(disable: 4996)
#pragma warning(disable: 4324)

/////////////////////////////////////////////////
//
#define VERS_302
//
//		Pte:
//			Poner PODA_JC como un número que sea la prof máxima a partir de la cual podemos podar (ahora mismo es 4 a piñón; podríamos probar 5, 6, ...) (Berserk usa 9)
// 
//			IIR en lugar de IID (no las dos cosas juntas) - explicación: igual que analizo prof 1 antes que prof 2 para ordenar el árbol, si me encuentro una rama desordenada (no hash move)
//				puedo hacer una búsqueda con prof reducida para ordenarlo, pero no rebusco otra vez a prof nominal de inmediato, sino que lo dejo ahí para la próxima vez que la búsqueda
//				pase por ese nodo
// 
//			Tabla hash: si tengo una Ubound x a prof d, y estoy buscando con prof < d, y ese x no es < alfa, no puedo podar, pero podría usar el dato para limitar el
//						resultado devuelto. Si mi búsqueda, a prof d-1, devuelve x+10, yo sé que en prof d es como máximo x, luego puedo devolver x, en vez de x+10
// 
//			Ideas: Ver 'Main search loop' de Laser 1.7
//				 : Ver PROBCUT en RukChess
//				 : Probar QS_PODA_FUTIL más pequeño (Hakka rel 3 utiliza 50)
//				 : Ajuste de tiempo en Astra 5.1, bestMove()
//				 : Extensión inesperada -> cuando es muy obvio que espero fail low, y ocurre fail high o al revés, extender y repetir la búsqueda en ese nodo
// 
//			Ordenación: tengo que mejorar la ordenación, con alguna heurística tipo history o countermove o incluso https://www.chessprogramming.org/Last_Best_Reply
//						ver RukChess COUNTER_MOVE_HISTORY
//						https://www.chessprogramming.org/History_Heuristic
//						Ver Obsidian 16 (buscar '// Update histories' en search.cpp)
// 
//			Razoring: probar modo menos arriesgado (tipo SF 17.1)
// 
//			Eval static en tabla hash normal, en vez de una especial (problema porque en la hash eval almaceno muchos bitboards)
//			IIR: jugar con la prof de corte (ahora mismo >= 4 a piñón)
//			En AlfaBeta, cuando estoy en excluded move, ya no necesito evaluar porque esa posición ya la he evaluado antes de esta llamada ¿no?
//			Idea: ver código de extensión singular en Alfabeta
// 
//			3.02_:
//				Implemento los cambios de Jim Ablett que, en principio, no se activan
//
//			3.02a:	(12)
//				1, 25 100 9, 250 100 13
//
//			3.02b:	(10)
//				1, 50 100 9, 250 100 13
//
//			3.02c:	(1)
//				1, 75 100 9, 250 100 13
//
//			3.02d:	(11)
//				1, 90 100 9, 250 100 13
//
//			3.02e:	(11)
//				1, 125 100 9, 250 100 13
//
//			3.02f:	(8)
//				1, 150 100 9, 250 100 13
//
//			3.02g:	(18)
//				1, 175 100 9, 250 100 13
//
//			3.02h:	(5)
//				1, 200 100 9, 250 100 13
// 
//			Me quedo con la g (el 13 final ha sido un error de compilación, y las pruebas se han hecho así, aunque yo querría haber dejado 12)
//
/////////////////////////////////////////////////

// Fichero log
#define	AV_NO_LOG		0
#define	AV_LOG			1
/////////////////////////////////////////////////
#define	FICHERO_LOG		AV_NO_LOG
/////////////////////////////////////////////////
//#define DEBUG_NNUE
//#define MODO_PARAM

#ifdef _MSC_VER       
	//#define AVX2
	#define AVX512
	//#define SOFT_INTRINSICS
#endif

#if defined(VERS_302)
	#define VERSION "Anubis v3.02"
	#define NOMBRE_LOG "An_3.02_"


											//	1.00	1.01	1.02	2.00	2.01	2.02	2.03	2.10	3.00	3.01	3.02
											//--------------------------------------------------------------------------------------
	#define AS_DELTA_FH_MULT 2				//																					   x	// Multiplicador de delta en fail high
	//#define AS_DELTA_FL_MULT 2			//																					   .	// Multiplicador de delta en fail low
	#define AS_DELTA_FL_MULT 3				//																					   x	// Multiplicador de delta en fail low
	#define AS_DELTA_DIN_BASE 10			//			   8																	   x	// El delta se recalcula en cada iteración a partir de la eval; el número determina la base del delta
	#define AS_DELTA_DIN_DIVISOR 8			//			   8																	   x	// Complementa al anterior (DIN es de dinámico)
	//#define BUS_BORRAR_KILLERS			//																					   .

	#define BUS_IID							//																					   x
	#define BUS_IIR_PV 1					//																					   x	// Reducir 1 ply en nodos PV
	#define BUS_IIR_CUT 1					//																					   x	// Reducir 1 ply en nodos cut
	//#define BUS_IIR_CUT 2					//													 -15							   .	// Respecto a BUS_IIR_CUT 1
	#define BUS_IIR_ALL 1					//													 114							   x	// Respecto a quitarlo
	//#define BUS_LMR 1						//																					   .
	//#define BUS_LMR 2						//			  -5	 -28															   .	// Respecto a BUS_LMR 3
	#define BUS_LMR 3						//																					   x	// BUS_LMR 2 + s32Prof >= 17 && (bEsCutNode || !bMejorando) && u32Legales >= (s32Prof + 5)
	//#define BUS_LMR 4						//													 -53							   .	// Respecto a BUS_LMR 3
	//#define BUS_RED_LEGALES_X_PROF		//																					   .	// En reducciones, limitar legales por prof

	//#define BUS_NM_ADAP 1					//																					   .	// 3 + prof / 5
	//#define BUS_NM_ADAP 2					//																					   .	// 3 + prof / 3 + min
	#define BUS_NM_ADAP 3					//																					   x	// 3 + prof / 3 + min + verif
	//#define BUS_NM_NO_CREO_QUE_PUEDA_FH	//																					   .

	//#define BUS_RAZOR_MARGEN 300			//							 -24													   .	// Respecto a BUS_RAZOR_MARGEN 400
	//#define BUS_RAZOR_MARGEN 350			//							 -17													   .	// Respecto a BUS_RAZOR_MARGEN 400
	#define BUS_RAZOR_MARGEN 400			//																					   x
	//#define BUS_RAZOR_MARGEN 450			//							 -12													   .	// Respecto a BUS_RAZOR_MARGEN 400
	//#define EXT_CAPT_CERCA				//																					   .
	//#define EXT_FINAL						//					 -27															   .
	//#define EXT_PEON_7					//					 -14															   .
	//#define EXT_RECAPTURA					//					 -33															   .
	//#define EXT_SINGULAR 500				//																					   .	// Diferencia para extensión singular doble
	//#define EXT_SINGULAR 100				//			 -16	  13															   .	// Respecto a EXT_SINGULAR 500
	#define EXT_SINGULAR 50					//					  33															   x	// Respecto a EXT_SINGULAR 500
	//#define EXT_SINGULAR 25				//			 		   1															   .	// Respecto a EXT_SINGULAR 500
	//#define HASH_NO_GRABAR_MATES			//																					   .
	//#define HASH_RANURAS 3				//													 -40							   .	// Respecto a HASH_RANURAS 4
	#define HASH_RANURAS 4					//																					   x
	//#define HASH_RANURAS 5				//									 +43	 -25									   .	// Respecto a HASH_RANURAS 4
	#define PODA_FUTIL						//																					   x
	//#define PODA_JC 1						//																					   .
	#define PODA_JC 2						//																					   x
	//#define PODA_JC 3						//					 -47															   .	// Respecto a PODA_JC 2
	//#define PODA_LMP						//					 -19							-118							   .
	#define PODA_SEE_NEGATIVO 1				//																					   x
	//#define PODA_SEE_NEGATIVO 2			//													-287							   .	// Respecto a PODA_SEE_NEGATIVO 1

	// 1, 100 100 7, 250 100 10				//																	  -44			   .
	// 1, 100 100 8, 250 100 10				//																	  -23			   .
	// 1, 100 100 9, 250 100 10				//																			  -3	   .	// Ver PODA_HASH_PROF_DIF en AlfaBeta.c
	// 1, 100 100 10, 250 100 10			//																	  -39			   .
	// 1, 100 100 11, 250 100 10			//																	  -25			   .
	// 1, 100 100 12, 250 100 10			//																	  -25			   .
	// 1, 100 100 9, 250 100 6				//																			  -11	   .
	// 1, 100 100 9, 250 100 7				//																			  -10	   .
	// 1, 100 100 9, 250 100 8				//																			  -3	   .
	// 1, 100 100 9, 250 100 9				//																			  -12	   .
	// 1, 100 100 9, 250 100 11				//																			  -10	   .
	// 1, 100 100 9, 250 100 12				//																					   x
	// 1, 100 100 9, 250 100 13				//																			  -6	   .

	// 1, 25 100 9, 250 100 13				//a																					  12
	// 1, 50 100 9, 250 100 13				//b																					  10
	// 1, 75 100 9, 250 100 13				//c																					   1
	// 1, 90 100 9, 250 100 13				//d																					  11

	// 1, 125 100 9, 250 100 13				//e																					  11
	// 1, 150 100 9, 250 100 13				//f																					   8
	// 1, 175 100 9, 250 100 13				//g																					  18
	// 1, 200 100 9, 250 100 13				//h																					   5

	#define REFUTACION						//																		7			   x

	//#define QS_PODA_DELTA 1000			//									 +17											   .
	#define QS_PODA_DELTA 1050				//									 +48											   x
	//#define QS_PODA_DELTA 1100			//									 +28											   .
	#define QS_PODA_FUTIL 300				//																					   x
	#define QS_PODA_SEE_NEGATIVO			//																					   x
	#define QS_PROF_JAQUES 2				//																					   x
	//#define QS_STANDPAT_AUM 5				//																					   .	// Stand pat aumentado. Idea tomada de Ippolit
	//#define RED_SOSEZ 10					//																					   .
	//#define RED_SOSEZ 15					//																					   .
	//#define RED_FH						//																					   .

	//#define PER_EXTDOBLE1SOLOESCAPE		//																					   .
	#define PER_PODARJAQUESSEENEGATIVO		//																					   x	// Sólo con QS_PROF_JAQUES activada
	#define PER_PODARJAQUESPOSPERDIDA		//																					   x	// Sólo con QS_PROF_JAQUES activada
	#define PER_MEMORIAHASH 350				//																					   x

	//#define QS_COMPROBAR_PROGRESO_JAQUES
	//#define QS_INTENTAR_MATES_EN_QSNORMAL

#endif // #if defined(VERS_xx)
