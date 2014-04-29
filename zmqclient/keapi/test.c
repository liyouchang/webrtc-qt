#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include "keapi.h"

int main(void )
{
//	InitParam();
//	while(getRebootStatus()==0)
//	    sleep(1);
//	UnInitParam();
	int fd=-1;
	int iCnt;
	FILE * f;
	int iLen;
	unsigned int stamp;
	char *buf=(char *)malloc(0x100000);
	Raycomm_InitParam();
//	int Raycomm_GetWifiList(int* pCount, pt_WIFI_INFO wifi);
	int iWifiCnt;
	t_WIFI_INFO wifi[128];
	Raycomm_GetWifiList(&iWifiCnt,wifi);
	printf("iWifiCnt %d\n",iWifiCnt);
	Raycomm_SetParam("wifi_ssid=adddd",0);
	while(1)
	{
		t_VidRecFile_QueryInfo fileinfo;
		int iRecTotal = Raycomm_QueryNVR("20140428000001","20140428235959",&fileinfo,0,20);
		printf("iRecTotal %d\n",iRecTotal);
		
		iCnt = 0;
		Raycomm_MediaDataInit();
		fd = Raycomm_ConnectMedia("video2_data",0);
		while(1)
		{
			iLen = Raycomm_GetMediaData(fd,buf,0x100000,&stamp);
			if (iLen > 0)
			{
				if (f == NULL)
					f = fopen("/tmp/video.h264","w+");
				if (f != NULL)
					fwrite(buf,1,iLen,f);
				printf("iCnt %d\n",iCnt);
				iCnt++;
				if (iCnt >=300)
				{	
					fclose(f);
					f = NULL;
					break;
				}
			}
			else
				usleep(10);
			
		}
		Raycomm_DisConnectMedia(fd);
		fd = Raycomm_ConnectMedia("audio1_data",0);
		while(1)
		{
			iLen = Raycomm_GetMediaData(fd,buf,0x100000,&stamp);
			if (iLen > 0)
			{
				if (f == NULL)
					f = fopen("/tmp/audio.g711","w+");
				if (f != NULL)
					fwrite(buf,1,iLen,f);
				//printf("iCnt %d\n",iCnt);
				iCnt++;
				if (iCnt >=1000)
				{	
					fclose(f);
					f = NULL;
					break;
				}
			}
			else
				usleep(10);
			
		}
		Raycomm_DisConnectMedia(fd);
		while(1)
		{	
			usleep(100000);
		}
		
		Raycomm_MediaDataUnInit();
		sleep(1);
	}
	Raycomm_UnInitParam();
	free(buf);
}