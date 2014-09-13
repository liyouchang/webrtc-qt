#include "fifo.h"

int  g_sps_len[MAXVIDEOCHNS][3];
int  g_pps_len[MAXVIDEOCHNS][3];
char sps_level[MAXVIDEOCHNS][3][128];
char pps_param[MAXVIDEOCHNS][3][128];
st_fifoInfo_t g_stFifo[FIFO_MAX_NUM];

int g_iFifoAlarm_Full =0;
int g_iFifoAlarm_Write=0;
st_alarm_upload_t g_stFifoAlarm[FIFO_ALARM_MAX_SHEET];

int FIFO_Initialize(void)
{
	int i;
	for (i=0; i<FIFO_MAX_NUM; i++)
	{
		g_stFifo[i].pBuf = NULL;
		g_stFifo[i].capacity = 0;
		g_stFifo[i].enType = -1;
		g_stFifo[i].iGrp = -1;
		g_stFifo[i].iChn = -1;
	}
	g_iFifoAlarm_Full  = 0;
	g_iFifoAlarm_Write = 0;
	return 0;
}

int FIFO_Create(st_fifo_t stFifo,int iCapacity)
{
	int i;
	for (i=0; i<FIFO_MAX_NUM; i++)
	{
		if ((g_stFifo[i].pBuf==NULL)&&(g_stFifo[i].capacity==0))
			break;
	}
	if (i >= FIFO_MAX_NUM)
	{
		printf("can't allocate free fifo\n");
		return -1;
	}
	
	iCapacity &= 0xfffffffc;
	if (iCapacity < 4096)
		iCapacity = 4096;
	g_stFifo[i].pBuf = malloc(iCapacity);
	if (g_stFifo[i].pBuf == NULL)
	{
		perror("fifo allocate memery failed:\n");
		return -1;
	}
	
	g_stFifo[i].enType		= stFifo.enType;
	g_stFifo[i].iGrp		= stFifo.iGrp;
	g_stFifo[i].iChn		= stFifo.iChn;
	g_stFifo[i].capacity	= iCapacity;
	g_stFifo[i].iDrop		= 0;
	g_stFifo[i].iWrite		= 0;
	g_stFifo[i].iRead_Base	= 0;
	printf("%s:%d handle %d enType %d iGrp %d iChn %d\n",__FUNCTION__,__LINE__,i,g_stFifo[i].enType,g_stFifo[i].iGrp,g_stFifo[i].iChn);
	pthread_mutex_init(&g_stFifo[i].lock_write,NULL);
	int j;
	for (j=0; j<FIFO_READ_NUM; j++)
	{	
		g_stFifo[i].iRead[j] = -1;
		pthread_mutex_init(&g_stFifo[i].lock_read[j],NULL);
	}
		
	return i;
}

int fifo_read_jump(int handle,int * out)
{
    int iLen;
    char buf[8];
    int offset = *out;
    if ((offset+6) > g_stFifo[handle].capacity)
    {
        memcpy(buf,g_stFifo[handle].pBuf+offset,g_stFifo[handle].capacity-offset);
        memcpy(buf+(g_stFifo[handle].capacity-offset),g_stFifo[handle].pBuf,6-(g_stFifo[handle].capacity-offset));
        memcpy((char *)&iLen,buf+2,4);
    }
    else
        memcpy((char *)&iLen,g_stFifo[handle].pBuf+offset+2,4);

    if ((iLen>=1500)||(iLen<0))
    {
        printf("fifo_read_jump:read error iLen %d\n\n",iLen);
        fflush(stdout);
        * out = g_stFifo[handle].iWrite;
        return 1;
    }

    if ((offset+iLen) > g_stFifo[handle].capacity)
    {
        * out = iLen-(g_stFifo[handle].capacity-offset);
        return 1;
    }
    else
    {
        offset += iLen;
        if (offset >= g_stFifo[handle].capacity)
        {
            printf("fifo_read_jump:fifo 转头\n");
            offset = 0;
            * out = 0;
            return 1;
        }
    }
    * out = offset;
    return 0;
}


void fifo_is_full(int handle,int iLen)
{
    if (g_stFifo[handle].iRead_Base == g_stFifo[handle].iWrite)
        return ;
        
    int oddSize;									//缓冲区剩余大小
    int out_base=g_stFifo[handle].iRead_Base;		//写指针写满缓冲区最早写的数据指针
    oddSize = g_stFifo[handle].capacity-(g_stFifo[handle].capacity+g_stFifo[handle].iWrite-g_stFifo[handle].iRead_Base)%g_stFifo[handle].capacity;
//    printf("****************iLen = %d, oddSize = %d\n", iLen, oddSize);
    if (oddSize < iLen)
    {
    	int i;
    	int  iRead[FIFO_READ_NUM];
	    char cFlag[FIFO_READ_NUM];
	    for (i=0; i<FIFO_READ_NUM; i++)
	    {
	        cFlag[i] = 0;
	        iRead[i] = g_stFifo[handle].iRead[i];
	    }
	    
	    if (g_stFifo[handle].iDrop == g_stFifo[handle].capacity)
            g_stFifo[handle].iDrop = g_stFifo[handle].capacity>>2;
        else
            g_stFifo[handle].iDrop += g_stFifo[handle].capacity>>2;
        if (g_stFifo[handle].iDrop >= g_stFifo[handle].capacity)
            g_stFifo[handle].iDrop = g_stFifo[handle].capacity;
        while (out_base < g_stFifo[handle].iDrop)
        {
            for (i=0; i<FIFO_READ_NUM; i++)
            {
                if (out_base==iRead[i])			//?????多个连接时，不同步的话可能出错
                {
                    cFlag[i] = 1;
                    printf("out_base %d read %d oddSize %d iLen %d write %d g_base %d drop %d\n",out_base,iRead[i],oddSize,iLen,g_stFifo[handle].iWrite,g_stFifo[handle].iRead_Base,g_stFifo[handle].iDrop);
                }
            }
            if (fifo_read_jump(handle,&out_base) == 1)
                break;
        }
        g_stFifo[handle].iRead_Base = out_base;
        for (i=0; i<FIFO_READ_NUM; i++)
        {
            if (cFlag[i] == 1)
            {
                pthread_mutex_lock(&g_stFifo[handle].lock_read[i]);
                if (g_stFifo[handle].iRead[i] >= 0)
                {
                    printf("handle %d read[%d] %d so slowly,change to %d write is %d\n",handle,i,g_stFifo[handle].iRead[i],g_stFifo[handle].iRead_Base,g_stFifo[handle].iWrite);
                    g_stFifo[handle].iRead[i] = out_base;
                }
                pthread_mutex_unlock(&g_stFifo[handle].lock_read[i]);
            }
        }	    
    }    
}

int FIFO_Write(int iHandle,char *pData,int iLen)
{
	if ((iLen<0) || (iLen>(g_stFifo[iHandle].capacity>>3)))
		 printf("%s:%d 出错啦!出错啦!iHandle %d iLen %d\n\n",__FUNCTION__,__LINE__,iHandle,iLen);
	if (pData[1] == 0x39)
	{
		memcpy(&g_stFifo[iHandle].iStamp,&pData[15],4);
		if ((pData[27]&0x1f) == 0x07)
		{
			memcpy(&g_sps_len[g_stFifo[iHandle].iGrp][g_stFifo[iHandle].iChn],&pData[21],2);
			memcpy(sps_level[g_stFifo[iHandle].iGrp][g_stFifo[iHandle].iChn] ,&pData[23],g_sps_len[g_stFifo[iHandle].iGrp][g_stFifo[iHandle].iChn]);
			short usLen;
			usLen = g_sps_len[g_stFifo[iHandle].iGrp][g_stFifo[iHandle].iChn]+23;
			if ((pData[usLen+27]&0x1f) == 0x08)
			{
				memcpy(&g_pps_len[g_stFifo[iHandle].iGrp][g_stFifo[iHandle].iChn],&pData[usLen+21],2);
				memcpy(pps_param[g_stFifo[iHandle].iGrp][g_stFifo[iHandle].iChn] ,&pData[usLen+23],g_pps_len[g_stFifo[iHandle].iGrp][g_stFifo[iHandle].iChn]);
			}
		}
	}
	
	pthread_mutex_lock(&g_stFifo[iHandle].lock_write);
	fifo_is_full(iHandle,iLen);
	if ((g_stFifo[iHandle].iWrite+iLen) > g_stFifo[iHandle].capacity)
    {
        memcpy(g_stFifo[iHandle].pBuf+g_stFifo[iHandle].iWrite,pData,g_stFifo[iHandle].capacity-g_stFifo[iHandle].iWrite);
		memcpy(g_stFifo[iHandle].pBuf,pData+(g_stFifo[iHandle].capacity-g_stFifo[iHandle].iWrite),iLen-(g_stFifo[iHandle].capacity-g_stFifo[iHandle].iWrite));
		g_stFifo[iHandle].iWrite = iLen-(g_stFifo[iHandle].capacity-g_stFifo[iHandle].iWrite);
    }
    else
    {
        memcpy(g_stFifo[iHandle].pBuf+g_stFifo[iHandle].iWrite,pData,iLen);
        int in = g_stFifo[iHandle].iWrite+iLen;
		if (in >= g_stFifo[iHandle].capacity)
        {
            printf("FIFO_Write:fifo 转头\n");
            in = 0;
        }
        g_stFifo[iHandle].iWrite = in;
    }	
	pthread_mutex_unlock(&g_stFifo[iHandle].lock_write);
	
	return 0;
}

void fifo_read_stamp(int handle,int rId,int * out,unsigned int *stamp)
{
    int offset = *out;
    int iLen;
    char buf[24];
    if ((offset+20) > g_stFifo[handle].capacity)
    {
        memcpy(buf,g_stFifo[handle].pBuf+offset,g_stFifo[handle].capacity-offset);
        memcpy(buf+(g_stFifo[handle].capacity-offset),g_stFifo[handle].pBuf,20-(g_stFifo[handle].capacity-offset));
        memcpy(&iLen,buf+2,4);
        memcpy((char *)stamp,buf+15,4);
    }
    else
    {
        memcpy(&iLen,g_stFifo[handle].pBuf+offset+2,4);
        memcpy((char *)stamp,g_stFifo[handle].pBuf+offset+15,4);
    }

    if ((offset+iLen) > g_stFifo[handle].capacity)
        offset = iLen-(g_stFifo[handle].capacity-offset);
    else
    {   
    	offset += iLen;
    	if (offset == g_stFifo[handle].capacity)
    		offset = 0;
    }
    * out = offset;
}

int FIFO_Request(st_fifo_t stFifo,int * pHandle,int second)
{
	int i;
	int handle;
	for (handle=0; handle<FIFO_MAX_NUM; handle++)
	{
		if ((stFifo.enType==g_stFifo[handle].enType) && (stFifo.iGrp==g_stFifo[handle].iGrp) && (stFifo.iChn==g_stFifo[handle].iChn))
			break;
	}
	if (handle >= FIFO_MAX_NUM)
		return -1;
	*pHandle = handle;
	for (i=0; i<FIFO_READ_NUM; i++)
	{
	    if (g_stFifo[handle].iRead[i] == -1)
        {
        	if (second == -1)
        	{
        		pthread_mutex_lock(&g_stFifo[handle].lock_write);
    			g_stFifo[handle].iRead[i] = g_stFifo[handle].iRead_Base;
    			pthread_mutex_unlock(&g_stFifo[handle].lock_write);
        	}
        	else if ((second==0) || (second>10))
        	{
        		pthread_mutex_lock(&g_stFifo[handle].lock_write);
    			g_stFifo[handle].iRead[i] = g_stFifo[handle].iWrite;
    			pthread_mutex_unlock(&g_stFifo[handle].lock_write);
        	}
        	else
        	{
    			unsigned int r_stamp = 0;
    			unsigned int n_stamp = g_stFifo[handle].iStamp;
    			int offset = g_stFifo[handle].iRead_Base;
    			pthread_mutex_lock(&g_stFifo[handle].lock_write);
    			fifo_read_stamp(handle,i,&offset,&r_stamp);
    			while ((r_stamp+(second+3)) < n_stamp)
	 				fifo_read_stamp(handle,i,&offset,&r_stamp);
    			g_stFifo[handle].iRead[i] = offset;
    			pthread_mutex_unlock(&g_stFifo[handle].lock_write);
        	}
            printf("%s:%d request a fifo handle %d reader %d\n",__FUNCTION__,__LINE__,handle,i);
            return i;
        }
	}
	return -1;
}

int FIFO_Get_MediaInfo(st_fifo_t stFifo,char * sps,int *sps_len,char *pps,int *pps_len)
{
	memcpy(sps,sps_level[stFifo.iGrp][stFifo.iChn],g_sps_len[stFifo.iGrp][stFifo.iChn]);
	*sps_len = g_sps_len[stFifo.iGrp][stFifo.iChn];
	memcpy(pps,pps_param[stFifo.iGrp][stFifo.iChn],g_pps_len[stFifo.iGrp][stFifo.iChn]);
	*pps_len = g_pps_len[stFifo.iGrp][stFifo.iChn];
	return 0;
}

int FIFO_Read(int iHandle,int rId,char *pData)
{
	if (g_stFifo[iHandle].iRead[rId] == g_stFifo[iHandle].iWrite)
        return 0;
	if (g_stFifo[iHandle].iRead[rId] < 0)
    {
    	printf("出错啦!出错啦!fifo已经关闭啦\n");
        return 0;
    }
    
    int iLen;
    char buf[8];
	pthread_mutex_lock(&g_stFifo[iHandle].lock_read[rId]);
	int offset =g_stFifo[iHandle].iRead[rId];
//    printf("offset = %d, iReadBase = %d, iWrite = %d\n", offset,g_stFifo[iHandle].iWrite,g_stFifo[iHandle].iRead_Base);
    if ((offset+6) > g_stFifo[iHandle].capacity)
    {
        memcpy(buf,g_stFifo[iHandle].pBuf+offset,g_stFifo[iHandle].capacity-offset);
        memcpy(buf+(g_stFifo[iHandle].capacity-offset),g_stFifo[iHandle].pBuf,6-(g_stFifo[iHandle].capacity-offset));
        memcpy((char *)&iLen,buf+2,4);
    }
    else
        memcpy((char *)&iLen,g_stFifo[iHandle].pBuf+offset+2,4);
    if ((iLen>=1500)||(iLen<0))
    {
        printf("FIFO_Read:error iLen %d r %d w %d b %d d %d\n\n",iLen,g_stFifo[iHandle].iRead[rId],g_stFifo[iHandle].iWrite,g_stFifo[iHandle].iRead_Base,g_stFifo[iHandle].iDrop);
        fflush(stdout);
        g_stFifo[iHandle].iRead[rId] = g_stFifo[iHandle].iWrite;
        pthread_mutex_unlock(&g_stFifo[iHandle].lock_read[rId]);
        return -1;
    }
    
    if ((offset+iLen) > g_stFifo[iHandle].capacity)
    {
        memcpy(pData,g_stFifo[iHandle].pBuf+offset,g_stFifo[iHandle].capacity-offset);
        memcpy(pData+(g_stFifo[iHandle].capacity-offset),g_stFifo[iHandle].pBuf,iLen-(g_stFifo[iHandle].capacity-offset));
        offset = iLen-(g_stFifo[iHandle].capacity-offset);
    }
    else
    {
        memcpy(pData,g_stFifo[iHandle].pBuf+offset,iLen);
        offset += iLen;
        if (offset >= g_stFifo[iHandle].capacity)
        {
            printf("FIFO_Read:fifo 转头\n");
            offset = 0;
        }
    }
    g_stFifo[iHandle].iRead[rId] = offset;
    pthread_mutex_unlock(&g_stFifo[iHandle].lock_read[rId]);
    return iLen;
//	return 0;
}

int FIFO_Release(int iHandle,int rId)
{
    printf("%s:%d rId %d\n",__FUNCTION__,__LINE__,rId);
	pthread_mutex_lock(&g_stFifo[iHandle].lock_read[rId]);
	if (g_stFifo[iHandle].iRead[rId] == -1)
	    printf("iHandle %d rId %d already is free\n",iHandle,rId);
	g_stFifo[iHandle].iRead[rId] = -1;
	pthread_mutex_unlock(&g_stFifo[iHandle].lock_read[rId]);
	return 0;
}

int FIFO_Destory(int iHandle)
{
	int j;
	for (j=0; j<FIFO_READ_NUM; j++)
	{	
		g_stFifo[iHandle].iRead[j] = -1;
		pthread_mutex_destroy(&g_stFifo[iHandle].lock_read[j]);
	}
	pthread_mutex_destroy(&g_stFifo[iHandle].lock_write);
	if (g_stFifo[iHandle].pBuf != NULL)
		free(g_stFifo[iHandle].pBuf);
	g_stFifo[iHandle].pBuf = NULL;
	g_stFifo[iHandle].capacity = 0;
	return 0;
}

int FIFO_Cleanup(void)
{
	return 0;
}

/******************************************************************************/
//FIFO Stream
/******************************************************************************/
int g_iInitalize=0;
st_fifoStream_t g_stFifoStream[FIFO_MAX_STREAM];
int FIFO_Stream_Open(e_fifo_stream enStream,int iGroup,int iChn,int second)
{
	int i;
	int j;
	if (g_iInitalize == 0)
	{
		g_iInitalize = 1;
		for (i=0; i<FIFO_MAX_STREAM; i++)
		{
			g_stFifoStream[i].enType=-1;
			g_stFifoStream[i].iGrp=-1;
			g_stFifoStream[i].iWrite= 0;
			for (j=0; j<FIFO_READ_STREAM; j++)
				g_stFifoStream[i].iRead[j] = -1;
			for (j=0; j<FIFO_MAX_BUFFER; j++)
			{	
				g_stFifoStream[i].stFifoData[j].iLen = 0;
				g_stFifoStream[i].stFifoData[j].pData = NULL;
			}
		}
	}
	for (i=0; i<FIFO_MAX_STREAM; i++)
	{
		if ((g_stFifoStream[i].enType==enStream) && (iGroup==g_stFifoStream[i].iGrp) && (iChn==g_stFifoStream[i].iChn))
			goto FIFO_STREAM_OLD;
	}
	for (i=0; i<FIFO_MAX_STREAM; i++)
	{
		if (g_stFifoStream[i].enType==-1)
			goto FIFO_STREAM_NEW;
	}	
	return -1;
	
FIFO_STREAM_NEW:
	g_stFifoStream[i].enType=enStream;
	g_stFifoStream[i].iGrp=iGroup;
	g_stFifoStream[i].iChn=iChn;
	g_stFifoStream[i].iPreSecond=second;
	
	if (!((enStream==FIFO_STREAM_AVI)||(enStream==FIFO_STREAM_H264)||(enStream==FIFO_STREAM_AUDIO)))
	{
		for (j=0; j<FIFO_MAX_BUFFER; j++)
		{	
			g_stFifoStream[i].stFifoData[j].pData = malloc(1024);
			if (g_stFifoStream[i].stFifoData[j].pData == NULL)
			{
				int k;
				for (k=0; k<j; k++)
				{	
					free(g_stFifoStream[i].stFifoData[k].pData);
					g_stFifoStream[i].stFifoData[k].pData = NULL;
				}			
			}
		}
		g_stFifoStream[i].iRealMaxBuffer = FIFO_MAX_BUFFER;
	}
	g_stFifoStream[i].iConvertThread = 1;
	switch(enStream)
	{
		case FIFO_STREAM_RTP:
			pthread_create(&g_stFifoStream[i].thread_convert,NULL,convert_rtp_stream,(void *)i);
			break;
		case FIFO_STREAM_AVI:
			g_stFifoStream[i].iRealMaxBuffer = FIFO_MAX_BUFFER;//FIFO_AVI_BUFFER;
			pthread_create(&g_stFifoStream[i].thread_convert,NULL,convert_avi_stream,(void *)i);		//avi需要一次输出一帧数据，所以输出缓冲区在内部申请内存
			break;
		case FIFO_STREAM_TS:
			pthread_create(&g_stFifoStream[i].thread_convert,NULL,convert_ts_stream,(void *)i);
			break;
		case FIFO_STREAM_PS:
			pthread_create(&g_stFifoStream[i].thread_convert,NULL,convert_ps_stream,(void *)i);
			break;
		case FIFO_FILE_WEB2RTP:
			pthread_create(&g_stFifoStream[i].thread_convert,NULL,convert_web2rtp_file,(void *)i);
			break;
		case FIFO_FILE_AVI2RTP:
			pthread_create(&g_stFifoStream[i].thread_convert,NULL,convert_avi2rtp_file,(void *)i);
			break;
		case FIFO_STREAM_H264:
			pthread_create(&g_stFifoStream[i].thread_convert,NULL,convert_h264_stream,(void *)i);		//输出纯264码流，并直接回调出去。
			break;
		case FIFO_STREAM_AUDIO:
			pthread_create(&g_stFifoStream[i].thread_convert,NULL,convert_audio_stream,(void *)i);		//输出纯264码流，并直接回调出去。
			break;
		default:
			break;
	}
	usleep(200000);
FIFO_STREAM_OLD:	
	return i;
}
int FIFO_Stream_RequestID(int iHandle)
{
	int j;
	for (j=0; j<FIFO_READ_STREAM; j++)
	{
		if (g_stFifoStream[iHandle].iRead[j] == -1)
		{
			if (g_stFifoStream[iHandle].iPreSecond == 0)
				g_stFifoStream[iHandle].iRead[j] = g_stFifoStream[iHandle].iWrite;
			else
				g_stFifoStream[iHandle].iRead[j] = ((g_stFifoStream[iHandle].iWrite+512)>=FIFO_MAX_BUFFER)?g_stFifoStream[iHandle].iWrite+512-FIFO_MAX_BUFFER:g_stFifoStream[iHandle].iWrite+512;
			return j;
		}
	}
	return -1;
}
int FIFO_Stream_Set_(int iHandle,char *filename,int speed,int pulltime)
{
	g_stFifoStream[iHandle].speed = speed;
	g_stFifoStream[iHandle].pull_time = pulltime;
	memset(g_stFifoStream[iHandle].file_name,0,128);
	strcpy(g_stFifoStream[iHandle].file_name,filename);
	return 0;
}
//int FIFO_Stream_Write(int iHandle,int iID,char *pData,int iLen)
//{
//	return 0;
//}
int FIFO_Stream_Read(int iHandle,int iID,char *pData)
{
	int iRetLen=-1;
	if ((iID<0) || (iID>=FIFO_READ_STREAM) || (g_stFifoStream[iHandle].iRead[iID]<0))
		return iRetLen;
	if (g_stFifoStream[iHandle].iWrite != g_stFifoStream[iHandle].iRead[iID])
	{
		if (g_stFifoStream[iHandle].stFifoData[g_stFifoStream[iHandle].iRead[iID]].pData != NULL)
		{
			memcpy(pData,g_stFifoStream[iHandle].stFifoData[g_stFifoStream[iHandle].iRead[iID]].pData,g_stFifoStream[iHandle].stFifoData[g_stFifoStream[iHandle].iRead[iID]].iLen);
			if (g_stFifoStream[iHandle].enType == FIFO_STREAM_AVI)
			{
				free(g_stFifoStream[iHandle].stFifoData[g_stFifoStream[iHandle].iRead[iID]].pData);
				g_stFifoStream[iHandle].stFifoData[g_stFifoStream[iHandle].iRead[iID]].pData = NULL;
			}
			iRetLen = g_stFifoStream[iHandle].stFifoData[g_stFifoStream[iHandle].iRead[iID]].iLen;
			if ((g_stFifoStream[iHandle].iRead[iID]+1) >= g_stFifoStream[iHandle].iRealMaxBuffer)
				g_stFifoStream[iHandle].iRead[iID] = 0;
			else
				g_stFifoStream[iHandle].iRead[iID]++;
		}
	}
	return iRetLen;
}
int FIFO_Stream_ReleaseID(int iHandle,int iID)
{
	g_stFifoStream[iHandle].iRead[iID] = -1;
	return 0;
}
int FIFO_Stream_Close(int iHandle)
{
	g_stFifoStream[iHandle].iConvertThread=0;
	pthread_join(g_stFifoStream[iHandle].thread_convert,NULL);
	g_stFifoStream[iHandle].enType=-1;
	g_stFifoStream[iHandle].iGrp=-1;
	return 0;
}

int FIFO_Register_Callback(e_fifo_h264 enStreamChn,FIFO_CALLBACK fifo_callback)
{
	fifo_h264_CallBack[(int)enStreamChn] = fifo_callback;
	return 0;
}

int FIFO_Alarm_Write(st_alarm_upload_t *cInfo)
{
	memcpy(&g_stFifoAlarm[g_iFifoAlarm_Write],cInfo,sizeof(st_alarm_upload_t));
	if ((g_iFifoAlarm_Write+1) >= FIFO_ALARM_MAX_SHEET)
	{
		g_iFifoAlarm_Write = 0;
		g_iFifoAlarm_Full  = 1;
	}
	else
		g_iFifoAlarm_Write++;
	return 0;
}

int FIFO_Alarm_Get_Wpoint(e_fifo_read enRead)
{
	int iReturn=0;
	if (enRead == FIFO_ALARM_READ_CURRENT)
		return g_iFifoAlarm_Write;
	else if (enRead == FIFO_ALARM_READ_EARLY)
	{
		if (g_iFifoAlarm_Full != 0)
		{
			iReturn = g_iFifoAlarm_Write;
			iReturn = (iReturn+2)%FIFO_ALARM_MAX_SHEET;
		}	
	}
	
	return iReturn;
}

int FIFO_Alarm_Read(int iReader,st_alarm_upload_t *cInfo)
{
	memcpy(cInfo,&g_stFifoAlarm[iReader],sizeof(st_alarm_upload_t));
	return 0;
}
