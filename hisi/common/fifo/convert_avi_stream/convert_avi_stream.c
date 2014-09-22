#include "convert_avi_stream.h"

void *convert_avi_stream(void * cvt)
{
	int iHandle;
	int i=(int)cvt;
	
	st_fifo_t stFifo;	
	stFifo.iGrp = g_stFifoStream[i].iGrp;
	stFifo.iChn = g_stFifoStream[i].iChn;
	stFifo.enType = FIFO_TYPE_MEDIA;//g_stFifoStream[i].enType;
	int iFifo = FIFO_Request(stFifo,&iHandle,g_stFifoStream[i].iPreSecond);
	g_stFifoStream[i].iPreSecond = 0;
	int iGlen;
	int iPoint = 0;
	unsigned short usframeCur=0;
	unsigned short usframeCmp=0;
	char cBuf[1600];
	char * cNalu=malloc(0x800000);
	int hasIDR = 0;
    while(g_stFifoStream[i].iConvertThread == 1)
	{
		iGlen = FIFO_Read(iHandle,iFifo,cBuf);
		if (iGlen > 11)
		{
			if (cBuf[1] == 0x38)
			{
				if (hasIDR == 1)
				{
					iGlen -= 27;													//去掉0 0 0 1所以-27
					g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData = malloc(iGlen+12);
					memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData,"01wb",4);
					memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4,&iGlen,4);
					memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+8,cBuf+27,iGlen);
					g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].iLen = iGlen+8;
					if (g_stFifoStream[i].iWrite+1 >= FIFO_MAX_BUFFER)
						g_stFifoStream[i].iWrite = 0;
					else
						g_stFifoStream[i].iWrite++;
				}
			}
			else if (cBuf[1] == 0x39)
			{
				if (hasIDR == 1)
				{
					memcpy(&usframeCur,cBuf+11,2);
					if (usframeCur == usframeCmp)
					{
						if ((cBuf[20]&0x80) != 0)
						{
							memcpy(cNalu+iPoint,&cBuf[27],iGlen-27);
							iPoint += iGlen-27;
						}
						else
						{
							memcpy(cNalu+iPoint,&cBuf[23],iGlen-23);
							iPoint += iGlen-23;
						}
					}
					else
					{
						g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData = malloc(iPoint+12);
						memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData,"00dc",4);
						memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4,&iPoint,4);
						memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+8,cNalu,iPoint);
						g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].iLen = iPoint+8;
						if (g_stFifoStream[i].iWrite+1 >= FIFO_MAX_BUFFER)
							g_stFifoStream[i].iWrite = 0;
						else
							g_stFifoStream[i].iWrite++;
						memcpy(cNalu,&cBuf[23],iGlen-23);
						iPoint = iGlen-23;
						usframeCmp = usframeCur;
					}
				}
				else
				{
					if ((cBuf[20]&0x80) == 0)
					{
						if ((cBuf[27]&0x1f) == 7)
						{
							hasIDR  = 1;
							memcpy(cNalu,&cBuf[23],iGlen-23);
							iPoint  = iGlen-23;
						}
					}
					memcpy(&usframeCmp,cBuf+11,2);
					usframeCur = usframeCmp;
				}
			}
		}
		else
			usleep(10000);
	}
	FIFO_Release(iHandle,iFifo);
	if (cNalu != NULL)
		free(cNalu);
	return cvt;
}

void FIFO_Stream_Get_AviHead(char *head_buf,int *ptr,int reso,int frame)
{
	int len;
	unsigned short tmp;
	(*ptr) = 0;
	memcpy(head_buf+(*ptr),"RIFF",4);(*ptr)+=4;
	memset(head_buf+(*ptr),0,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"AVI ",4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"LIST",4);(*ptr)+=4;
	len = 4 + 4*16 + 12 + 4*16 + 4*12 +12 + 4*16 + 8+4*4+2;		
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"hdrl",4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"avih",4);(*ptr)+=4;
    len = 4*16 - 8;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 1000000/frame;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;			// ************my test 视频帧间隔时间 以毫秒为单位
	memset(head_buf+(*ptr),0xff,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	len = AVIF_HASINDEX|AVIF_ISINTERLEAVED|AVIF_TRUSTCKTYPE;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 3000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;			//  *************my test 文件的总帧数 此处有误,不过不影响。
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	len = 2;											
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 1000000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;	
	int width;
	int height;
	UTILITY_MEDIA_Convert_Size(reso,&width,&height);
	printf("reso %d width %d height %d\n",reso,width,height);
	memcpy(head_buf+(*ptr),&width,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),&height,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"LIST",4);(*ptr)+=4;
    len = 4 + 4*16 + 4*12;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"strl",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"strh",4);(*ptr)+=4;
    len = 4*16 - 8;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"vids",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"h264",4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	len = 1000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;	 // dwScale 这个流使用的时间尺度	
	len = frame*1000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;	//  ********* my test dwRate  dwRate / dwScale = 帧率（fram rate ）。
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	len = 3000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;	// 	**********my test dwLength 表示流的长度。这其实就是总帧数，用dwLength除以帧率，即得到流的总时长。
	len = (1<<20);
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = -1;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = width*height;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	tmp = width;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	tmp = height;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	memcpy(head_buf+(*ptr),"strf",4);(*ptr)+=4;
    len = 4*12 - 8;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 4*12 - 8;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;			// 0xb0
	memcpy(head_buf+(*ptr),&width,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),&height,4);(*ptr)+=4;
	tmp = 1;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	tmp = 24;	
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	memcpy(head_buf+(*ptr),"h264",4);(*ptr)+=4;
    len = width*height;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	
	memcpy(head_buf+(*ptr),"LIST",4);(*ptr)+=4;
    len = 4 + 4*16 + 4*6 + 2;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"strl",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"strh",4);(*ptr)+=4;
    len = 4*16 - 8;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"auds",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"G711",4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	len = 1;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 8000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	len = 325;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 32768;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 0;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 8000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	tmp = 0;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	tmp = 0;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	
	memcpy(head_buf+(*ptr),"strf",4);(*ptr)+=4;
    len = 4*4+2;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	tmp = 0x07;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	tmp = 0x01;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	len = 8000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	len = 8000;
	memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
	tmp = 1;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	tmp = 8;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	tmp = 0;
	memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
	
	memcpy(head_buf+(*ptr),"LIST",4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
	memcpy(head_buf+(*ptr),"movi",4);(*ptr)+=4;
}

void FIFO_Stream_Get_HeadIndex(char *index_buf,int *index_len,int len)
{
	memcpy(index_buf+(*index_len),"idx1",4);
	(*index_len)+=4;
	memset(index_buf+(*index_len),0x00,4);
	(*index_len)+=4;
	memcpy(index_buf+(*index_len),&len,4);
	(*index_len)+=4;
}

void FIFO_Stream_Get_DataIndex(int isAudio,char idrFlag,char *index_buf,int *index_len,int len)
{
	int aviTotal;
	memcpy(&aviTotal,index_buf,4);
	if (isAudio == 1)
	{
		memcpy(index_buf+(*index_len),"01wb",4);
		memset(index_buf+(*index_len)+4,0,4);
	}
	else
	{
		memcpy(index_buf+(*index_len),"00dc",4);
		if ((idrFlag&0x1f) == 0x07)
		{
			index_buf[(*index_len)+4] = 0x10;
			index_buf[(*index_len)+5] = 0x00;
			index_buf[(*index_len)+6] = 0x00;
			index_buf[(*index_len)+7] = 0x00;
		}
		else
			memset(index_buf+(*index_len)+4,0,4);
	}
	memcpy(index_buf+(*index_len)+8,&aviTotal,4);		//在avi文件中的偏移地址
	memcpy(index_buf+(*index_len)+12,&len,4);			//本数据块的长度
	if (len&0x01)
		aviTotal = aviTotal+len+8+1;
	else
		aviTotal = aviTotal+len+8;
//	printf("==========aviTotal %d len %d\n",aviTotal,len);
	memcpy(index_buf,&aviTotal,4);
	(*index_len) += 16;
}


