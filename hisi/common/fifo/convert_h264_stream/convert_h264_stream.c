#include "convert_h264_stream.h"
#include "../fifo.h"

FIFO_CALLBACK fifo_h264_CallBack[4]={0};

void *convert_h264_stream(void * cvt)
{
	int iHandle;
	int i=(int)cvt;
	st_fifo_t stFifo;	
	stFifo.iGrp = g_stFifoStream[i].iGrp;
	stFifo.iChn = g_stFifoStream[i].iChn;
	printf("%s:%d iGrp %d iChn %d\n",__FUNCTION__,__LINE__,stFifo.iGrp,stFifo.iChn);
	stFifo.enType = FIFO_TYPE_MEDIA;//g_stFifoStream[i].enType;
	int iFifo = FIFO_Request(stFifo,&iHandle,0);
	
	int iGlen=0;
	int iPoint = 0;
	unsigned short usframeCur=0;
	unsigned short usframeCmp=0;
	int  hasIDR=0;
	char cBuf[4096];
	char * cNalu=malloc(0x60000);
    while(g_stFifoStream[i].iConvertThread == 1)
	{
		iGlen = FIFO_Read(iHandle,iFifo,cBuf);
		if (iGlen > 11)
		{
			if (cBuf[1] == 0x39)
			{
//				if (stFifo.iChn == 0)
//					printf("%s:%d iGrp %d iChn %d\n",__FUNCTION__,__LINE__,stFifo.iGrp,stFifo.iChn);
				if ((cBuf[27]&0x1f) == 7)
				{
					if (hasIDR == 0)
					{
						memcpy(&usframeCmp,&cBuf[11],2);
						usframeCur = usframeCmp;
						iPoint = 0;
					}
					hasIDR  = 1;
				}
				
				if (hasIDR == 1)
				{
					memcpy(&usframeCur,&cBuf[11],2);
					if (usframeCur == usframeCmp)
					{
						if ((cBuf[20]&0x80) != 0)
						{
							if ((iPoint+iGlen-27)>0x60000)
								printf("ERROR:%s:%d\n",__FUNCTION__,__LINE__);
							else
							{
								memcpy(cNalu+iPoint,&cBuf[27],iGlen-27);
								iPoint += iGlen-27;
							}
						}
						else
						{
							if ((iPoint+iGlen-23)>0x60000)
								printf("ERROR:%s:%d\n",__FUNCTION__,__LINE__);
							else
							{
								memcpy(cNalu+iPoint,&cBuf[23],iGlen-23);
								iPoint += iGlen-23;
							}
						}
					}
					else
					{
						if (fifo_h264_CallBack[stFifo.iChn] != 0)
							fifo_h264_CallBack[stFifo.iChn](cNalu,iPoint);
						if ((cBuf[20]&0x80) != 0)
						{
							memcpy(cNalu,&cBuf[27],iGlen-27);
							iPoint = iGlen-27;
						}
						else
						{
							memcpy(cNalu,&cBuf[23],iGlen-23);
							iPoint = iGlen-23;
						}
						usframeCmp = usframeCur;
					}
				}
			}				
		}
		else
			usleep(10000);
	}
	FIFO_Release(iHandle,iFifo);
	if (cNalu != NULL)
		free(cNalu);
	//fifo_h264_CallBack[stFifo.iChn] = 0;
	return cvt;
}

void *convert_audio_stream(void * cvt)
{
	int iHandle;
	int i=(int)cvt;
	st_fifo_t stFifo;	
	stFifo.iGrp = g_stFifoStream[i].iGrp;
	stFifo.iChn = 2;//g_stFifoStream[i].iChn;//获取扩展通道的音频，因为扩展通道数据量小，这个线程不会占太多的资源
	stFifo.enType = FIFO_TYPE_MEDIA;//g_stFifoStream[i].enType;
	printf("%s:%d iGrp %d iChn %d\n",__FUNCTION__,__LINE__,stFifo.iGrp,stFifo.iChn);
	int iFifo = FIFO_Request(stFifo,&iHandle,0);
	
	int iGlen=0;
	char cBuf[2048];
	while(g_stFifoStream[i].iConvertThread == 1)
	{
		iGlen = FIFO_Read(iHandle,iFifo,cBuf);
		if (iGlen > 11)
		{
			if (cBuf[1] == 0x38)
			{
				if (fifo_h264_CallBack[3] != 0)
					fifo_h264_CallBack[3](&cBuf[27],iGlen-27);
			}
		}
		else
			usleep(10000);
	}
	FIFO_Release(iHandle,iFifo);
	//fifo_h264_CallBack[3] = 0;
	return cvt;
}


