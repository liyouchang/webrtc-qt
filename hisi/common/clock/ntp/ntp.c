#include "ntp.h"

static st_clock_hanndle_t * g_stp_handle=NULL;

#define NTPPORT 22618
int ntp_ip=-1;
int ntp_port=0;
signed int ntp_zero;
/***************************************************************************
 *	函数名称：	SecondToDateTime
 *	功能描述：	将相对2000-1-1 00:00:00 的以秒为单位的时间戳转换成对应时间
 *	日    期：	2008-09-01 16:51:53
 *	作    者：	袁志强
 *	参数说明：
 *	返 回 值：
 ***************************************************************************/
void SecondToDateTime(unsigned char *time, unsigned int Day, unsigned int Second)
{
	int a, c, w, y;
	w = 2000;
	a = Day;

	while(a > 365)
	{
		if ((w%4==0 && w%100!=0) || (w%400==0))//闰年
			a = a - 366;
		else
			a = a - 365;
		w++;
	}
	if ((a==365) && (w>2000))
	{
		if (!(((w)%4==0 && (w)%100!=0) || ((w)%400==0)))//闰年
		{
			a = a - 365;
			w++;
		}
	}
	c = a;
	y = w;
	if (Day < 1)
	{
		time[0] = 0;
		time[1] = 1;
		time[2] = 1;
	}
	else
	{
		time[0] = y - 2000;
		if( (y%4==0 && y%100!=0)||(y%400==0))//闰年
		{
			  if ( c <= 30 )
			  {
	       			time[1] = 1;
	       			time[2] = c + 1;
			  }
			  else if ((30<c) && (c<=59))
			  {
	       			time[1] = 2;
	      			time[2] = c - 30;
			  }
			  else if ((59<c) && (c<=90))
			  {
	       			time[1] = 3;
	      			time[2]= c - 59;
			  }
			  else if ((90<c) && (c<=120))
			  {
	       			time[1] = 4;
	       			time[2] = c - 90;
			  }
			  else if ((120<c) && (c<=151))
			  {
	       			time[1] = 5;
	      			time[2] = c - 120;
			  }
			  else if ((151<c) && (c<=181))
			  {
	       			time[1] = 6;
	      			time[2] = c - 151;
			  }
			  else if ((181<c) && (c<=212))
			  {
	       			time[1] = 7;
	      			time[2] = c - 181;
			  }
			  else if ((212<c) && (c<=243))
			  {
	       			time[1] = 8;
	      			time[2] = c - 212;
			  }
			  else if ((243<c) && (c<=273))
			  {
	       			time[1] = 9;
	      			time[2] = c - 243;
			  }
			  else if ((273<c) && (c<=304))
			  {
	       			time[1] = 10;
	      			time[2] = c - 273;
			  }
			  else if ((304<c) && (c<=334))
			  {
	       			time[1] = 11;
	       			time[2] = c - 304;
			  }
			  else if ((334<c) && (c<=365))
			  {
					time[1] = 12;
					time[2] = c - 334;
			  }
		}
		else	//非闰年
		{
			if (c <= 30)
			{
				time[1] = 1;
				time[2] = c + 1;
			}
			else if ((30<c) && (c<=58))
			{
				time[1] = 2;
				time[2] = c - 30;
			}
			else if ((58< c) && (c<=89))
			{
				time[1] = 3;
				time[2] = c - 58;
			}else if ((89<c) && (c<=119))
			{
				time[1] = 4;
				time[2] = c - 89;
			}else if ((119<c) && (c<=150))
			{
				time[1] = 5;
				time[2] = c - 119;
			}else if ((150<c) && (c<=180))
			{
				time[1] = 6;
				time[2] = c - 150;
			}else if ((180<c) && (c<=211))
			{
				time[1] = 7;
				time[2] = c - 180;
			}else if ((211<c) && (c<=242))
			{
				time[1] = 8;
				time[2] = c - 211;
			}else if ((242<c) && (c<=272))
			{
				time[1] = 9;
				time[2] = c - 242;
			}else if ((272<c) && (c<=303))
			{
				time[1] = 10;
				time[2] = c - 272;
			}else if ((303<c) && (c<=333))
			{
				time[1] = 11;
				time[2] = c - 303;
			}else if ((333<c) && (c<=364))
			{
				time[1] = 12;
				time[2] = c - 333;
			}
		}
	}
	time[3] = Second / 3600;
	time[4] = (Second % 3600) / 60;
	time[5] = Second % 60;
}

int dayofweek(int year, int month, int day)
{
	int   a = (14-month) / 12;
	int   y = year - a;
	int   m = month + 12*a - 2;
	return (day + y + y/4 - y/100 + y/400 + (31*m)/12) % 7;
}

int ConvtLgToSm( int n)
{
	return ((n>>24)&0xff) + ((n>>8)&0xff00) + ((n<<8)&0xff0000) + (n<<24);
}

void ntp_ajust_time(int ip,short port,signed int zero,st_clock_hanndle_t *p_st_handle)
{
	if ((ip==0)||(ip==-1)||(port==0))
	{
		ip = 0x650278ca;
		port = 123;
		zero = 8*3600;
	}
	printf("################## 开始NTP校时 ####################\n");
	int ntp_sock = socket(PF_INET,SOCK_DGRAM,0);
	if (ntp_sock < 0)
	{
		return;
	}
	int rb = 1;
	setsockopt(ntp_sock,SOL_SOCKET,SO_REUSEADDR,&rb,sizeof(rb));
	struct sockaddr_in uAddr;
	bzero(&uAddr,sizeof(uAddr));
	uAddr.sin_family = PF_INET;
	uAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	uAddr.sin_port = htons(NTPPORT);
	if (bind(ntp_sock,(struct sockaddr *)&uAddr,sizeof(uAddr)) < 0)
	{
		perror("[ntp_ajust_time]: Bind UDP socket error:\n");
		close(ntp_sock);
		return;
	}
	struct timeval timeout={3,0};
	setsockopt(ntp_sock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	
	char ntp_request[512];
	int ntp_request_len=0;
	st_ntp_request stMessage;
	memset(&stMessage,0,sizeof(st_ntp_request));
	stMessage.flags = 0x1b;
	time_t time2K;
	struct timeval t;
	struct tm tm_time;
	tm_time.tm_year = 2000-1900;	//2000-1-1 00:00:00
	tm_time.tm_mon  = 0;
	tm_time.tm_mday = 1;
	tm_time.tm_hour = 0;
	tm_time.tm_min  = 0;
	tm_time.tm_sec  = 0;
	time2K = mktime(&tm_time);
	gettimeofday(&t,0);
	stMessage.originate_timestamp.sec = htonl(t.tv_sec-time2K);
	memcpy(ntp_request,(char *)&stMessage,sizeof(st_ntp_request));
	ntp_request_len = sizeof(st_ntp_request);
	
	struct sockaddr_in sAddr;
	bzero(&sAddr,sizeof(sAddr));
	sAddr.sin_family = PF_INET;
	sAddr.sin_addr.s_addr = ip;
	sAddr.sin_port   = htons(port);
	sendto(ntp_sock,ntp_request,ntp_request_len,0,(struct sockaddr *)&sAddr,sizeof(sAddr));

	char recv[1600];
	int  rev_len= 0;
	socklen_t addrlen;
	int  recv_timeout_cnt=0;
	while(recv_timeout_cnt < 3)
	{
		addrlen = sizeof(uAddr);
		rev_len = recvfrom(ntp_sock,recv,1600,MSG_TRUNC,(struct sockaddr *)&uAddr,&addrlen);
		if (rev_len < 0)
		{
			recv_timeout_cnt++;
			if (recv_timeout_cnt >= 3)
				continue;
			sendto(ntp_sock,ntp_request,ntp_request_len,0,(struct sockaddr *)&sAddr,sizeof(sAddr));
		}
		else
		{
			if (rev_len == sizeof(st_ntp_request))
			{
				char stime[7];
				unsigned long rsec;
				st_ntp_request rMessage;
				memcpy((char *)&rMessage,recv,sizeof(st_ntp_request));
				rsec = htonl(rMessage.transmit_timestamp.sec);
				rsec += zero;
				SecondToDateTime((unsigned char *)stime,rsec/86400,rsec%86400);
				stime[0] -= 100;
				stime[6] = dayofweek(stime[0]+2000,stime[1],stime[2]);
				printf("[%s]: Current Time %04d-%02d-%02d %02d:%02d:%02d %02d\n",__FUNCTION__,
					stime[0]+2000,stime[1],stime[2],stime[3],stime[4],stime[5],stime[6]);
				p_st_handle->systemTime[0] = stime[0]+2000;
				p_st_handle->systemTime[1] = stime[1];
				p_st_handle->systemTime[2] = stime[2];
				p_st_handle->systemTime[3] = stime[3];
				p_st_handle->systemTime[4] = stime[4];
				p_st_handle->systemTime[5] = stime[5];
				p_st_handle->systemTime[6] = stime[6];
				setClockToSystem(p_st_handle->systemTime);
				break;
			}
		}
	}
	close(ntp_sock);
	printf("################## NTP校时完毕 ####################\n");
}

int open_ntp(st_clock_hanndle_t * p_st_handle)
{
	if (g_stp_handle == NULL)
	{
		g_stp_handle = p_st_handle;
		p_st_handle->enType = CLOCK_TYPE_NTP;
		p_st_handle->iHandle = -1;
		ntp_ajust_time(ntp_ip,ntp_port,ntp_zero,p_st_handle);
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
	return (CLOCK_TYPE_NTP|0x80);
}

int close_ntp(st_clock_hanndle_t * p_st_handle)
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

int read_ntp(st_clock_t * p_st_clock)
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

int write_ntp(st_clock_t * p_st_clock)
{
	if (g_stp_handle != NULL)
	{
		ntp_ajust_time(ntp_ip,ntp_port,ntp_zero,g_stp_handle);
		g_stp_handle->iErrorNo = CLOCK_SUCESS;
	}
	else
		return CLOCK_NONE_EXIST;
		
	return CLOCK_SUCESS;
}

int write_ntp_server(int ip,short port,signed int zero)
{
	ntp_ip = ip;
	ntp_port = port;
	ntp_zero = zero;
	return CLOCK_SUCESS;
}


