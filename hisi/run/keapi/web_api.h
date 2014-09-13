#ifndef WEB_API_H
#define WEB_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "web_define.h"

/**********************************************************************/
//module WEB
/**********************************************************************/
int WEB_Initialize(void);
int WEB_Cleanup(void);

int WEB_Client_Procotol(char *pData,int iLen,char *pDst);

int RTSP_SERVER_GetPort(void);

#ifdef __cplusplus
}
#endif
#endif		//WEB_API_H
