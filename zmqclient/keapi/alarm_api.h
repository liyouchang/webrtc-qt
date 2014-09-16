#ifndef __ALARM_API_H__
#define __ALARM_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "alarm_define.h"

#ifdef __cplusplus
extern "C" {
#endif

int ALARM_Initialize(void);								//初始化媒体编码
int ALARM_Cleanup(void);								//反初始化媒体编码

int ALARM_MD_Defense(int iChn);
int ALARM_MD_UnDefense(int iChn);
int ALARM_Register_Callback(ALARM_CALLBACK alarm_callback);

#ifdef __cplusplus
}
#endif

#endif	//__ALARM_API_H__
