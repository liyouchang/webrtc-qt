#include "common_api.h"
#include "common_define.h"
#include "media_api.h"
#include "media_define.h"

int netAppCallBack(void * pData)
{
	printf("%s:%d to do\n",__FUNCTION__,__LINE__);
	return 0;
}

int mediaAppCallBack(void * pData)
{
	printf("%s:%d to do\n",__FUNCTION__,__LINE__);
	return 0;
}
int inputAppCallBack(void * pData)
{
	printf("%s:%d to do\n",__FUNCTION__,__LINE__);
	return 0;
}
FILE * h264_video=NULL;
int H264_Get_CallBack(char * pFrameData,int iFrameLen)
{
	if (h264_video == NULL)
	{
		h264_video = fopen("/tmp/h264_video.h264","w+");
	}
	if (h264_video != NULL)
	{
		fwrite(pFrameData,1,iFrameLen,h264_video);
	}
	return 0;
}

int main(void)
{
	CONFIG_Initialize();
	//==============================================================
	struct NETPARAM	net;
	CONFIG_Register_Callback(CONFIG_TYPE_NET,netAppCallBack);
	CONFIG_Get(CONFIG_TYPE_NET,(void *)&net);
	printf("localIP:%d.%d.%d.%d\n",net.localIP[0],net.localIP[1],net.localIP[2],net.localIP[3]);
	net.localIP[2] = 35;
	CONFIG_Set(CONFIG_TYPE_NET,(void *)&net);
	CONFIG_Get(CONFIG_TYPE_NET,(void *)&net);
	printf("localIP:%d.%d.%d.%d\n",net.localIP[0],net.localIP[1],net.localIP[2],net.localIP[3]);
	//==============================================================
	struct MEDIAPARAM media;
	CONFIG_Register_Callback(CONFIG_TYPE_MEDIA,mediaAppCallBack);
	CONFIG_Get(CONFIG_TYPE_MEDIA,(void *)&media);
	printf("resolution %d frame_rate %d rate_ctrl_mode %d bitrate %d piclevel %d\n",media.main[0].resolution,media.main[0].frame_rate,media.main[0].rate_ctrl_mode,media.main[0].bitrate,media.main[0].piclevel);
	media.main[0].frame_rate = 15;
	CONFIG_Set(CONFIG_TYPE_MEDIA,(void *)&media);
	CONFIG_Get(CONFIG_TYPE_MEDIA,(void *)&media);
	printf("resolution %d frame_rate %d rate_ctrl_mode %d bitrate %d piclevel %d\n",media.main[0].resolution,media.main[0].frame_rate,media.main[0].rate_ctrl_mode,media.main[0].bitrate,media.main[0].piclevel);
	//==============================================================
	struct ALMINPUT input[2];
	CONFIG_Register_Callback(CONFIG_TYPE_INPUT,inputAppCallBack);
	CONFIG_Get(CONFIG_TYPE_INPUT,(void *)input);
	printf("TypeAlarmor %d CallCenter %d Enable %d AlmTime %d RecDelay %d\n",input[0].TypeAlarmor,input[0].CallCenter,input[0].Enable,input[0].AlmTime,input[0].RecDelay);
	input[0].Enable = 1;
	input[0].CallCenter = 1;
	CONFIG_Set(CONFIG_TYPE_INPUT,(void *)input);
	CONFIG_Get(CONFIG_TYPE_INPUT,(void *)input);
	printf("TypeAlarmor %d CallCenter %d Enable %d AlmTime %d RecDelay %d\n",input[0].TypeAlarmor,input[0].CallCenter,input[0].Enable,input[0].AlmTime,input[0].RecDelay);
	
	MEDIA_Initialize();
	sleep(1);
	char cBuf[2048];
	int iHandle = FIFO_Stream_Open(FIFO_STREAM_H264,0,0);
	FIFO_Register_Callback(FIFO_H264_MAIN,H264_Get_CallBack);
//	int iID = FIFO_Stream_RequestID(iHandle);
	while (1)
	{
//		if (FIFO_Stream_Read(iHandle,iID,cBuf) < 0)
			usleep(20000);
	}
	MEDIA_Cleanup();
	CONFIG_Cleanup();
	return 0;
}
