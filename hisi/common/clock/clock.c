#include "clock.h"
#include "ntp/ntp.h"
#include "hirtc/rtc.h"
#include "p8563/p8563.h"
#include "../common_api.h"

//=============================================================================//

int mutex_system_clock=0;					//防止多个地方启动读时间的线程
int clock_open_type = -1;
static st_clock_hanndle_t stHandle[3];
//=============================================================================//

int CLOCK_Open(e_clock_type enType)
{
	int iRet;
	if (clock_open_type == -1)
	{
		stHandle[0].enType = -1;
		stHandle[1].enType = -1;
		stHandle[2].enType = -1;
	}
	if (enType == CLOCK_TYPE_HIRTC)
	{	
		iRet = open_hirtc(&stHandle[(int)enType]);
		clock_open_type = CLOCK_TYPE_HIRTC;
	}
	else if (enType == CLOCK_TYPE_PCF8563)
	{	
		iRet = open_pcf8563(&stHandle[(int)enType]);
		if ((clock_open_type==-1)||(clock_open_type>0))
			clock_open_type = CLOCK_TYPE_PCF8563;
	}
	else
	{	
		iRet = open_ntp(&stHandle[(int)enType]);
		if (clock_open_type==-1)
			clock_open_type = CLOCK_TYPE_NTP;
	}
	return iRet;
}

int CLOCK_Close(int handle)
{
	int iRet;
	if (stHandle[handle&0x7f].enType == CLOCK_TYPE_HIRTC)
		iRet = close_hirtc(&stHandle[(int)stHandle[handle&0x7f].enType]);
	else if (stHandle[handle&0x7f].enType == CLOCK_TYPE_PCF8563)
		iRet = close_pcf8563(&stHandle[(int)stHandle[handle&0x7f].enType]);
	else
		iRet = close_ntp(&stHandle[(int)stHandle[handle&0x7f].enType]);
	return iRet;
}

int CLOCK_Get(st_clock_t * p_st_clock)
{
	int * pTime = stHandle[clock_open_type].systemTime;
	p_st_clock->year = pTime[0];
	p_st_clock->month = pTime[1];
	p_st_clock->day = pTime[2];
	p_st_clock->hour = pTime[3];
	p_st_clock->minute = pTime[4];
	p_st_clock->second = pTime[5];
	p_st_clock->week = pTime[6];
	return 0;
}

int CLOCK_Set(st_clock_t * p_st_clock)
{
	int iRet=-1;
	if (clock_open_type != -1)
	{
		if (stHandle[0].enType != -1)
			iRet = write_hirtc(p_st_clock);
		if (stHandle[1].enType != -1)
			iRet = write_pcf8563(p_st_clock);
		if (stHandle[2].enType != -1)
			iRet = write_ntp(p_st_clock);
	}
	return iRet;
}

int CLOCK_Get_Error(int handle)
{
	return stHandle[handle&0x7f].iErrorNo;
}

int CLOCK_Version(void)
{
	return CLOCK_MODULE_VERSION;
}

int CLOCK_Set_NTP(int ip,short port,signed int zero)
{
	write_ntp_server(ip,port,zero);
	return CLOCK_SUCESS;
}

void setClockToSystem(int * systemTime)
{
	time_t sysTime;
	struct tm tm_time;
	if ((systemTime[0]>0)&&(systemTime[0]<2000))
		tm_time.tm_year = (systemTime[0]+100)%2000;
	else
		tm_time.tm_year = (systemTime[0]%2000)+100;
	tm_time.tm_mon  = systemTime[1] - 1;
	tm_time.tm_mday = systemTime[2];
	tm_time.tm_hour = systemTime[3];
	tm_time.tm_min  = systemTime[4];
	tm_time.tm_sec  = systemTime[5];
	sysTime = mktime(&tm_time);
	stime(&sysTime);
    printf("SYSTEM SET TIME:%d-%d-%d %d:%d:%d\n",tm_time.tm_year+1900,tm_time.tm_mon+1,tm_time.tm_mday,tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec);
    int * pTime = stHandle[clock_open_type].systemTime;
    pTime[0] = systemTime[0];
	pTime[1] = systemTime[1];
	pTime[2] = systemTime[2];
	pTime[3] = systemTime[3];
	pTime[4] = systemTime[4];
	pTime[5] = systemTime[5];
	pTime[6] = systemTime[6];	
}

void * thread_system_clock(void * hirtc)
{
	st_clock_hanndle_t * this =(st_clock_hanndle_t *)hirtc;
	struct tm Cur_t;
	time_t 	  Time_t;
	
	while(this->iThreadCtrl == 1)
	{
		time(&Time_t);
		localtime_r(&Time_t,&Cur_t);
		this->systemTime[0] = Cur_t.tm_year-100+2000;
		this->systemTime[1] = Cur_t.tm_mon + 1;
		this->systemTime[2] = Cur_t.tm_mday;
		this->systemTime[3] = Cur_t.tm_hour;
		this->systemTime[4] = Cur_t.tm_min;
		this->systemTime[5] = Cur_t.tm_sec;
		this->systemTime[6] = Cur_t.tm_wday;
		
		usleep(80000);
	}
	return hirtc;
}
