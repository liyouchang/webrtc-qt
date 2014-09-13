#ifndef _CPU_MEMORY_H_
#define _CPU_MEMORY_H_


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

struct MemInfo
{
	char memname[20];
    unsigned int total;
    unsigned int free;
    unsigned int buffer;
    unsigned int cached;
    unsigned int Dirty;
};

extern int cpuUsageRate;
extern int memUsageRate;
extern struct MemInfo mem;

extern int cal_mem_occupy(void);
extern int cal_cpu_occupy (void);

#endif
