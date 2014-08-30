#ifndef __STORE_DEFINE_H__
#define __STORE_DEFINE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common_define.h"

typedef enum
{
	STORE_TYPE_SWITCH = 0,									//开关量
	STORE_TYPE_MV,											//移动侦测
	STORE_TYPE_PLAN,										//计划录像
	STORE_TYPE_MANUAL,										//手动录像
	STORE_TYPE_JPEG,										//手动录像
	STORE_TYPE_COUNT
}	e_store_type;

typedef struct store_list
{
	char filePath[48];										//文件路径加名称，名称里面包含开始时间
	int iFileSize;											//文件大小
	st_clock_t stEndTime;									//文件的结束时间
}	st_store_list_t;



#endif	//__STORE_DEFINE_H__
