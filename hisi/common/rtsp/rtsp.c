#include "rtsp.h"

static int rtsp_init=0;
static st_rtsp_info_t stRtspInfo[32];
/***************************************************************************
*	函数名称：	RTSP_Initialize
*	功能描述：	从src查找content复制到dst
*	日    期：	2010-01-19 08:20
*	作    者：	季增光
*	返 回 值：	0表示拔掉网线1表示插上网线
***************************************************************************/
int RTSP_Initialize(void)
{
	int i;
	for (i=0; i<32; i++)
	{
		stRtspInfo[i].iSock = -1;
		memset(stRtspInfo[i].cRtpInfo,0,128);
	}
	return 0;
}
/***************************************************************************
*	函数名称：	searchStr
*	功能描述：	从src查找content复制到dst
*	日    期：	2010-01-19 08:20
*	作    者：	季增光
*	参数说明：
*
*	返 回 值：	0表示拔掉网线1表示插上网线
***************************************************************************/
int searchStr(char *src,char *dst,char *content,int add)
{
	int  len;
	char * q=NULL;
	char * e=NULL;

	dst[0] = 0;
	q = strstr(src,content);
	if (q != NULL)
	{
		e = strstr(q,"\r\n");
		if (e != NULL)
		{
			len = (int)e-(int)q+add;
			memcpy(dst,q,len);
			dst[len] = 0;
		}
	}
	return 0;
}

int RTSP_Options(int iSock,char *pData,int iLen,char *pOut)
{
	char CSeq[128]={0};
	char UAnt[128]={0};
	int  session;
	session = time((time_t *)NULL);
	searchStr(pData,CSeq,"CSeq:",2);
	searchStr(pData,UAnt,"User-Agent:",2);
	if (strlen(CSeq) == 0)
		return 0;
	
	if (rtsp_init == 0)
	{
		rtsp_init = 1;
		RTSP_Initialize();
	}
	if (strlen(UAnt) == 0)
		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nPublic: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS SET_PARAMETER\r\nContent-Length: 0\r\n\r\n",CSeq);
	else
		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nPublic: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS SET_PARAMETER\r\n%sContent-Length: 0\r\n\r\n",CSeq,UAnt);
	char * p = strstr(pData+strlen("OPTIONS"),"OPTIONS");
	if (p != NULL)
	{
		searchStr(p,CSeq,"CSeq:",2);
		searchStr(p,UAnt,"User-Agent:",2);
		if (strlen(UAnt) == 0)
			sprintf(pOut+strlen(pOut),"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nPublic: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS SET_PARAMETER\r\nContent-Length: 0\r\n\r\n",CSeq);
		else
			sprintf(pOut+strlen(pOut),"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nPublic: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS SET_PARAMETER\r\n%sContent-Length: 0\r\n\r\n",CSeq,UAnt);
	}
	return strlen(pOut);
}
int RTSP_Describe(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut)
{
	char CSeq[128]={0};
	char UAnt[128]={0};
	char ACpt[128]={0};
	int  session;
	session = time((time_t *)NULL);
	searchStr(pData,CSeq,"CSeq:",2);
	searchStr(pData,UAnt,"User-Agent:",2);
	searchStr(pData,ACpt,"Accept:",2);
	if (strlen(CSeq) == 0)
		return 0;
	
	if (rtsp_init == 0)
	{
		rtsp_init = 1;
		RTSP_Initialize();
	}
	char content[1024];
	st_rtsp_real_t * stReal = (st_rtsp_real_t *)pAttr;
	st_rtsp_history_t * stHistory = (st_rtsp_history_t *)pAttr;
	switch (enRtsp)
	{
		case NET_RTSP_MEDIA:
			if (stHistory->hasAudio == 1)
				sprintf(content,"v=0\r\no=PU %d 201 IN IP4 x.y.z.w\r\ns=Media Presentation\r\nc=IN IP4 0.0.0.0\r\nt=0 0\r\na=range:npt=0-%d\r\nm=video 0 RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\na=fmtp:96 profile-level-id=%s; sprop-parameter-sets=%s; packetization-mode=1\r\na=control:trackID=1\r\nm=audio 0 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\na=control:trackID=2\r\n\r\n",session,stReal->hasAudio,stReal->level,stReal->param);
			else
				sprintf(content,"v=0\r\no=PU %d 201 IN IP4 x.y.z.w\r\ns=Media Presentation\r\nc=IN IP4 0.0.0.0\r\nt=0 0\r\na=range:npt=0-%d\r\nm=video 0 RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\na=fmtp:96 profile-level-id=%s; sprop-parameter-sets=%s; packetization-mode=1\r\na=control:trackID=1\r\n\r\n",session,stReal->hasAudio,stReal->level,stReal->param);
			break;
		case NET_RTSP_FILE:
			if (stHistory->hasAudio == 1)
				sprintf(content,"v=0\r\no=PU %d 201 IN IP4 x.y.z.w\r\ns=Media Presentation\r\nc=IN IP4 0.0.0.0\r\nt=0 0\r\na=range:npt=0-%d\r\nm=video 0 RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\na=fmtp:96 profile-level-id=%s; sprop-parameter-sets=%s; packetization-mode=1\r\na=control:trackID=1\r\nm=audio 0 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\na=control:trackID=2\r\n\r\n",session,stHistory->hasAudio,stHistory->level,stHistory->param);
			else
				sprintf(content,"v=0\r\no=PU %d 201 IN IP4 x.y.z.w\r\ns=Media Presentation\r\nc=IN IP4 0.0.0.0\r\nt=0 0\r\na=range:npt=0-%d\r\nm=video 0 RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\na=fmtp:96 profile-level-id=%s; sprop-parameter-sets=%s; packetization-mode=1\r\na=control:trackID=1\r\n\r\n",session,stHistory->hasAudio,stHistory->level,stHistory->param);
			break;
		default:
			break;
	}
	char rtsp[128];
	searchStr(pData,rtsp,"rtsp:",-9);
	if (strlen(UAnt) == 0)
		sprintf(pOut,"RTSP/1.0 200 OK\r\n%s%sContent-Base: %s\r\nContent-Type: application/sdp\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nContent-Length: %d\r\n\r\n%s",ACpt,CSeq,rtsp,strlen(content),content);
	else
    	sprintf(pOut,"RTSP/1.0 200 OK\r\n%s%sContent-Base: %s\r\nContent-Type: application/sdp\r\nServer: PVSS/2.01 (Platform/Linux;)\r\n%sContent-Length: %d\r\n\r\n%s",ACpt,CSeq,rtsp,UAnt,strlen(content),content);
	
	return strlen(pOut);
}
int RTSP_Setup(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut)
{
	char CSeq[128]={0};
	char UAnt[128]={0};
	int  session;
	session = time((time_t *)NULL);
	searchStr(pData,CSeq,"CSeq:",2);
	searchStr(pData,UAnt,"User-Agent:",2);
	if (strlen(CSeq) == 0)
		return 0;
	
	int i;
	for (i=0; i<32; i++)
	{
		if (iSock == stRtspInfo[i].iSock)
			break;
	}
	if (i >= 32)
	{
		for (i=0; i<32; i++)
		{
			if (-1 == stRtspInfo[i].iSock)
				break;
		}
	}	
	char * pS = strstr(pData,"rtsp://");
	if (pS != NULL)
	{
		int  iLen;
		char * pE = strstr(pS,"realplay");
		if (pE == NULL)
		{
			pE = strstr(pS,"/h264");
            if (pE == NULL)
            {
            	pE = strstr(pS,"\r\n");
            	iLen = pE-pS+9;
            }                
            else
            	iLen = pE-pS+strlen("/h264");
		}
		else
			iLen = pE-pS+strlen("realplay");
		memcpy(stRtspInfo[i].cRtpInfo,pS,iLen);
	}
	char trans[128];
	searchStr(pData,trans,"Transport:",0);
	st_rtsp_setup_t * stSetup = (st_rtsp_setup_t *)pAttr;
	if (stSetup->isTcp == 0)
	{
		if (strlen(stSetup->cSource)==0)
		{
			if (strlen(UAnt)==0)
				sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nSession: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\nContent-Length: 0\r\n\r\n",CSeq,session,stSetup->iClientPort1,stSetup->iClientPort2,stSetup->iServerPort1,stSetup->iServerPort2);
			else
				sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nSession: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n%sContent-Length: 0\r\n\r\n",CSeq,session,stSetup->iClientPort1,stSetup->iClientPort2,stSetup->iServerPort1,stSetup->iServerPort2,UAnt);
		}
		else
		{
			if (strlen(UAnt)==0)
				sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nSession: %d\r\nTransport: RTP/AVP/UDP;unicast;destination=%s;client_port=%d-%d;mode=play;source=%d.%d.%d.%d;server_port=%d-%d\r\nContent-Length: 0\r\n\r\n",CSeq,session,stSetup->cTarget,stSetup->iClientPort1,stSetup->iClientPort2,stSetup->cSource[0],stSetup->cSource[1],stSetup->cSource[2],stSetup->cSource[3],stSetup->iServerPort1,stSetup->iServerPort2);
			else
				sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nSession: %d\r\nTransport: RTP/AVP/UDP;unicast;destination=%s;client_port=%d-%d;mode=play;source=%d.%d.%d.%d;server_port=%d-%d\r\n%sContent-Length: 0\r\n\r\n",CSeq,session,stSetup->cTarget,stSetup->iClientPort1,stSetup->iClientPort2,stSetup->cSource[0],stSetup->cSource[1],stSetup->cSource[2],stSetup->cSource[3],stSetup->iServerPort1,stSetup->iServerPort2,UAnt);
		}
	}
	else
	{
		if (strlen(UAnt) == 0)
			sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nSession: %d\r\n%s\r\nContent-Length: 0\r\n\r\n",CSeq,session,trans);
		else
			sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nSession: %d\r\n%s\r\n%sContent-Length: 0\r\n\r\n",CSeq,session,trans,UAnt);
	}
	
	return strlen(pOut);
}
int RTSP_Play(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut)
{
	char CSeq[128]={0};
	char UAnt[128]={0};
	int  session;
	session = time((time_t *)NULL);
	searchStr(pData,CSeq,"CSeq:",2);
	searchStr(pData,UAnt,"User-Agent:",2);
	if (strlen(CSeq) == 0)
		return 0;
	
	int i;
	for (i=0; i<32; i++)
	{
		if (iSock == stRtspInfo[i].iSock)
			break;
	}
	if (i >= 32)
		return 0;
	
	char range[128];
	searchStr(pData,range,"Range:",0);
	if (enRtsp != NET_RTSP_PLAY)
	{
		if (strlen(stRtspInfo[i].cRtpInfo) == 0)
		{
			if (strlen(range) == 0)
			{
				if (strlen(UAnt)==0)
	        		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nSession: %d\r\nContent-Length: 0\r\n\r\n",CSeq,session);
	        	else
	        		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nSession: %d\r\n%sContent-Length: 0\r\n\r\n",CSeq,session,UAnt);
			}
			else
			{
				if (strlen(UAnt)==0)
	        		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=%s/trackID=1\r\nSession: %d\r\nContent-Length: 0\r\n\r\n",CSeq,stRtspInfo[i].cRtpInfo,session);
	        	else
	        		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=%s/trackID=1\r\nSession: %d\r\n%sContent-Length: 0\r\n\r\n",CSeq,stRtspInfo[i].cRtpInfo,session,UAnt);
			}
		}
		else
		{
			if (strlen(range) == 0)
			{
				if (strlen(UAnt)==0)
	        		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=%s/trackID=1\r\nSession: %d\r\nContent-Length: 0\r\n\r\n",CSeq,stRtspInfo[i].cRtpInfo,session);
	        	else
	        		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=%s/trackID=1\r\nSession: %d\r\n%sContent-Length: 0\r\n\r\n",CSeq,stRtspInfo[i].cRtpInfo,session,UAnt);
			}
			else
			{
				if (strlen(UAnt)==0)
	        		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=%s/trackID=1\r\n%s\r\nSession: %d\r\nContent-Length: 0\r\n\r\n",CSeq,stRtspInfo[i].cRtpInfo,range,session);
	        	else
	        		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=%s/trackID=1\r\n%s\r\nSession: %d\r\n%sContent-Length: 0\r\n\r\n",CSeq,stRtspInfo[i].cRtpInfo,range,session,UAnt);
			}
		}
	}
	else
	{
		st_rtsp_play_t *stPlay = (st_rtsp_play_t *)pAttr;
	    if (strlen(UAnt) == 0)
    		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=trackID=1;seq=%d;rtptime=%d,url=trackID=2;seq=%d;rtptime=%d\r\n%s\r\nSession: %d\r\nContent-Length: 0\r\n\r\n",CSeq,stPlay->v_rtpseq,stPlay->v_rtptime,stPlay->a_rtpseq,stPlay->a_rtptime,range,session);
    	else
    		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=trackID=1;seq=%d;rtptime=%d,url=trackID=2;seq=%d;rtptime=%d\r\n%s\r\nSession: %d\r\n%sContent-Length: 0\r\n\r\n",CSeq,stPlay->v_rtpseq,stPlay->v_rtptime,stPlay->a_rtpseq,stPlay->a_rtptime,range,session,UAnt);
	}
	
	return strlen(pOut);
}
int RTSP_Pause(int iSock,char *pData,int iLen,char *pOut)
{
	char CSeq[128]={0};
	char UAnt[128]={0};
	int  session;
	session = time((time_t *)NULL);
	searchStr(pData,CSeq,"CSeq:",2);
	searchStr(pData,UAnt,"User-Agent:",2);
	if (strlen(CSeq) == 0)
		return 0;
	
	if (strlen(UAnt) == 0)
		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nSession: %d\r\nContent-Length: 0\r\n\r\n",CSeq,session);
	else
		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Type: text/xml\r\nSession: %d\r\n%s\r\nContent-Length: 0\r\n\r\n",CSeq,session,UAnt);
	
	return strlen(pOut);
}
int RTSP_Teardown(int iSock,char *pData,int iLen,char *pOut)
{
	if (iLen != 0)
	{
		char CSeq[128]={0};
		char UAnt[128]={0};
		int  session;
		session = time((time_t *)NULL);
		searchStr(pData,CSeq,"CSeq:",2);
		searchStr(pData,UAnt,"User-Agent:",2);
		if (strlen(CSeq) == 0)
			return 0;		
		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sSession: %d\r\n\r\n",CSeq,session);
	}
	int i;
	for (i=0; i<32; i++)
	{
		if (iSock == stRtspInfo[i].iSock)
			break;
	}
	if (i >= 32)
		return 0;	
	stRtspInfo[i].iSock = -1;
	memset(stRtspInfo[i].cRtpInfo,0,128);
	
	return strlen(pOut);
}
int RTSP_SetParameter(int iSock,char *pData,int iLen,char *pOut)
{
	char CSeq[128]={0};
	char UAnt[128]={0};
	int  session;
	session = time((time_t *)NULL);
	searchStr(pData,CSeq,"CSeq:",2);
	searchStr(pData,UAnt,"User-Agent:",2);
	if (strlen(CSeq) == 0)
		return 0;
	
	char range[128];
	searchStr(pData,range,"Range:",0);
	if (strlen(UAnt) == 0)
		sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Length: 0\r\nContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=trackID=991\r\n%s\r\nSession: %d\r\n",CSeq,range,session);
	else
    	sprintf(pOut,"RTSP/1.0 200 OK\r\n%sContent-Length: 0\r\nContent-Type: text/xml\r\nServer: PVSS/2.01 (Platform/Linux;)\r\nRTP-Info: url=trackID=991\r\n%s\r\nSession: %d\r\n%s\r\n",CSeq,range,session,UAnt);
	
	return strlen(pOut);
}

int RTSP_SetError(int iSock,char *pData,int iLen,int error,char *pOut)
{
	char CSeq[128]={0};
	char UAnt[128]={0};
	int  session;
	session = time((time_t *)NULL);
	searchStr(pData,CSeq,"CSeq:",2);
	searchStr(pData,UAnt,"User-Agent:",2);
	if (strlen(CSeq) == 0)
		return 0;
	
	if (error == 1)
	{
		if (strlen(UAnt) != 0)
			sprintf(pOut,"RTSP/1.0 403 Forbidden\r\n%sContent-Type: text/xml\r\n%sContent-Length: 0\r\n\r\n",CSeq,UAnt);
		else
			sprintf(pOut,"RTSP/1.0 403 Forbidden\r\n%sContent-Type: text/xml\r\nContent-Length: 0\r\n\r\n",CSeq);
	}
	else if (error == 2)
	{
		if (strlen(UAnt) != 0)
			sprintf(pOut,"RTSP/1.0 404 Not Found\r\n%sContent-Type: text/xml\r\n%sContent-Length: 0\r\n\r\n",CSeq,UAnt);
		else
			sprintf(pOut,"RTSP/1.0 404 Not Found\r\nContent-Type: text/xml\r\n%sContent-Length: 0\r\n\r\n",CSeq);
	}
	
	return strlen(pOut);
}
