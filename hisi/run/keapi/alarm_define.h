#ifndef __ALARM_DEFINE_H__
#define __ALARM_DEFINE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common_define.h"

typedef int (*ALARM_CALLBACK)(st_alarm_upload_t *,char * pJpegData,int iJpegLen);

#endif	//__ALARM_DEFINE_H__
