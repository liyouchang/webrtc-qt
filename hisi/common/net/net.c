#include "net.h"
#include "../common_define.h"
#include "../config/config_dev/dev.h"
#include "../config/config_net/net.h"

struct TCP_SERVER g_TcpServer[MAX_TCP_SERVER];
struct TCP_CLIENT g_TcpClient[MAX_TCP_CLIENT];
struct UDP_SERVER g_UdpServer[MAX_UDP_SERVER];

int NET_Initialize(void)
{
	int i,j;
	for (i=0; i<MAX_TCP_SERVER; i++)
	{
		g_TcpServer[i].iSock = -1;
		g_TcpServer[i].iEnable = 0;
		for (j=0; j<MAX_TCP_CLIENT; j++)
		{
			g_TcpServer[i].link[j].iSock = -1;
			g_TcpServer[i].link[j].iEnable = 0;
			g_TcpServer[i].link[j].receiveCallBack = 0;
			g_TcpServer[i].link[j].protocolCallBack= 0;		
		}
	}
	for (i=0; i<MAX_TCP_CLIENT; i++)
	{
		g_TcpClient[i].iSock = -1;
		g_TcpClient[i].iEnable = 0;
		g_TcpClient[i].receiveCallBack = 0;
		g_TcpClient[i].protocolCallBack= 0;		
	}
	for (i=0; i<MAX_UDP_SERVER; i++)
	{
		g_UdpServer[i].iSock = -1;
		g_UdpServer[i].iEnable = 0;
		g_UdpServer[i].protocolCallBack= 0;	
	}
	NET_Manage_Open();
	return 0;
}
int NET_Get_RouteIP(char * cip)
{
	st_net_status_t st_staus;
    NET_Get_Status(&st_staus);
    char  localIp[4]={0};
    int iip = 0;

    if(st_staus.lan.iIP != -1)
    {
        memcpy(localIp,&st_staus.lan.iIP,4);   
        iip = st_staus.lan.iIP;
    }    
    else if (st_staus.wifi.iIP != -1)
    {
        memcpy(localIp,&st_staus.wifi.iIP,4);
        iip = st_staus.wifi.iIP;
    }
    else if (st_staus.adsl.iIP != -1)
    {
        memcpy(localIp,&st_staus.adsl.iIP,4);
        iip = st_staus.adsl.iIP;
    }

    if (cip != NULL)
        sprintf(cip, "%d.%d.%d.%d",localIp[0],localIp[1],localIp[2],localIp[3]);
    return iip;
}
int NET_Cleanup(void)
{
	NET_Manage_Close();
	return 0;
}

int TCP_Server_Create(short sPort,int iMaxConn,int iTimeout)
{
	int i;
	for (i=0; i<MAX_TCP_SERVER; i++)
	{
		if ((g_TcpServer[i].iSock<0) && (g_TcpServer[i].iEnable==0))
		{
			g_TcpServer[i].iEnable = 1;
			break;
		}	
	}
	if (i >= MAX_TCP_SERVER)
	{
		printf("can not create tcp server because no resource\n");
		return -1;
	}
	
	signal(SIGPIPE,SIG_IGN);
	g_TcpServer[i].iSock = socket(PF_INET,SOCK_STREAM,0);
	if (g_TcpServer[i].iSock < 0)
	{
		g_TcpServer[i].iEnable = 0;
		perror("TCP_Server_Create:create socket error");
		return -1;
	}
	int rb = 1;
	int ret = setsockopt(g_TcpServer[i].iSock,SOL_SOCKET,SO_REUSEADDR,&rb,sizeof(rb));
	if (ret != 0)
		printf("TCP_Server_Create: setsockopt return %d\n",ret);
	struct sockaddr_in addr;
	addr.sin_family      = PF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(sPort);
	if (bind(g_TcpServer[i].iSock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
	{
		printf("TCP_Server_Create:bind port %d error\n",sPort);
SERVER_CREATE_ERROR:		
		perror("\n");
		g_TcpServer[i].iEnable = 0;
		close(g_TcpServer[i].iSock);
		g_TcpServer[i].iSock = -1;
		return -1;
	}
	g_TcpServer[i].iMaxLink = iMaxConn;
	if (listen(g_TcpServer[i].iSock,g_TcpServer[i].iMaxLink) < 0)
	{
		printf("TCP_Server_Create:listen sock %d max %d link error",g_TcpServer[i].iSock,g_TcpServer[i].iMaxLink);
		goto SERVER_CREATE_ERROR;
	}
	g_TcpServer[i].iLinkTimeOut = iTimeout;
	return i;
}

void *tcp_server_send_thread(void *snd)
{
	struct TCP_CLIENT * link = (struct TCP_CLIENT *)snd;
	printf("%s:%d link %p\n",__FUNCTION__,__LINE__,link);
	int iCnt=0;
	int iWlen=0;
	int iTimeCnt=0;
	int sendPoint = 0;
	while(link->iRcvThread == 1)
	{
		if (link->iWrite != link->iRead)
		{
			if (link->stData[link->iRead].iSndLen != 0)
			{
				link->iTimeCnt = 0;
				iWlen = sendto(link->iSock,link->stData[link->iRead].pSndData+sendPoint,link->stData[link->iRead].iSndLen,0,NULL,0);
				if (iWlen > 0)
				{
					link->stData[link->iRead].iSndLen -= iWlen;
					sendPoint += iWlen;
					if (link->stData[link->iRead].iSndLen == 0)
					{
						free(link->stData[link->iRead].pSndData);
						link->stData[link->iRead].pSndData= NULL;						
					}
				}
				else
				{
					if (errno != 11)
					{
					    printf("errno=%d\n",errno);
					    perror("tcp_server_send_thread 1 failed!:");
					    link->iRcvThread = 0;
					    break;
					}
					else
					{
					    iCnt++;
					    if (iCnt >= 5)
					    {
					        iCnt = 0;
					        usleep(100000);
							iTimeCnt++;
					        if (iTimeCnt >= 10)
							{
								printf("errno=%d\n",errno);
					    		perror("tcp_server_send_thread 2 failed!:");
					    		link->iRcvThread = 0;
					    		break;
							}
					    }
					}
				}
			}
			if (link->stData[link->iRead].iSndLen == 0)
			{
				link->iRead++;
				if (link->iRead >= MAX_TCP_SENDNUM)
					link->iRead = 0;
				iCnt = 0;
				iTimeCnt = 0;
				sendPoint = 0;
			}
		}
		else
			usleep(10000);
	}
	return snd;
}
void *tcp_server_receive(void * rcv)
{
	struct TCP_CLIENT * link = (struct TCP_CLIENT *)rcv;
	printf("%s:%d link %p\n",__FUNCTION__,__LINE__,link);
	usleep(40000);
	
	link->iRead = 0;
	link->iWrite = 0;
	link->iTimeCnt=0;
	pthread_t thread_send;
	pthread_create(&thread_send,NULL,tcp_server_send_thread,link);
	usleep(40000);
	while(link->iRcvThread == 1)
	{
		if (link->receiveCallBack != 0)
		{
			link->iRlen = link->receiveCallBack(link->iSock,link->pRcvData,link->iRcvThread);
			if (link->iRlen >= 0)
			{
				link->iTimeCnt = 0;
				if (link->protocolCallBack != 0)
				{	
					if (link->protocolCallBack(link->iSock,link->pRcvData,link->iRlen) < 0)
						link->iRcvThread = 0;
				}
				else
					printf("调用接口缺少回调函数protocolCallBack的定义\n");
			}
			else if (link->iRlen == -1)
				link->iTimeCnt++;
			else if (link->iRlen == -2)
			{
				link->iRcvThread = 0;
				printf("link->iSock %d接收到对方断开\n",link->iSock);
			}
			if (link->iTimeCnt > link->iTimeOut)
			{
				link->iRcvThread = 0;
				printf("link->iSock %d超时断开\n",link->iSock);
			}
		}
		else
		{	
			sleep(1);
			printf("TCP调用接口缺少回调函数receiveCallBack的定义\n");
		}
	}
	pthread_join(thread_send,NULL);
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	return rcv;
}
int TCP_Server_Wait(int handle,int *pSock,int iRcvSize,NET_RECEIVE_CALLBACK receive,NET_PROTOCOL_CALLBACK protocol)
{
	socklen_t iLen;
	struct sockaddr_in addr;
	iLen = sizeof(struct sockaddr_in);
	
	struct timeval timeout={1,0};
	setsockopt(g_TcpServer[handle].iSock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	
	int iTmpSock = accept(g_TcpServer[handle].iSock,(struct sockaddr*)&addr,&iLen);
	if (iTmpSock < 0)
		return -1;
	int i;
	for (i=0; i<g_TcpServer[handle].iMaxLink; i++)
	{
		if ((g_TcpServer[handle].link[i].iEnable==0) && (g_TcpServer[handle].link[i].iSock<0))
		{	
			g_TcpServer[handle].link[i].iEnable = 1;
			break;
		}
	}
	g_TcpServer[handle].link[i].iSock = iTmpSock;
	*pSock = iTmpSock;
	inet_aton(inet_ntoa(addr.sin_addr),&addr.sin_addr);
	g_TcpServer[handle].link[i].iSourceIP = addr.sin_addr.s_addr;
	int size=0xffff;
	setsockopt(g_TcpServer[handle].link[i].iSock,SOL_SOCKET,SO_SNDBUF,&size ,sizeof(size));
	setsockopt(g_TcpServer[handle].link[i].iSock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	setsockopt(g_TcpServer[handle].link[i].iSock,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	
	g_TcpServer[handle].link[i].receiveCallBack = receive;
	g_TcpServer[handle].link[i].protocolCallBack= protocol;
	g_TcpServer[handle].link[i].iRlen    = 0;
	g_TcpServer[handle].link[i].iMaxRcv  = iRcvSize;
	g_TcpServer[handle].link[i].pRcvData = malloc(iRcvSize);
	if (g_TcpServer[handle].link[i].pRcvData == NULL)
	{
		perror("TCP_Server_Wait:malloc error\n");
TCP_WAIT_ERROR:		
		close(g_TcpServer[handle].link[i].iSock);
		g_TcpServer[handle].link[i].iSock = -1;
		g_TcpServer[handle].link[i].iEnable =0;
		g_TcpServer[handle].link[i].receiveCallBack = 0;
		g_TcpServer[handle].link[i].protocolCallBack= 0;
		return -1;
	}
	
	int j;
	for (j=0; j<MAX_TCP_SENDNUM; j++)
	{
		g_TcpServer[handle].link[i].stData[j].pSndData = NULL;
		g_TcpServer[handle].link[i].stData[j].iSndLen = 0;
	}
	g_TcpServer[handle].link[i].iRcvThread = 1;
	g_TcpServer[handle].link[i].iTimeOut = g_TcpServer[handle].iLinkTimeOut;
	if (pthread_create(&g_TcpServer[handle].link[i].thread_recv,NULL,tcp_server_receive,(void *)&g_TcpServer[handle].link[i]) != 0)
	{
		perror("tcp_server_wait receive thread error");
		free(g_TcpServer[handle].link[i].pRcvData);
		g_TcpServer[handle].link[i].pRcvData = NULL;
		goto TCP_WAIT_ERROR;
	}
	return i;
}
int TCP_Server_Send(int handle,int link,char *pData,int iLen)
{
	if (g_TcpServer[handle].link[link].iRcvThread == 0)
		return -1;
	int iTmp = g_TcpServer[handle].link[link].iWrite;
	int iNext = iTmp+1;
	if (iNext >= MAX_TCP_SENDNUM)
		iNext = 0;
	if (iNext==g_TcpServer[handle].link[link].iRead)
		usleep(10000);
	if (iNext==g_TcpServer[handle].link[link].iRead)
	{	
		g_TcpServer[handle].link[link].iRead =iTmp;
		printf("TCP_Server_Send:send so slowly\n");
	}
	g_TcpServer[handle].link[link].stData[iTmp].iSndLen = iLen;
	if (g_TcpServer[handle].link[link].stData[iTmp].pSndData != NULL)
		free(g_TcpServer[handle].link[link].stData[iTmp].pSndData);
	g_TcpServer[handle].link[link].stData[iTmp].pSndData = malloc(iLen);
	memcpy(g_TcpServer[handle].link[link].stData[iTmp].pSndData,pData,iLen);
	g_TcpServer[handle].link[link].iWrite = iNext;
	return iLen;
}
int TCP_Server_GetLinkIP(int handle,int link,int ip)
{
	return g_TcpServer[handle].link[link].iSourceIP;
}
int TCP_Server_Finish(int handle,int link)
{
	if (g_TcpServer[handle].link[link].iEnable == 1)
	{	
		if (g_TcpServer[handle].link[link].iRcvThread == 0)
		{
			pthread_join(g_TcpServer[handle].link[link].thread_recv,NULL);
			if (g_TcpServer[handle].link[link].pRcvData != NULL)
				free(g_TcpServer[handle].link[link].pRcvData);
			g_TcpServer[handle].link[link].pRcvData = NULL;
			if (g_TcpServer[handle].link[link].iSock >= 0)
				close(g_TcpServer[handle].link[link].iSock);
			g_TcpServer[handle].link[link].iSock = -1;
			g_TcpServer[handle].link[link].iEnable = 0;
			int i;
			for (i=0; i<MAX_TCP_SENDNUM; i++)
			{
				if (g_TcpServer[handle].link[link].stData[i].pSndData != NULL)
				{
					free(g_TcpServer[handle].link[link].stData[i].pSndData);
					g_TcpServer[handle].link[link].stData[i].pSndData = NULL;
					g_TcpServer[handle].link[link].stData[i].iSndLen = 0;
				}
			}	
			return link;
		}
	}
	
	return -1;
}
int TCP_Server_Destory(int handle)
{
	int i;
	for (i=0; i<g_TcpServer[handle].iMaxLink; i++)
	{
		if (g_TcpServer[handle].link[i].iEnable == 1)
		{	
			if (g_TcpServer[handle].link[i].iRcvThread == 1)
			{
				g_TcpServer[handle].link[i].iRcvThread = 0;
				pthread_join(g_TcpServer[handle].link[i].thread_recv,NULL);
			}
			if (g_TcpServer[handle].link[i].pRcvData != NULL)
				free(g_TcpServer[handle].link[i].pRcvData);
			g_TcpServer[handle].link[i].pRcvData = NULL;
			if (g_TcpServer[handle].link[i].iSock >= 0)
				close(g_TcpServer[handle].link[i].iSock);
			g_TcpServer[handle].link[i].iSock = -1;
			g_TcpServer[handle].link[i].iEnable= 0;
			int j;
			for (j=0; j<MAX_TCP_SENDNUM; j++)
			{
				if (g_TcpServer[handle].link[i].stData[j].pSndData != NULL)
				{
					free(g_TcpServer[handle].link[i].stData[j].pSndData);
					g_TcpServer[handle].link[i].stData[j].pSndData = NULL;
					g_TcpServer[handle].link[i].stData[j].iSndLen = 0;
				}
			}	
		}
	}
	g_TcpServer[handle].iEnable = 0;
	close(g_TcpServer[handle].iSock);
	g_TcpServer[handle].iSock = -1;
	
	return 0;
}

int TCP_Client_Create(int iIp,short sPort,int iTimeout,int iRcvSize,NET_RECEIVE_CALLBACK receive,NET_PROTOCOL_CALLBACK protocol)
{
	if ((iIp==0xffffffff) || (iIp==-1))
        return -1;
	if ((sPort==0xffff) || (sPort==-1))
        return -1;
	int i;
	for (i=0; i<MAX_TCP_CLIENT; i++)
	{
		if ((g_TcpClient[i].iEnable==0) && (g_TcpClient[i].iSock<0))
		{
			g_TcpClient[i].iEnable = 1;
			break;
		}
	}
	if (i >= MAX_TCP_CLIENT)
		return -1;
		
	g_TcpClient[i].iSock = socket(AF_INET,SOCK_STREAM,0);
	if (g_TcpClient[i].iSock < 0)
	{
		g_TcpClient[i].iEnable = 0;
		perror("TCP_Client_Create:create socket error");
		return -1;
	}
	
	struct timeval timeout={5,0};
	setsockopt(g_TcpClient[i].iSock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	setsockopt(g_TcpClient[i].iSock,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(sPort);
	addr.sin_addr.s_addr = iIp;
	printf("iIp %08x sPort %d iSock %d start connect\n",iIp,sPort,g_TcpClient[i].iSock);
	if (connect(g_TcpClient[i].iSock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
	{
		perror("tcp_client_create:create connect error");
TCP_CLIENT_ERROR:
		close(g_TcpClient[i].iSock);
		g_TcpClient[i].iSock = -1;
		g_TcpClient[i].iEnable = 0;
		return -1;
	}
	printf("connect iIp %08x sPort %d success\n",iIp,sPort);
	
	int size = 0x10000;
	setsockopt(g_TcpClient[i].iSock,SOL_SOCKET,SO_RCVBUF,&size ,sizeof(size));
	setsockopt(g_TcpClient[i].iSock,SOL_SOCKET,SO_SNDBUF,&size ,sizeof(size));
	timeout.tv_sec = 1;
	setsockopt(g_TcpClient[i].iSock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	g_TcpClient[i].receiveCallBack = receive;
	g_TcpClient[i].protocolCallBack= protocol;
	g_TcpClient[i].iRlen	= 0;
	g_TcpClient[i].iMaxRcv  = iRcvSize;
	g_TcpClient[i].pRcvData = malloc(iRcvSize);
	if (g_TcpClient[i].pRcvData == NULL)
	{
		perror("TCP_Client_Create:malloc error\n");
		g_TcpClient[i].receiveCallBack = 0;
		g_TcpClient[i].protocolCallBack= 0;
		goto TCP_CLIENT_ERROR;
	}
	int j;
	for (j=0; j<MAX_TCP_SENDNUM; j++)
	{
		g_TcpClient[i].stData[j].pSndData = NULL;
		g_TcpClient[i].stData[j].iSndLen = 0;
	}	
	pthread_mutex_init(&g_TcpClient[i].mutex,NULL);
	g_TcpClient[i].iRcvThread = 1;
	g_TcpClient[i].iTimeOut = iTimeout;
	if (pthread_create(&g_TcpClient[i].thread_recv,NULL,tcp_server_receive,(void *)&g_TcpClient[i]) != 0)
	{
		perror("tcp_server_wait receive thread error");
		free(g_TcpClient[i].pRcvData);
		g_TcpClient[i].pRcvData = NULL;
		g_TcpClient[i].receiveCallBack = 0;
		g_TcpClient[i].protocolCallBack= 0;
		goto TCP_CLIENT_ERROR;
	}
	usleep(100000);
	return g_TcpClient[i].iSock;
}
int TCP_Client_BindPHY(int iSock,char * phy)
{
	if (setsockopt(iSock,SOL_SOCKET,SO_BINDTODEVICE,phy,sizeof(phy)) < 0)
		return -1;
	return 0;
}
int TCP_Client_Send(int iSock,char *pData,int iLen)
{
	int handle;
	for (handle=0; handle<MAX_TCP_CLIENT; handle++)
	{
		if (g_TcpClient[handle].iSock >= 0)
			printf("g_TcpClient[handle].iSock %d iSock %d\n",g_TcpClient[handle].iSock,iSock);
		if (g_TcpClient[handle].iSock == iSock)
			goto CLIENT_SOCKET_OK;
	}
	return -1;
	
CLIENT_SOCKET_OK:	
	pthread_mutex_lock(&g_TcpClient[handle].mutex);
	int iTmp = g_TcpClient[handle].iWrite;
	int iNext = iTmp+1;
	if (iNext >= MAX_TCP_SENDNUM)
		iNext = 0;
	if (iNext==g_TcpClient[handle].iRead)
		usleep(10000);
	if (iNext==g_TcpClient[handle].iRead)
	{	
		g_TcpClient[handle].iRead =iTmp;
		printf("TCP_Client_Send:send so slowly\n");
	}
	g_TcpClient[handle].stData[iTmp].iSndLen = iLen;
	if (g_TcpClient[handle].stData[iTmp].pSndData != NULL)
		free(g_TcpClient[handle].stData[iTmp].pSndData);
	g_TcpClient[handle].stData[iTmp].pSndData = malloc(iLen);
	memcpy(g_TcpClient[handle].stData[iTmp].pSndData,pData,iLen);
	g_TcpClient[handle].iWrite = iNext;
	pthread_mutex_unlock(&g_TcpClient[handle].mutex);
	return iLen;
}
int TCP_Client_Destory(int iSock)
{
	int handle;
	for (handle=0; handle<MAX_TCP_CLIENT; handle++)
	{
		if (g_TcpClient[handle].iSock == iSock)
			goto DESTROY_SOCKET_OK;
	}
	return -1;
	
DESTROY_SOCKET_OK:	
	{
		if (g_TcpClient[handle].iRcvThread == 1)
		{
			g_TcpClient[handle].iRcvThread = 0;
			pthread_join(g_TcpClient[handle].thread_recv,NULL);
		}
		if (g_TcpClient[handle].pRcvData != NULL)
			free(g_TcpClient[handle].pRcvData);
		g_TcpClient[handle].pRcvData = NULL;
		g_TcpClient[handle].receiveCallBack = 0;
		g_TcpClient[handle].protocolCallBack= 0;
		if (g_TcpClient[handle].iSock >= 0)
			close(g_TcpClient[handle].iSock);
		pthread_mutex_destroy(&g_TcpClient[handle].mutex);
		g_TcpClient[handle].iSock = -1;
		g_TcpClient[handle].iEnable = 0;		
	}
	return 0;
}

void *udp_server_receive(void * udp)
{
	struct UDP_SERVER * link = (struct UDP_SERVER *)udp;
	socklen_t addrlen;
	struct sockaddr_in udpaddr;
	while(link->iRcvThread == 1)
	{
		bzero(&udpaddr, sizeof(udpaddr));
    	addrlen = sizeof(struct sockaddr_in);
    	link->iRlen = recvfrom(link->iSock,link->pRcvData,link->iMaxRcv,0,(struct sockaddr *)&udpaddr,&addrlen);
    	if (link->iRlen > 0)
    	{
    		if (link->iRlen < link->iMaxRcv)
                link->pRcvData[link->iRlen] = 0;
            else
                link->pRcvData[link->iRlen-1] = 0;
    		
    		if (link->protocolCallBack != 0)
    		    link->protocolCallBack(udpaddr.sin_addr.s_addr,(short)udpaddr.sin_port,link->iSock,link->pRcvData,link->iRlen);
	    }
	}
	return udp;
}
int UDP_Server_Create(short sPort,int iRcvSize,UDP_PROTOCOL_CALLBACK protocol)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	int i;
	for (i=0; i<MAX_UDP_SERVER; i++)
	{
		if ((g_UdpServer[i].iSock<0) && (g_UdpServer[i].iEnable==0))
		{
			g_UdpServer[i].iEnable = 1;
			break;
		}	
	}
	if (i >= MAX_UDP_SERVER)
	{
		printf("can not create udp server because no resource\n");
		return -1;
	}
	
	g_UdpServer[i].iSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (g_UdpServer[i].iSock < 0)
	{
		perror("UDP_Server_Create:create socket error");
		g_UdpServer[i].iEnable = 0;
		return -1;
	}	
	int temp=1;
	setsockopt(g_UdpServer[i].iSock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int));
	setsockopt(g_UdpServer[i].iSock,SOL_SOCKET,SO_BROADCAST,&temp,sizeof(int));
	struct timeval timeout={1,0};
	setsockopt(g_UdpServer[i].iSock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	
	struct sockaddr_in udpaddr;
	bzero(&udpaddr,sizeof(udpaddr));
	udpaddr.sin_family = AF_INET;
	udpaddr.sin_port = htons(sPort);
	udpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(g_UdpServer[i].iSock,(struct sockaddr *)&udpaddr,sizeof(udpaddr)) < 0)
	{
		perror("UDP_Server_Create:bind socket error");
UDP_SERVER_ERROR:
		close(g_UdpServer[i].iSock);
		g_UdpServer[i].iSock = -1;
		g_UdpServer[i].iEnable= 0;
		return -1;
	}
	
	g_UdpServer[i].iMaxRcv = iRcvSize;
	g_UdpServer[i].pRcvData = malloc(g_UdpServer[i].iMaxRcv);
	if (g_UdpServer[i].pRcvData == NULL)
	{
		perror("UDP_Server_Create:malloc error");
		goto UDP_SERVER_ERROR;
	}
	
	pthread_mutex_init(&g_UdpServer[i].mutex,NULL);
	g_UdpServer[i].iRcvThread = 1;
	g_UdpServer[i].protocolCallBack= protocol;
	if (pthread_create(&g_UdpServer[i].thread_recv,NULL,udp_server_receive,(void *)&g_UdpServer[i]) != 0)
	{
		perror("UDP_Server_Create:create thread error");
		free(g_UdpServer[i].pRcvData);
		g_UdpServer[i].pRcvData =NULL;
		g_UdpServer[i].protocolCallBack= 0;
		goto UDP_SERVER_ERROR;
	}
	usleep(100000);
	return g_UdpServer[i].iSock;
}
int UDP_Server_Send(int iSock,int iIP,short sPort,char *pData,int iLen)
{
	int handle;
	for (handle=0; handle<MAX_UDP_SERVER; handle++)
	{
		if (g_UdpServer[handle].iSock == iSock)
			goto SEND_UDP_SOCKET_OK;
	}
	return -1;
	
SEND_UDP_SOCKET_OK:	
	pthread_mutex_lock(&g_UdpServer[handle].mutex);
	
	int nRet;
	struct sockaddr_in udpaddr;
	bzero(&udpaddr,sizeof(udpaddr));
	udpaddr.sin_family = AF_INET;
	udpaddr.sin_port = sPort;
	udpaddr.sin_addr.s_addr = iIP;
//	printf("ip=%08x port=%d len=%d\n",iIP,sPort,iLen);
//	int i;
//	for (i=0; i<iLen;i++)
//        printf("%02x ",pData[i]);
//    printf("\n");
	nRet=sendto(iSock,pData,iLen,0,(struct sockaddr *)&udpaddr,sizeof(udpaddr));
	pthread_mutex_unlock(&g_UdpServer[handle].mutex);
    return nRet;
}
int UDP_Server_Destory(int iSock)
{
	int handle;
	for (handle=0; handle<MAX_TCP_CLIENT; handle++)
	{
		if (g_TcpClient[handle].iSock == iSock)
			goto DESTROY_UDP_OK;
	}
	return -1;
	
DESTROY_UDP_OK:	
	if (g_UdpServer[handle].iRcvThread == 1)
	{
		g_UdpServer[handle].iRcvThread = 0;
		pthread_join(g_UdpServer[handle].thread_recv,NULL);
	}
	if (g_UdpServer[handle].pRcvData != NULL)
		free(g_UdpServer[handle].pRcvData);
	g_UdpServer[handle].pRcvData = NULL;
	g_UdpServer[handle].protocolCallBack= 0;
	if (g_UdpServer[handle].iSock >= 0)
		close(g_UdpServer[handle].iSock);
	pthread_mutex_destroy(&g_UdpServer[handle].mutex);
	g_UdpServer[handle].iSock = -1;
	g_UdpServer[handle].iEnable= 0;
	return 0;
}

int NET_Get_WifiList(int iMaxList,st_wifi_list_t *pList)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
    int iNumList= 0;
    char * cFile = malloc(1280*iMaxList);
    if (cFile == NULL)
    {
    	printf("in NET_Get_WifiList() 分配内存失败\n");
    	return 0;
    }
	struct DEVHARDPARAM dev;
	devFileGet(&dev);
	struct NETPARAM net;
	netFileGet(&net);
	
	char cCmd[128];
    sprintf(cCmd,"ifconfig %s up", dev.wifiEth);
	if (system(cCmd) >= 0)
	{
		sprintf(cCmd,"iwlist %s scanning > /tmp/AP_List.ini_bak", dev.wifiEth);
        if (system(cCmd) >= 0)
        {
			FILE * apList = fopen("/tmp/AP_List.ini_bak","r");
			if (apList != NULL)
			{
				int iLen = fread(cFile,1,1280*iMaxList,apList);
				fclose(apList);
				if (iLen >= 1280*iMaxList)
					cFile[1280*iMaxList-1]=0;
				else if (iLen <= 0)
					cFile[0]=0;
				else
					cFile[iLen]=0;
				remove("/tmp/AP_List.ini_bak");				//读出AP列表

				char *m_WIFI = strstr(cFile, "Cell ");		//当前一个Cell
				if (m_WIFI != NULL)
				{
					char *msg=NULL;
					while (m_WIFI != NULL)
					{										//找下一个Cell
						char *n_WIFI = strstr(m_WIFI+5, "Cell ");
						if (n_WIFI != NULL)					//找到了
						{
							*(n_WIFI-1)=0;
							msg = malloc(n_WIFI-m_WIFI);
							if (msg != NULL)				//取出一整条Cell
								memcpy(msg,m_WIFI,n_WIFI-m_WIFI);
							m_WIFI = n_WIFI;
						}
						else
						{
							msg = malloc(strlen(m_WIFI)+1);	//取出一整条Cell,最后一条
							if (msg != NULL)
								strcpy(msg,m_WIFI);
							m_WIFI = NULL;
						}
						if (msg != NULL)
							printf("%s:%d.....\n%s\n",__FUNCTION__,__LINE__,msg);
						else
							break;
						//=============================读出一条Cell 开始取信息==============================//	
						char * pEnd=NULL;					//取SSID
						char * pSSID=strstr(msg,"ESSID");
						if (pSSID != NULL)
						{
							memset(pList->ssid,0,32);
							pEnd = strchr(pSSID+7,'"');
							if (pEnd != NULL)
							{
								if (pEnd-(pSSID+7) != 0)
									memcpy(pList->ssid,pSSID+7,pEnd-(pSSID+7));
								printf("the AP SSID is %s\n",pList->ssid);
							}
							else
							{
								printf("未找到ESSID,退出查询\n");
								break;
							}	
						}
						else
						{
							printf("未找到ESSID,退出查询\n");
							if (msg != NULL)
								free(msg);
							break;
						}
						if(!strcmp(pList->ssid,(char *)net.wifiSSID))
							pList->enable = 1;
						else
							pList->enable = 0;

						char *plevel = strstr(msg,"level");	//找信号强度值
						if(plevel != NULL)
						{
							pEnd = strchr(plevel+6,'/');
							char tmp[128];
							if (pEnd-(plevel+6) != 0)
								memcpy(tmp,plevel+6,pEnd-(plevel+6));							
							pList->signalStrength = atoi(tmp);
							printf("the signal level is %d\n",pList->signalStrength);
						}
						
						char *pKey = strstr(msg,"key:");
						if (pKey != NULL)
						{
							if (!memcmp(pKey+4,"on",2))
							{
								pList->encryptMode = WEP;
								pKey += 6;
								char *aes  = NULL;
								char *tkip = NULL;						//加密类型
								char ctkip[512]= {0};
								if ((NULL!=strstr(pKey,"rsn_ie")) || (NULL!=strstr(pKey,"WPA2 Version")))
								{
									pList->encryptMode = WPA2;
									tkip = strstr(pKey,"WPA2 Version");
									if (tkip == NULL)
										tkip=strstr(pKey,"rsn_ie");
									if (tkip != NULL)
									{
										char * ie = strstr(tkip,"IE:");
										if (ie != NULL)
										{
											if ((int)ie-(int)tkip < 512)
											{
												memcpy(ctkip,tkip,(int)ie-(int)tkip);
												ctkip[(int)ie-(int)tkip] = 0;
											}
											else
											{
												memcpy(ctkip,tkip,512);
												ctkip[511]=0;
											}
										}
										else
											strncpy(ctkip,tkip,512);
											
										tkip = strstr(ctkip,"Pairwise Ciphers");
										if (tkip != NULL)
										{
											aes = strstr(tkip,"CCMP");						//WPA2一般不支持TJIP,所以默认或者自动都设置成AES
											if (aes != NULL)
												pList->encryptFormat = AES;
											else
											{
												aes = strstr(tkip,"TKIP");
												if (aes != NULL)
													pList->encryptFormat = TKIP;
											}
										}
									}
								}
								if ((NULL!=strstr(pKey,"wpa_ie")) || (NULL!=strstr(pKey,"WPA Version 1")))
								{
									pList->encryptMode = WPA;
									tkip=strstr(pKey,"WPA Version 1");
									if (tkip == NULL)
										tkip=strstr(pKey,"wpa_ie");										
									if (tkip != NULL)
									{
										char * ie = strstr(tkip,"IE:");
										if (ie != NULL)
										{
											if ((int)ie-(int)tkip < 512)
											{
												memcpy(ctkip,tkip,(int)ie-(int)tkip);
												ctkip[(int)ie-(int)tkip] = 0;
											}
											else
											{
												memcpy(ctkip,tkip,512);
												ctkip[511]=0;
											}
										}
										else
											strncpy(ctkip,tkip,512);
											
										tkip = strstr(ctkip,"Pairwise Ciphers");
										if (tkip != NULL)
										{
											aes = strstr(tkip,"TKIP");
											if (aes != NULL)
												pList->encryptFormat = TKIP;
											else
											{
												aes = strstr(tkip,"CCMP");
												if (aes != NULL)
													pList->encryptFormat = AES;
											}
										}
									}
								}
							}
							else
								pList->encryptMode = NONE;
						}
						else
						{
							printf("未找到key,退出查询\n");
							if (msg != NULL)
								free(msg);
							break;
						}
						if (msg != NULL)
						{
							free(msg);
							msg = NULL;
						}

                        pList++;
                        iNumList++;
						if(iNumList >= iMaxList)
                            break;
					}
				}
				else
					printf("can not find a Cell\n");
			}
			else
				printf("open AP_List.ini_bak fail\n");
        }
        else
        	printf("iwlist wifi scanning fail\n");
	}
	else
		printf("ifconfig wifi up fail\n");

	if (cFile != NULL)
		free(cFile);
	printf("%s:%d found %d AP\n",__FUNCTION__,__LINE__,iNumList);
	return iNumList;
}

int NET_Set_Wifi(st_wifi_list_t *pWifi)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	struct NETPARAM net;
	netFileGet(&net);
	strncpy((char *)net.wifiSSID,pWifi->ssid,32);
	strncpy((char *)net.wifiCode,pWifi->key,32);
	net.wifiEnable = pWifi->enable;
	if(pWifi->encryptMode == NONE)
    	net.wifiMode = 0;
    else if(pWifi->encryptMode == WEP)
	{
		net.wifiMode = 1;
		if ((strlen(pWifi->key)==10) || (strlen(pWifi->key)==26))
			net.wifiASC = 0;
		else
			net.wifiASC = 1;
		net.wifiNum = pWifi->wepPosition;	        
	}
    else 
    {
	    if (pWifi->encryptFormat == TKIP)
	    	net.wifiNum = 2;
	    else if (pWifi->encryptFormat == AES)
	    	net.wifiNum = 1;
	    	
	    if(pWifi->encryptMode == WPA)
	    	net.wifiMode = 2;
	    else if(pWifi->encryptMode == WPA2)
	    	net.wifiMode = 3;
	}
	
    net.wifiDhcp   = 1;
	net.pppoeEnable= 0;
    netFileSet(&net);
	return 0;
}

int Net_Get_WifiLink_Status(void)
{
	return g_iWifiLinkStatus;
}
