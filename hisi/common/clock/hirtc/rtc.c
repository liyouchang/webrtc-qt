#include "rtc.h"

static st_clock_hanndle_t * g_stp_handle=NULL;
int open_hirtc(st_clock_hanndle_t * p_st_handle)
{
	if (g_stp_handle == NULL)
	{
		g_stp_handle = p_st_handle;
		p_st_handle->enType = CLOCK_TYPE_HIRTC;
		p_st_handle->iHandle = open("/dev/hi_rtc", O_RDWR);
		if (p_st_handle->iHandle < 0)
		{
			perror("");
			printf("open /dev/hi_rtc failed\n");
			p_st_handle->iErrorNo = CLOCK_OPEN_FAILD;
			return p_st_handle->iErrorNo;
		}
		rtc_time_t tm;
		ioctl(p_st_handle->iHandle,HI_RTC_RD_TIME,&tm);
		close(p_st_handle->iHandle);
		printf("RTC READ TIME:%d-%d-%d %d:%d:%d\n",tm.year,tm.month,tm.date,tm.hour,tm.minute,tm.second);
		p_st_handle->systemTime[0] = tm.year;
		p_st_handle->systemTime[1] = tm.month;
		p_st_handle->systemTime[2] = tm.date;
		p_st_handle->systemTime[3] = tm.hour;
		p_st_handle->systemTime[4] = tm.minute;
		p_st_handle->systemTime[5] = tm.second;
		p_st_handle->systemTime[6] = tm.weekday;
		setClockToSystem(p_st_handle->systemTime);
		if (mutex_system_clock == 0)
		{
			mutex_system_clock = 1;
			p_st_handle->iThreadCtrl = 1;
			pthread_create(&p_st_handle->thClock,NULL,thread_system_clock,(void *)p_st_handle);
		}
		else
			p_st_handle->iThreadCtrl = 0;
		p_st_handle->iErrorNo = CLOCK_SUCESS;
	}
	else
		p_st_handle->iErrorNo = CLOCK_ALREADY_USED;
	return (CLOCK_TYPE_HIRTC|0x80);	
}

int close_hirtc(st_clock_hanndle_t * p_st_handle)
{
	if (g_stp_handle != NULL)
	{
		if (p_st_handle->iThreadCtrl == 1)
		{
			mutex_system_clock = 0;
			p_st_handle->iThreadCtrl = 0;
			pthread_join(p_st_handle->thClock,NULL);
		}
		g_stp_handle = NULL;
		p_st_handle->iErrorNo = CLOCK_SUCESS;
	}
	else
		p_st_handle->iErrorNo = CLOCK_NONE_EXIST;
	return p_st_handle->iErrorNo;
}

int read_hirtc(st_clock_t * p_st_clock)
{
	if (g_stp_handle != NULL)
	{
		p_st_clock->year = g_stp_handle->systemTime[0];
		p_st_clock->month = g_stp_handle->systemTime[1];
		p_st_clock->day = g_stp_handle->systemTime[2];
		p_st_clock->hour = g_stp_handle->systemTime[3];
		p_st_clock->minute = g_stp_handle->systemTime[4];
		p_st_clock->second = g_stp_handle->systemTime[5];
	}
	else
		return CLOCK_NONE_EXIST;
	return CLOCK_SUCESS;
}

int write_hirtc(st_clock_t * p_st_clock)
{
	if (g_stp_handle != NULL)
	{
		g_stp_handle->iHandle = open("/dev/hi_rtc", O_RDWR);
		if (g_stp_handle->iHandle < 0)
		{
			perror("");
			printf("open /dev/hi_rtc failed\n");
			g_stp_handle->iErrorNo = CLOCK_OPEN_FAILD;
			return g_stp_handle->iErrorNo;
		}
		rtc_time_t tm;
		tm.year = p_st_clock->year;
		tm.month = p_st_clock->month;
		tm.date = p_st_clock->day;
		tm.hour = p_st_clock->hour;
		tm.minute = p_st_clock->minute;
		tm.second = p_st_clock->second;
		tm.weekday = p_st_clock->week;
		printf("RTC SET TIME:%d-%d-%d %d:%d:%d week %d\n",tm.year,tm.month,tm.date,tm.hour,tm.minute,tm.second,tm.weekday);
		ioctl(g_stp_handle->iHandle,HI_RTC_SET_TIME,&tm);
		close(g_stp_handle->iHandle);
		int sys_time[8];
		sys_time[0] = p_st_clock->year;
		sys_time[1] = p_st_clock->month;
		sys_time[2] = p_st_clock->day;
		sys_time[3] = p_st_clock->hour;
		sys_time[4] = p_st_clock->minute;
		sys_time[5] = p_st_clock->second;
		sys_time[6] = p_st_clock->week;		
		setClockToSystem(sys_time);
		g_stp_handle->iErrorNo = CLOCK_SUCESS;
	}
	else
		return CLOCK_NONE_EXIST;
		
	return CLOCK_SUCESS;
}
