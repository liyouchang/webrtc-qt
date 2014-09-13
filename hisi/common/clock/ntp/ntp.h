#ifndef __MY_NTP_CLOCK_H__
#define __MY_NTP_CLOCK_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "../clock.h"

struct ntp_timestamp
{
	int sec;			//整数部分
	int sec_fraction;   //小数部分
};

typedef struct ntp_message
{
	char flags;       		//版本信息
	char stratum;     		//服务器工作的级别
	char poll;        		//连续信息间的最大间隔
	char precision;   		//时钟精确度

	int root_delay;   		//参考源的总延迟
	int root_dispersion;	//主要参考源的正常差错
	int reference_identifier;//识别特殊参考源
	struct ntp_timestamp reference_timestamp;   //当前时钟参考源的种类和最近一次更新的时间
	struct ntp_timestamp originate_timestamp;   //客户机的时间
	struct ntp_timestamp receive_timestamp;     //中心时间
	struct ntp_timestamp transmit_timestamp;    //服务器向客户发时间戳的时间
}	st_ntp_request;

extern int open_ntp(st_clock_hanndle_t * p_st_handle);
extern int close_ntp(st_clock_hanndle_t * p_st_handle);
extern int read_ntp(st_clock_t * p_st_clock);
extern int write_ntp(st_clock_t * p_st_clock);
extern int write_ntp_server(int ip,short port,signed int zero);

#endif


