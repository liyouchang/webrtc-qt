#ifndef __MEDIA_API_H__
#define __MEDIA_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//初始化媒体编码,
int MEDIA_Initialize(void);
//反初始化媒体编码
int MEDIA_Cleanup(void);
//获取idr帧
int MEDIA_Get_IDR(int iGrp,int iSubChn);

#ifdef __cplusplus
}
#endif

#endif	//__MEDIA_API_H__
