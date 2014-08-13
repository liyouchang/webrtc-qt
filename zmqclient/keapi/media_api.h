#ifndef __MEDIA_API_H__
#define __MEDIA_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

int MEDIA_Initialize(void);
int MEDIA_Cleanup(void);

int MEDIA_Get_IDR(int iGrp,int iSubChn);
#ifdef __cplusplus
}
#endif

#endif	//__MEDIA_API_H__
