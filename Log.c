/*
	Anubis

	José Carlos Martínez Galán
*/

#include "Preprocesador.h"


#include "Tipos.h"
#include <stdlib.h>
#include "Variables.h"
#include <string.h>

void AbrirFicheroLog(void)
{
#if (FICHERO_LOG == AV_LOG)
	SINT32	i;
	char	szLog[256];
	char	szNum[6];

	for (i = 0;i < 10000;i++)
	{
		_itoa(i,szNum,10);
		strcpy(szLog,NOMBRE_LOG);
		strncat(szLog,szNum,sizeof(szLog) - strlen(szLog) - 1);
		strncat(szLog,".log",sizeof(szLog) - strlen(szLog) - 1);
		if ((pFLog = fopen(szLog,"r")) == NULL)
			break;
		fclose(pFLog);
	}
	pFLog = fopen(szLog,"w");
	setbuf(pFLog,NULL);
#if defined(LOGLMR)
  pFLogLMR = fopen("LMR.log", "w");
  setbuf(pFLogLMR, NULL);
#endif // LOGLMR
#endif // #if (FICHERO_LOG == AV_LOG)
}


