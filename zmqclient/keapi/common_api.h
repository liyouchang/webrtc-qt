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
int CONFIG_Initialize(void);															//初始化参数配置模块
int CONFIG_Cleanup(void);																//关闭参数配置模块
int CONFIG_Register_Callback(e_config_type enAttrId,CONFIG_CALLBACK config_callback);	//注册配置参数的回调函数
int CONFIG_Get(e_config_type enAttrId,void * pData);									//获取某类参数
int CONFIG_Set(e_config_type enAttrId,void * pData);									//设置某类参数
/**********************************************************************/
//module clock
/**********************************************************************/
int CLOCK_Open(e_clock_type enType);													//启动时钟功能
int CLOCK_Close(int handle);															//关闭时钟功能
int CLOCK_Get(st_clock_t *);															//获取当前时间
int CLOCK_Set(st_clock_t *);															//设置系统时间
int CLOCK_Get_Error(int handle);
int CLOCK_Version(void);																//1.00.00=0x10000  0.00.01=0x000001
int CLOCK_Set_NTP(int ip,short port,signed int zero);									//如果启用NTP校时，则必须先调用此接口设置NTP的服务器和时区
/**********************************************************************/
//module gpio
/**********************************************************************/
int GPIO_Open(char *devName);															//打开gpio设备
int GPIO_Close(int handle);																//关闭gpio设备
int GPIO_Set_Dir(int gpio_group,int gpio_bit,int value);								//设置某个gpio的输入输出功能0-input 1-output
int GPIO_Set_Value(int gpio_group,int gpio_bit,int value);								//控制输出模式的gpio输出value状态
int GPIO_Get_Value(int gpio_group,int gpio_bit);										//获取输入模式的gpio口的当前状态
int GPIO_Control(int gpio_group,int gpio_bit,e_gpio_control enCtrl,int ms_value,int isHigh);//控制输出模式的gpio按照参数规律输出
/**********************************************************************/
//module system
/**********************************************************************/
int SYSTEM_Initialize(void);															//初始化系统
int SYSTEM_Cleanup(void);																//系统模块退出
int SYSTEM_Get_Mem(int *total,int *free);												//获取内存总容量，剩余容量和占用率
int SYSTEM_Get_Cpu(void);																//获取cpu占用率
int SYSTEM_Set_Reboot(void);															//执行reboot
int SYSTEM_Get_Run(void);																//获取当前是否要reboot的标志
int SYSTEM_Command(char *command);														//系统命令调用，封装了system在执行时加锁
/**********************************************************************/
//module fifo
/**********************************************************************/
int FIFO_Initialize(void);																//初始化FIFO功能模块
int FIFO_Create(st_fifo_t stFifo,int iCapacity);										//创建一个FIFO
int FIFO_Write(int iHandle,char *pData,int iLen);										//向FIFO的缓冲区里面写入数据
int FIFO_Destory(int iHandle);															//销毁创建的FIFO
int FIFO_Request(st_fifo_t stFifo,int * pHandle,int second);							//请求一个FIFO的读指针,此接口会返回所申请的FIFO的句柄
int FIFO_Get_MediaInfo(st_fifo_t stFifo,char * sps,int * sps_len,char *pps,int * pps_len);//获取H264的sps，pps
int FIFO_Read(int iHandle,int rId,char *pData);											//读取FIFO的数据
int FIFO_Release(int iHandle,int rId);													//释放请求的FIFO读指针
int FIFO_Cleanup(void);																	//FIFO模块关闭

int FIFO_Register_Callback(e_fifo_h264 enStreamChn,FIFO_CALLBACK fifo_callback);		//注册纯H264码流的回调函数

int FIFO_Stream_Open(e_fifo_stream enStream,int iGroup,int iChn,int second);			//打开某种类型码流的转换线程，如果是纯H264码流从回调反馈给调用者，每次返回一帧，iGroup和iChn从0开始
int FIFO_Stream_Set_(int iHandle,char *filename,int speed,int pulltime);				//设置一些特殊的参数到转换线程
int FIFO_Stream_RequestID(int iHandle);													//请求一个读指针
//int FIFO_Stream_Write(int iHandle,int iID,char *pData,int iLen);
int FIFO_Stream_Read(int iHandle,int iID,char *pData);									//读取转换后的流数据
int FIFO_Stream_ReleaseID(int iHandle,int iID);											//释放读指针
int FIFO_Stream_Close(int iHandle);														//停止码流的转换

void FIFO_Stream_Get_AviHead(char *head_buf,int *ptr,int reso,int frame);
void FIFO_Stream_Get_HeadIndex(char *index_buf,int *index_len,int len);
void FIFO_Stream_Get_DataIndex(int isAudio,char idrFlag,char *index_buf,int *index_len,int len);

int FIFO_Alarm_Write(st_alarm_upload_t *cInfo);											//写入一条报警消息
int FIFO_Alarm_Get_Wpoint(e_fifo_read enRead);														//获取当前的报警写指针
int FIFO_Alarm_Read(int iReader,st_alarm_upload_t *cInfo);								//
/**********************************************************************/
//module net
/**********************************************************************/
int TCP_Server_Create(short sPort,int iMaxConn,int iTimeout);							//创建一个tcp的server
int TCP_Server_Wait(int handle,int *pSock,int iRcvSize,NET_RECEIVE_CALLBACK,NET_PROTOCOL_CALLBACK);//tcp server等待一个连接 数据接收和协议处理在回调中完成
int TCP_Server_Send(int handle,int link,char *pData,int iLen);							//在某个连接上发送数据
int TCP_Server_GetLinkIP(int handle,int link,int ip);									//获取连接过来的client的ip
int TCP_Server_Finish(int handle,int link);												//断开某个连接
int TCP_Server_Destory(int handle);														//销毁tcp server。

int TCP_Client_Create(int iIp,short sPort,int iTimeout,int iRcvSize,NET_RECEIVE_CALLBACK,NET_PROTOCOL_CALLBACK);//创建一个tcp client 数据接收和协议处理在回调中完成
int TCP_Client_BindPYH(int iSock,char * phy);											//tcp client端绑定某个网卡，主要为无线设备准备的接口
int TCP_Client_Send(int iSock,char *pData,int iLen);									//在此连接上发送数据
int TCP_Client_Destory(int iSock);														//断开tcp连接，销毁tcp client

int UDP_Server_Create(short sPort,int iRcvSize,UDP_PROTOCOL_CALLBACK);					//创建一个UDP server
int UDP_Server_Send(int iSock,int iIP,short sPort,char *pData,int iLen);				//发送udp数据
int UDP_Server_Destory(int iSock);														//销毁udp server

int NET_Initialize(void);																//初始化系统网络
int NET_Get_WanStatus(void);															//由此接口来判断设备是否能够连接到公网
int NET_Get_RouteIP(char * cIP);														//获取当前路由使用的网卡的ip
int NET_Get_Status(st_net_status_t * status);											//获取各个网卡的状态
int NET_Get_WifiList(int iMaxList,st_wifi_list_t *);									//获取wifi网络列表 最多查询iMaxList条放入wifi列表里面，返回实际查询出来的条数
int NET_Set_Wifi(st_wifi_list_t *);														//设置连接wifi参数
int Net_Get_WifiLink_Status(void);														//设置wifi连接的状态，0-未开始+未使能(插着网线同时wifi未设置)，1-未开始+已使能(wifi参数已经设置但插着网线)，2-开始+未使能(网线拔掉了但wifi未设置)，3-开始连接或者正在连接，4-连接成功
int NET_Cleanup(void);																	//反初始化系统网络

int RTSP_Options(int iSock,char *pData,int iLen,char *pOut);							//
int RTSP_Describe(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut);
int RTSP_Setup(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut);
int RTSP_Play(int iSock,char *pData,int iLen,e_rtsp_type enRtsp,void *pAttr,char *pOut);
int RTSP_Pause(int iSock,char *pData,int iLen,char *pOut);
int RTSP_Teardown(int iSock,char *pData,int iLen,char *pOut);
int RTSP_SetParameter(int iSock,char *pData,int iLen,char *pOut);
int RTSP_SetError(int iSock,char *pData,int iLen,int error,char *pOut);

int gethostbyname_my(char * host,char * ip);											//获取域名的ip
int searchStr(char *src,char *dst,char *content,int add);								//获取文本字符串
/**********************************************************************/
//module Uart
/**********************************************************************/
int UART_Open(char * pName,UART_RECEIVE_CALLBACK receive,UART_PROTOCOL_CALLBACK protocol);//打开串口
int UART_Set_Attr(int handle,st_uart_attr_t * attr,st_485_ctrl_t *ctrl);				//设置串口属性
int UART_Send(int handle,char * pData,int iLen);										//从串口发送数据
int UART_Close(int handle);																//关闭串口
/**********************************************************************/
//module Motor
/**********************************************************************/
int MOTOR_Send(char * pData,int iLen);
//iCmd取值：停止0,上1,下2,左3,右4,左上5,左下6,右上7,右下8,//光圈自动9,光圈开10 open,光圈关11 close,光圈变化停止12,
//焦点近13 near,焦点远14 far,焦点变化停止15,变倍小16 IN,变倍大17 OUT,变倍变化停止18,//自动开始19,自动停止20,
//雨刷开21,雨刷关22,灯光开23,灯光关24,调用预置点25,设置预置点26,清除预置点27,
//模式开28,模式关29,运行模式30,180度翻转31,清除所有预置位32,清除所有预置位33
//iSpeed云台转动速度，默认63
//iParam预置位
int MOTOR_Control(int iChn, int iCmd,int iSpeed,int iParam);
/**********************************************************************/
//module Utility
/**********************************************************************/
int UTILITY_MD5(unsigned char *Src, unsigned char *dst, int len);						//MD5加密算法
int UTILITY_HMAC1_IOV(st_iov_t* iov,int iov_num,unsigned char* key,int key_len,unsigned char* dst);//哈希加密算法
int UTILITY_BASE64_Encode(unsigned char *src,int len,char *dst);						//base64加密算法
int UTILITY_MEDIA_Convert_Size(int resolution,int *width,int *high);					//分辨率转换成宽高	
int UTILITY_MEDIA_Convert_Format(int reso);												//编码参数的分辨率（由小到大表示分辨率由小到大),转换成卡尔协议使用的分辨率的定义规则
int UTILITY_MEDIA_Convert_Resolution(int format);										//卡尔协议传下来的参数转化成程序中实际应用的分辨率（由小到大表示分辨率由小到大)
int UTILITY_B64_ntop(unsigned char const *,size_t, char *,size_t);
int UTILITY_B64_pton(char const *,unsigned char *,size_t);
int UTILITY_String_IP(char *src,char *cip,int *iip);									//字符串ip转换成4字节十六进制ip或int类型ip
int UTILITY_String_MAC(char *src,char *char_mac,char *hex_mac);							//字符串mac转换成小写mac字符串，和十六进制mac

#ifdef __cplusplus
}
#endif

#endif		//COMMON_API_H
