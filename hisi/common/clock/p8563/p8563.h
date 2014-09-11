#ifndef __MY_PCF8563_CLOCK_H__
#define __MY_PCF8563_CLOCK_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../clock.h"
#include "pcf_8563/pcf8563.h"

extern int open_pcf8563(st_clock_hanndle_t * p_st_handle);
extern int close_pcf8563(st_clock_hanndle_t * p_st_handle);
extern int read_pcf8563(st_clock_t * p_st_clock);
extern int write_pcf8563(st_clock_t * p_st_clock);

#endif


