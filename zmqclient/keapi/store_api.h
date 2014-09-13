#ifndef __STORE_API_H__
#define __STORE_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "store_define.h"

#ifdef __cplusplus
extern "C" {
#endif

int STORE_Initialize(void);								//初始化媒体编码
int STORE_Cleanup(void);								//反初始化媒体编码
short STORE_Get_Lib_Version(void);

int STORE_SaveDataToFile(char * path_name,char *pBuf,int datalen);//存储一张图片或一个文件，内容在pBuf中，path_name为绝对路径包括文件名
int STORE_Get_Disk_Free(int ctrl, int drive);			//获取磁盘剩余容量，返回值单位是M，ctrl和drive都填0
int STORE_Get_Disk_Capacity(int ctrl, int drive);		//获取磁盘总容量，返回值单位是M，ctrl和drive都填0
int STORE_Get_Disk_Status(int ctrl, int drive);			//获取当前磁盘的状态
int STORE_Get_Store_Status(int iChn,int *plan,int *alarmIN,int *alarmMD,int *manual);//获取通道的存储状态
int STORE_Set_Disk_Format(int ctrl, int drive);			//格式化
														//获取文件列表startTime endTime是查询起止的时间段，iChn是通道号0开始，enType查询图像的类型，iMaxListNum是最多查询的条数，stList是文件列表，空间由调用者分配
int STORE_Get_File_List(st_clock_t * startTime,st_clock_t *endTime,int iChn,e_store_type enType,int iMaxListNum,st_store_list_t * stList);	//返回的值为实际查询到的文件列表的条数

void STORE_Stop_Manual(int chans);						//停止手动录像chans表示录像的通道
void STORE_Start_Manual(int chans);						//开始手动录像chans表示录像的通道
void STORE_Start_Switch(int chans, int alarmInputNum);	//开始开关量报警录像chans表示录像的通道，alarmInputNum表示报警器的通道号
void STORE_Stop_Switch(int chans,int alarmInputNum);	//停止开关量报警录像chans表示录像的通道，alarmInputNum表示报警器的通道号
void STORE_Start_Motion(int chans,int area);			//开始移动侦测报警录像chans表示触发移动侦测的通道，不是需要录像的通道，area表示触发移动侦测报警的区域号
void STORE_Stop_Motion(int chans);						//停止移动侦测报警录像chans录像的通道
void STORE_Stop_Plan(int chans);						//开始计划录像chans表示录像的通道
void STORE_Start_Plan(int chans);						//停止计划录像chans表示录像的通道

#ifdef __cplusplus
}
#endif

#endif	//__STORE_API_H__
