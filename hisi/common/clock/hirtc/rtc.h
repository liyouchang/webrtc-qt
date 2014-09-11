#ifndef __MY_RTC_CLOCK_H__
#define __MY_RTC_CLOCK_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../clock.h"
#include "rtc/hi_rtc.h"

extern int open_hirtc(st_clock_hanndle_t * p_st_handle);
extern int close_hirtc(st_clock_hanndle_t * p_st_handle);
extern int read_hirtc(st_clock_t * p_st_clock);
extern int write_hirtc(st_clock_t * p_st_clock);

#endif

