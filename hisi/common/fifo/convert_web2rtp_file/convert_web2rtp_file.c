#include "convert_web2rtp_file.h"
#include "../fifo.h"

FILE * FILE_Open(char * filename)
{
	FILE * pFd = fopen(filename,"r");
	if (pFd != NULL)
		fseek(pFd,0x09,SEEK_SET);
	return pFd;
}
int FILE_Read(FILE * pFd,char *pBuf)
{
	if (pFd == NULL)
		return 0;
	unsigned short usLen;
	fread(pBuf+11,1,11,pFd);
	memcpy(&usLen,&pBuf[21],2);
	fread(pBuf+23,1,usLen,pFd);
	int iLen;
	pBuf[0] = 0xff;
	pBuf[1] = 0x39;
	if (pBuf[20] > 0x50)
		pBuf[1] = 0x38;
	iLen = usLen+23;
	memcpy(&pBuf[2],&usLen,2);
	return usLen+23;
}
void *convert_web2rtp_file(void * cvt)
{
	int i=(int)cvt;
	
	st_fifo_t stFifo;	
	stFifo.iGrp   = g_stFifoStream[i].iGrp;
	stFifo.iChn   = g_stFifoStream[i].iChn;
	stFifo.enType = g_stFifoStream[i].enType;
	FILE * pFd = FILE_Open(g_stFifoStream[i].file_name);
	
	int iGlen;
	int iPoint = 0;
	unsigned short usframeCur=0;
	unsigned short usframeCmp=0;
	char cBuf[1600];
	char * cNalu=malloc(0x80000);
	st_web_rtp_head_t stRtpVideo;
	st_web_rtp_head_t stRtpAudio;
	
	stRtpVideo.ver= 2;
	stRtpVideo.p  = 0;
	stRtpVideo.x  = 0;
	stRtpVideo.cc = 0;
	stRtpVideo.pt = 96;
	stRtpVideo.ssrc = htonl(stFifo.iGrp*2);

	stRtpAudio.ver= 2;
	stRtpAudio.p  = 0;
	stRtpAudio.x  = 0;
	stRtpAudio.cc = 0;
	stRtpAudio.pt = 0;
    stRtpAudio.m  = 1;
    stRtpAudio.ts = 0;
    stRtpAudio.ssrc = htonl(stFifo.iGrp*2+1);
    int isIDR = 0;
    int hasIDR = 0;
    unsigned int   iNalu_cnt=0;
	unsigned int   audio_pts=0;
	unsigned int   video_pts=0;
	unsigned short audio_seq=0;
	unsigned short video_seq=0;
	unsigned int   iNalu_len[8];
	st_web_rtsp_head_t * pRtsp=NULL;
	while(g_stFifoStream[i].iConvertThread == 1)
	{
		iGlen = FILE_Read(pFd,cBuf);
		if (iGlen > 11)
		{
			if (cBuf[1] == 0x38)
			{
				pRtsp = (st_web_rtsp_head_t *)g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData;
				stRtpAudio.ts = htonl(audio_pts);
				audio_pts += 160;
				stRtpAudio.seq = htons(audio_seq++);
				pRtsp->symbol = 0x24;
				pRtsp->chan = 0x2;
				pRtsp->ilen = htons(iGlen-27+sizeof(st_rtp_head_t));
				
				memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4,&stRtpAudio,sizeof(st_rtp_head_t));
				g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].iLen = iGlen-27+sizeof(st_rtp_head_t)+sizeof(st_rtsp_head_t);
				memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4+sizeof(st_rtp_head_t),&cBuf[27],iGlen-27);
				if (g_stFifoStream[i].iWrite+1 >= FIFO_MAX_BUFFER)
					g_stFifoStream[i].iWrite = 0;
				else
					g_stFifoStream[i].iWrite++;
			}
			else if (cBuf[1] == 0x39)
			{
				if (hasIDR == 0)
				{
					memcpy(&usframeCur,cNalu+11,2);
					if (usframeCur == usframeCmp)
					{
						if ((cNalu[20]&0x80) != 0)
						{
							memcpy(cNalu+iPoint,&cNalu[27],iGlen-27);
							iNalu_len[iNalu_cnt] += iGlen-27;
							iPoint += iGlen-27;
						}
						else
						{
							memcpy(cNalu+iPoint,&cNalu[27],iGlen-27);
							iPoint += iGlen-27;
							iNalu_cnt++;
							iNalu_len[iNalu_cnt] = iGlen-27;
						}
					}
					else
					{
						if ((cNalu[4]&0x1f) == 7)
							isIDR = 1;
						else
							isIDR = 0;
						int j;
						unsigned short rtpLen;
						for (j=0; j<iNalu_cnt; j++)
						{
							if ((iNalu_len[j]>1001) || (isIDR==1))
							{
								int iFirst  = 0;
								int iRtpPoint=0;
								while (iNalu_len[j] > 0)
								{
									if (iNalu_len[j] > 1000)
									{	
										rtpLen = 1000;
										stRtpVideo.m = 0;
									}
									else
									{
										rtpLen = iNalu_len[j];
										stRtpVideo.m = 1;
									}
									stRtpVideo.ts = htonl(video_pts);										
										
									pRtsp->symbol = 0x24;
									pRtsp->chan   = 0x2;
									stRtpVideo.seq = htons(video_seq++);
									memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4,&stRtpVideo,sizeof(st_rtp_head_t));
									if (iFirst == 0)
									{
										iFirst = 1;
										pRtsp->ilen = htons(rtpLen+sizeof(st_rtp_head_t)+2-1);
										if (isIDR == 1)
										{
											g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData[12] = 0x7c;
											g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData[13] = 0x85;
										}
										else
										{
											g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData[12] = 0x5c;
											g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData[13] = 0x81;
										}
										memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4+sizeof(st_rtp_head_t)+2,cBuf+iRtpPoint+1,rtpLen-1);
										g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].iLen = sizeof(st_rtsp_head_t)+sizeof(st_rtp_head_t)+2-1+rtpLen;
									}
									else
									{	
										pRtsp->ilen = htons(rtpLen+sizeof(st_rtp_head_t)+2);
										if (isIDR == 1)
										{
											g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData[13] = 0x45;
											if (stRtpVideo.m == 1)
												g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData[13] = 0x05;
										}
										else
										{
											g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData[13] = 0x41;
											if (stRtpVideo.m == 1)
												g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData[13] = 0x01;
										}
										memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4+sizeof(st_rtp_head_t)+2,cBuf+iRtpPoint,rtpLen);
										g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].iLen = sizeof(st_rtsp_head_t)+sizeof(st_rtp_head_t)+2+rtpLen;
									}
									if (g_stFifoStream[i].iWrite+1 >= FIFO_MAX_BUFFER)
										g_stFifoStream[i].iWrite = 0;
									else
										g_stFifoStream[i].iWrite++;
									iRtpPoint += rtpLen;
									iNalu_len[j] -= rtpLen;
								}
							}
							else
							{
								pRtsp = (st_web_rtsp_head_t *)g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData;
								stRtpVideo.ts = htonl(video_pts);
								stRtpVideo.seq = htons(video_seq++);
								stRtpVideo.m = 1;
								pRtsp->symbol = 0x24;
								pRtsp->chan = 0x2;
								pRtsp->ilen = htons(iGlen-27+sizeof(st_rtp_head_t));
								
								memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4,&stRtpVideo,sizeof(st_rtp_head_t));
								g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].iLen = iGlen-27+sizeof(st_rtp_head_t)+sizeof(st_rtsp_head_t);
								memcpy(g_stFifoStream[i].stFifoData[g_stFifoStream[i].iWrite].pData+4+sizeof(st_rtp_head_t),&cBuf[27],iGlen-27);
								if (g_stFifoStream[i].iWrite+1 >= FIFO_MAX_BUFFER)
									g_stFifoStream[i].iWrite = 0;
								else
									g_stFifoStream[i].iWrite++;
							}
						}
						
						iNalu_cnt=0;
						video_pts += 3600;
						memcpy(cNalu,&cNalu[23],iGlen-23);
						iPoint  = iNalu_len[0] = iGlen-23;
					}
				}
				else
				{
					if ((cNalu[20]&0x80) == 0)
					{
						if ((cNalu[27]&0x1f) == 7)
						{
							hasIDR  = 1;
							iNalu_cnt=0;
							memcpy(cNalu,&cNalu[23],iGlen-23);
							iPoint  = iNalu_len[0] = iGlen-23;
						}
					}
					memcpy(&usframeCmp,cNalu+11,2);
					usframeCur = usframeCmp;
				}
			}
		}
		else
			usleep(10000);
	}
	if (pFd != NULL)
		fclose(pFd);
	if (cNalu != NULL)
		free(cNalu);
	return cvt;
}
