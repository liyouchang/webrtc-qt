#include "p8563.h"

static st_clock_hanndle_t * g_stp_handle=NULL;

int open_pcf8563(st_clock_hanndle_t * p_st_handle)
{
	if (g_stp_handle == NULL)
	{
		g_stp_handle = p_st_handle;
		p_st_handle->enType = CLOCK_TYPE_PCF8563;
		p_st_handle->iHandle = open("/dev/pcf8563", O_RDWR);
		if (p_st_handle->iHandle < 0)
		{
			perror("");
			printf("open /dev/hi_rtc failed\n");
			p_st_handle->iErrorNo = CLOCK_OPEN_FAILD;
			return p_st_handle->iErrorNo;
		}
		char time[16];
		ioctl(p_st_handle->iHandle,PCF8563_GETTIME,time);
		close(p_st_handle->iHandle);
		int i;
		for (i=0; i<7; i++)
		{
		    time[15] = time[i];
			time[i] = (time[15]>>4)*10+(time[15]&0x0f);
		}
		printf("PCF8563 READ TIME:%d-%d-%d %d:%d:%d\n",time[0]+2000,time[1],time[2],time[3],time[4],time[5]);
		p_st_handle->systemTime[0] = time[0]+2000;
		p_st_handle->systemTime[1] = time[1];
		p_st_handle->systemTime[2] = time[2];
		p_st_handle->systemTime[3] = time[3];
		p_st_handle->systemTime[4] = time[4];
		p_st_handle->systemTime[5] = time[5];
		p_st_handle->systemTime[6] = time[6];
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
	return (CLOCK_TYPE_PCF8563|0x80);
}

int close_pcf8563(st_clock_hanndle_t * p_st_handle)
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

int read_pcf8563(st_clock_t * p_st_clock)
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

int write_pcf8563(st_clock_t * p_st_clock)
{
	if (g_stp_handle != NULL)
	{
		g_stp_handle->iHandle = open("/dev/pcf8563", O_RDWR);
		if (g_stp_handle->iHandle < 0)
		{
			perror("");
			printf("open /dev/hi_rtc failed\n");
			g_stp_handle->iErrorNo = CLOCK_OPEN_FAILD;
			return g_stp_handle->iErrorNo;
		}
		char cTime[16];
		cTime[0] = (p_st_clock->year%2000);
		cTime[1] = p_st_clock->month;
		cTime[2] = p_st_clock->day;
		cTime[3] = p_st_clock->hour;
		cTime[4] = p_st_clock->minute;
		cTime[5] = p_st_clock->second;
		cTime[6] = p_st_clock->week;
		int i;
		for(i=0; i<7; i++)
            cTime[i] = ((cTime[i]/10)<<4)+cTime[i]%10;
        ioctl(g_stp_handle->iHandle,PCF8563_SETTIME,cTime);
        close(g_stp_handle->iHandle);
        printf("RTC SET TIME:%d-%d-%d %d:%d:%d\n",cTime[0]+2000,cTime[1],cTime[2],cTime[3],cTime[4],cTime[5]);
        int sys_time[8];
		sys_time[0] = p_st_clock->year;
		sys_time[1] = p_st_clock->month;
		sys_time[2] = p_st_clock->day;
		sys_time[3] = p_st_clock->minute;
		sys_time[4] = p_st_clock->second;
		sys_time[5] = p_st_clock->second;
		sys_time[6] = p_st_clock->week;		
		setClockToSystem(sys_time);
		g_stp_handle->iErrorNo = CLOCK_SUCESS;
	}
	else
		return CLOCK_NONE_EXIST;
		
	return CLOCK_SUCESS;
}

