#ifndef __MY_CONVERT_RTP_H__
#define __MY_CONVERT_RTP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "../../common_define.h"

typedef struct ST_RTP_HEAD
{
	unsigned int cc : 4;
	unsigned int x : 1;
	unsigned int p : 1;
	unsigned int ver : 2;
	unsigned int pt : 7;
	unsigned int m : 1;
	unsigned int seq : 16;
	unsigned int ts;
	unsigned int ssrc;
	
}__attribute__((packed))st_rtp_head_t;

typedef struct ST_RTSP_HEAD
{
	unsigned char symbol;
	unsigned char chan;
	unsigned short ilen;
	
}__attribute__((packed))st_rtsp_head_t;

extern void *convert_rtp_stream(void * cvt);

#endif	//__MY_CONVERT_RTP_H__
