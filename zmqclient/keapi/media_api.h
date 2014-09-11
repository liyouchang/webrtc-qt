#ifndef __MEDIA_API_H__
#define __MEDIA_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "media_define.h"

#ifdef __cplusplus
extern "C" {
#endif

int MEDIA_Initialize(void);								//初始化媒体编码
int MEDIA_Cleanup(void);								//反初始化媒体编码

int MEDIA_Get_IDR(int iGrp,int iSubChn);				//获取idr帧
unsigned long long MEDIA_Get_Stamp(int iGrp,int iSubChn);//获取当前编码的最新时间戳。

int MEDIA_Audio_Talk(char *pAudioData,int iLen);		//对讲音频调用接口，pAudioData是纯音频数据，iLen是数据的长度，这里数据部分不需要任何处理G.711 160字节的纯数据，不包含0 0 0 1
int MEDIA_Jpeg_Generate(int iJpegNum,int iJpegGop,MEDIA_JPEG_CALLBACK jpeg_callback);//生成iJpegNum张图片，间隔iJpegGop帧生成一张，并且将生成的图片同过jpeg_callback回调给调用者

int MEDIA_VDA_MD_Start(int iGrp,int iArea);				//开始检测移动侦测
int MEDIA_VDA_MD_Stop(int iGrp,int iArea);				//停止检测移动侦测

int MEDIA_VDA_OD_Start(int iChn);						//开始检测遮挡报警
int MEDIA_VDA_OD_Stop(int iChn);						//停止检测遮挡报警

int MEDIA_VDA_SL_Start(int iChn);						//开始检测信号丢失报警
int MEDIA_VDA_SL_Stop(int iChn);						//停止检测信号丢失报警

#ifdef __cplusplus
}
#endif

#endif	//__MEDIA_API_H__
