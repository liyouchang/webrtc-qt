#ifndef __MY_CONVERT_H264_H__
#define __MY_CONVERT_H264_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "../../common_define.h"

extern FIFO_CALLBACK fifo_h264_CallBack[4];
extern void *convert_h264_stream(void * cvt);
extern void *convert_audio_stream(void * cvt);

#endif	//__MY_CONVERT_H264_H__
