#ifndef __MY_OSD_TIME_CONFIG_H__
#define __MY_OSD_TIME_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int osdTimeFileRead(struct DEVHARDPARAM * dev);
extern int osdTimeFileGet(struct TIMEOSD *osdTime);
extern int osdTimeFileSet(struct TIMEOSD *osdTime);
extern CONFIG_CALLBACK osdTimeCallBack;

#endif	//__MY_OSD_TIME_CONFIG_H__
