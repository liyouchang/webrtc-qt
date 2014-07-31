#ifndef COMMON_API_H
#define COMMON_API_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************/
//module config
/**********************************************************************/
int CONFIG_Initialize(void);
int CONFIG_Cleanup(void);
int CONFIG_Register_Callback(e_config_type enAttrId,CONFIG_CALLBACK config_callback);
int CONFIG_Get(e_config_type enAttrId,void * pData);
int CONFIG_Set(e_config_type enAttrId,void * pData);
/**********************************************************************/
//module clock
/**********************************************************************/
int CLOCK_Open(e_clock_type enType);
int CLOCK_Close(int handle);
int CLOCK_Get(int handle,st_clock_t *);
int CLOCK_Set(int handle,st_clock_t *);
int CLOCK_Get_Error(int handle);
int CLOCK_Version(void);										//1.00.00=0x10000  0.00.01=0x000001
int CLOCK_Set_NTP(int ip,short port,signed int zero);			//如果启用NTP校时，则必须先调用此接口设置NTP的服务器和时区
/**********************************************************************/
//module gpio
/**********************************************************************/
int GPIO_Open(char *devName);
int GPIO_Close(int handle);
int GPIO_Set_Dir(int gpio_group,int gpio_bit,int value);
int GPIO_Set_Value(int gpio_group,int gpio_bit,int value);
int GPIO_Get_Value(int gpio_group,int gpio_bit);
int GPIO_Control(int gpio_group,int gpio_bit,e_gpio_control enCtrl,int ms_value,int isHigh);
/**********************************************************************/
//module system
/**********************************************************************/
int SYSTEM_Initialize(void);
int SYSTEM_Cleanup(void);
int SYSTEM_Get_Mem(int *total,int *free);
int SYSTEM_Get_Cpu(void);
int SYSTEM_Set_Reboot(void);
int SYSTEM_Get_Run(void);
/**********************************************************************/
//module fifo
/**********************************************************************/
int FIFO_Initialize(void);
int FIFO_Create(st_fifo_t stFifo,int iCapacity);
int FIFO_Write(int iHandle,char *pData,int iLen);
int FIFO_Destory(int iHandle);
int FIFO_Request(st_fifo_t stFifo,int * pHandle,int second);
int FIFO_Get_MediaInfo(st_fifo_t stFifo,char * sps,int * sps_len,char *pps,int * pps_len);
int FIFO_Read(int iHandle,int rId,char *pData);
int FIFO_Release(int iHandle,int rId);
int FIFO_Cleanup(void);

int FIFO_Register_Callback(e_fifo_h264 enStreamChn,FIFO_CALLBACK fifo_callback);
int FIFO_Stream_Open(e_fifo_stream enStream,int iGroup,int iChn);		//iGroup和iChn从0开始

int FIFO_Stream_Set_(int iHandle,char *filename,int speed,int pulltime);
int FIFO_Stream_RequestID(int iHandle);
//int FIFO_Stream_Write(int iHandle,int iID,char *pData,int iLen);
int FIFO_Stream_Read(int iHandle,int iID,char *pData);
int FIFO_Stream_ReleaseID(int iHandle,int iID);
int FIFO_Stream_Close(int iHandle);
/**********************************************************************/
//module net
/**********************************************************************/
int TCP_Server_Create(short sPort,int iMaxConn,int iTimeout);
int TCP_Server_Wait(int handle,int *pSock,int iRcvSize,NET_RECEIVE_CALLBACK,NET_PROTOCOL_CALLBACK);
int TCP_Server_Send(int handle,int link,char *pData,int iLen);
int TCP_Server_GetLinkIP(int handle,int link,int ip);
int TCP_Server_Finish(int handle,int link);
int TCP_Server_Destory(int handle);

int TCP_Client_Create(int iIp,short sPort,int iTimeout,int iRcvSize,NET_RECEIVE_CALLBACK,NET_PROTOCOL_CALLBACK);
int TCP_Client_BindPYH(int iSock,char * phy);
int TCP_Client_Send(int iSock,char *pData,int iLen);
int TCP_Client_Destory(int iSock);

int UDP_Server_Create(short sPort,int iRcvSize,UDP_PROTOCOL_CALLBACK);
int UDP_Server_Send(int iSock,int iIP,short sPort,char *pData,int iLen);
int UDP_Server_Destory(int iSock);

int NET_Initialize(void);
int NET_Get_WanStatus(void);
int NET_Get_RouteIP(char * cIP);
int NET_Get_Status(st_net_status_t * status);
int NET_Cleanup(void);

int RTSP_Options(int iSock,char *pData,int iLen,char *pOut);
int RTSP_Describe(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut);
int RTSP_Setup(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut);
int RTSP_Play(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut);
int RTSP_Pause(int iSock,char *pData,int iLen,char *pOut);
int RTSP_Teardown(int iSock,char *pData,int iLen,char *pOut);
int RTSP_SetParameter(int iSock,char *pData,int iLen,char *pOut);
int RTSP_SetError(int iSock,char *pData,int iLen,int error,char *pOut);

int gethostbyname_my(char * host,char * ip);
int searchStr(char *src,char *dst,char *content,int add);
/**********************************************************************/
//module Uart
/**********************************************************************/
int UART_Open(char * pName,UART_RECEIVE_CALLBACK receive,UART_PROTOCOL_CALLBACK protocol);
int UART_Set_Attr(int handle,st_uart_attr_t * attr,st_485_ctrl_t *ctrl);
int UART_Send(int handle,char * pData,int iLen);
int UART_Close(int handle);
/**********************************************************************/
//module Utility
/**********************************************************************/
int UTILITY_MD5(unsigned char *Src, unsigned char *dst, int len);
int UTILITY_HMAC1_IOV(st_iov_t* iov,int iov_num,unsigned char* key,int key_len,unsigned char* dst);
int UTILITY_BASE64_Encode(unsigned char *src,int len,char *dst);
int UTILITY_MEDIA_Convert_Size(int resolution,int *width,int *high);
int UTILITY_MEDIA_Convert_Format(int reso);
int UTILITY_String_IP(char *src,char *cip,int *iip);
int UTILITY_B64_ntop(unsigned char const *,size_t, char *,size_t);
int UTILITY_B64_pton(char const *,unsigned char *,size_t);
#ifdef __cplusplus
}
#endif

#endif		//COMMON_API_H
