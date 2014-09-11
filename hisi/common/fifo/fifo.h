#ifndef __MY_FIFO_MODULE_H__
#define __MY_FIFO_MODULE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include "../common_define.h"
#include "convert_ts_stream/convert_ts_stream.h"
#include "convert_ps_stream/convert_ps_stream.h"
#include "convert_rtp_stream/convert_rtp_stream.h"
#include "convert_avi_stream/convert_avi_stream.h"
#include "convert_avi2rtp_file/convert_avi2rtp_file.h"
#include "convert_web2rtp_file/convert_web2rtp_file.h"
#include "convert_h264_stream/convert_h264_stream.h"

#define FIFO_MAX_NUM		8
#define FIFO_READ_NUM		8
typedef struct FIFO_CHN
{
	e_fifo_start enType;
	int iGrp;
	int iChn;
	int	capacity;									//容量，单位字节
	
	char * pBuf;									//缓存内存区的首地址
	int iWrite;										//写指针
	int iDrop;										//缓存地址区域块(4块)首地址
	int iRead_Base;									//最旧数据指针
    int iRead[FIFO_READ_NUM];						//FIFO_READ_NUM个读指针
	pthread_mutex_t	lock_write;
	pthread_mutex_t	lock_read[FIFO_READ_NUM];
	int iStamp;
}	st_fifoInfo_t;

extern st_fifoInfo_t g_stFifo[FIFO_MAX_NUM];
extern int FIFO_Request(st_fifo_t stFifo,int * pHandle,int second);
extern int FIFO_Read(int iHandle,int rId,char *pData);
extern int FIFO_Release(int iHandle,int rId);

#define FIFO_MAX_STREAM		12
#define FIFO_READ_STREAM	4
#define FIFO_MAX_BUFFER		750
#define FIFO_AVI_BUFFER		100
struct FIFO_DATA
{
	int iLen;
	char *pData;
};
typedef struct FIFO_STREAM
{
	e_fifo_stream enType;
	int iGrp;
	int iChn;
	int iWrite;
	int iConvertThread;
	pthread_t thread_convert;
	
	int iRealMaxBuffer;
	struct FIFO_DATA stFifoData[FIFO_MAX_BUFFER];
	
	int iRead[FIFO_READ_STREAM];
	
	int speed;
	int pull_time;
	char file_name[128];
	int iPreSecond;									//预录时间
	
	int tsPtmCnt;
	int tsVideoCnt;
	int tsAudioCnt;
	int tsFrameCnt;
}	st_fifoStream_t;

extern st_fifoStream_t g_stFifoStream[FIFO_MAX_STREAM];

#endif	//__MY_FIFO_MODULE_H__
