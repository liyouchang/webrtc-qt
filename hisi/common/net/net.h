#ifndef __NETWORK_MODULE_H__
#define __NETWORK_MODULE_H__

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <signal.h>
	
#include "../common_define.h"
#include "manage/manage.h"

#define MAX_TCP_SERVER			4
#define MAX_TCP_SERVER_LINK		32
#define MAX_TCP_SENDNUM			512
#define MAX_TCP_CLIENT			12
#define MAX_UDP_SERVER			4
struct TCP_SENDBUF
{
	char * pSndData;
	int iSndLen;
};
struct TCP_CLIENT
{
	int iEnable;
	int iSock;
	int iTimeOut;
	int iSourceIP;
	
	int iRlen;
	int iMaxRcv;
	int iTimeCnt;
	char *pRcvData;
	int iRcvThread;
	pthread_t thread_recv;
	NET_RECEIVE_CALLBACK receiveCallBack;
	NET_PROTOCOL_CALLBACK protocolCallBack;
	
	int iWrite;
	int iRead;
	pthread_mutex_t mutex;
	struct TCP_SENDBUF stData[MAX_TCP_SENDNUM];
};

struct TCP_SERVER
{
	int iEnable;
	int iSock;
	int iMaxLink;
	int iLinkTimeOut;
	struct TCP_CLIENT link[MAX_TCP_SERVER_LINK];
};

struct UDP_SERVER
{
	int iEnable;
	int iSock;
	int iTimeOut;
	int iSourceIP;
	
	int iRlen;
	int iMaxRcv;
	int iTimeCnt;
	char *pRcvData;
	int iRcvThread;
	pthread_t thread_recv;
	UDP_PROTOCOL_CALLBACK protocolCallBack;
	
	pthread_mutex_t mutex;
};

#endif	//__NETWORK_MODULE_H__
