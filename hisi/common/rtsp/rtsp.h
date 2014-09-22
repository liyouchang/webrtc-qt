#ifndef __MY_RTSP_MODULE_H__
#define __MY_RTSP_MODULE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../common_api.h"
#include "../common_define.h"

typedef struct rtsp_info
{
	int  iSock;
	char cRtpInfo[128];
}	st_rtsp_info_t;

#endif	//__MY_RTSP_MODULE_H__

