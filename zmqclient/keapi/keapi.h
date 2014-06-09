#ifndef __MY_KEAPI_H__
#define __MY_KEAPI_H__

#ifdef __cplusplus
extern "C" {               
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define REC_PATH_LEN 40
#define REC_DATE_LEN 40
#define MAX_REC_QUERYNUM 80

#define MAX_NVR_QUERYNUM   30
#define NVR_PATH_LEN   128
#define NVR_DATE_LEN   16

#define DEBUG_PRINT_ENABLE
#if defined(DEBUG_PRINT_ENABLE)

#define DBG_PRINT(format,...)   printf(format,##__VA_ARGS__);printf("\n")
#define DBG_PRINT_FILE_FUNC_LINE  printf("file = %s  func = %s line = %d \n",__FILE__,__FUNCTION__,__LINE__)
#define DBG_PRINT_MEM(buf,len)       {int  i =0;char* p=(char*)buf;for(;i<len;printf("%x ",p[i]),i++);printf("\n");}
#define DBG_PRINT_MEM_C(buf,len)       {int  i =0;char* p=(char*)buf;for(;i<len;printf("%c ",p[i]),i++);printf("\n");}
#define DBG_PRINT_PTR(ptr)  printf("pointer to %p", ptr);printf("\n")
#define DBG_PRINT_TIME(time)         { printf("year =%d month =%d day=%d hour=%d min=%d sec=%d",time.year,time.mon,time.day,time.hour,time.min,time.sec);}

#else
#define DBG_PRINT(format,...)
#define DBG_PRINT_FILE_FUNC_LINE
#define DBG_PRINT_MEM(buf,len)
#define DBG_PRINT_MEM_C(buf,len)
#define DBG_PRINT_PTR(ptr)
#define DBG_PRINT_TIME(time)
#endif


typedef struct
{ 
	char path[NVR_PATH_LEN];
	char date[NVR_DATE_LEN];
	int  size;
}t_VidRec_FileInfo, *pt_VidRec_FileInfo;

typedef struct
{ 
	int file_num;
    t_VidRec_FileInfo rec_file[MAX_NVR_QUERYNUM];
}t_VidRecFile_QueryInfo, *pt_VidRecFile_QueryInfo;

typedef struct
{
	char sSsID[32];
	char sKey[32];
	int u32Enable;
	int u32Auth;
	int u32Enc;
	int u32Mode;
}t_WIFI_PARAM,*pt_WIFI_PARAM;
typedef struct
{
	char sSsID[32];
	unsigned int u32Quality;
	unsigned int u32Auth;
	unsigned int u32Enc;
	unsigned int u32Mode;
}t_WIFI_INFO, *pt_WIFI_INFO;

int Raycomm_InitParam(void);
int Raycomm_UnInitParam(void);
int Raycomm_GetParam(char *keyword,char *buf,int cam_id);
int Raycomm_SetParam(char *command,int cam_id);
//int IPC_QueryVideoRecod(char* start_day,char* end_day,pt_VidRecFile_QueryInfo pt_vidrecfile_queryinfo,int offset,int usCount);
int Raycomm_QueryNVR(char* start_day,char* end_day,pt_VidRecFile_QueryInfo pt_vidrecfile_queryinfo,int offset,int usCount);
int Raycomm_SetWifi(pt_WIFI_PARAM params);
int Raycomm_GetWifiList(int* pCount, pt_WIFI_INFO wifi);
int Raycomm_MediaDataInit(void);
int Raycomm_MediaDataUnInit(void);
int Raycomm_ConnectMedia(const char* keyword,int cam_id);
int Raycomm_GetMediaData(int handle,char* buf,int max_size,unsigned int* timestamp);
int Raycomm_DisConnectMedia(int handle);
int Raycomm_ForceIframe(int handle );
//int IPC_TalkPlay(const char* keyword,char* buf,int len,unsigned int timestamp,unsigned int audiotype);
int Raycomm_TalkPlay (int handle, char* buf, int len, unsigned int timestamp, unsigned int audiotype);
int Raycomm_SetPtz(const char* keyword,int preset,int cam_id);
//通道号   chn = 1.2.3.4
//移动侦测 rea = 1
//遮挡报警 rea = 4
//开关量   rea = 5
//门磁     rea = 2，io = 20
//人体红外 rea = 2，io = 21
//烟感报警 rea = 2，io = 22
typedef int (*NOTIFY_CALLBACK)(int chn,int rea,int io,int snapcount,int snapsize,char* snapbuf);
int Raycomm_Register_Callback(NOTIFY_CALLBACK notify_callback);

//typedef int (*SNAP_CALLBACK)(int iJpgSize, char* pBuf);
//int Raycomm_SNAP_Callback(SNAP_CALLBACK snap_callback);

int Raycomm_GetIP();

int Raycomm_GetNetType();

int Raycomm_Reboot(void);

int Raycomm_SetTitle(char* pChnName);

int Raycomm_SetAlarmEnable(char bEnable);


#ifdef __cplusplus
}
#endif
#endif
