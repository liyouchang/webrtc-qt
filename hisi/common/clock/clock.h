#ifndef __MY_CLOCK_H__
#define __MY_CLOCK_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../common_define.h"

#define CLOCK_SUCESS		0
#define CLOCK_ALREADY_USED	-1
#define CLOCK_OPEN_FAILD	-2
#define CLOCK_NONE_EXIST	-3

#define CLOCK_MODULE_VERSION	0x000001

typedef struct struct_clock_handle_s
{
	int iHandle;
	e_clock_type enType;
	pthread_t thClock;
	int iThreadCtrl;
	int iErrorNo;
	int systemTime[8];
	
}	st_clock_hanndle_t;

extern int mutex_system_clock;
extern void setClockToSystem(int * systemTime);
extern void * thread_system_clock(void * hirtc);

#endif

