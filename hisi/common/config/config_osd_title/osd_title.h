#ifndef __MY_OSD_TITLE_CONFIG_H__
#define __MY_OSD_TITLE_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int osdTitleFileRead(struct DEVHARDPARAM * dev);
extern int osdTitleFileGet(struct TITLEOSD *osdTitle);
extern int osdTitleFileSet(struct TITLEOSD *osdTitle);
extern CONFIG_CALLBACK osdTitleCallBack;

#endif	//__MY_OSD_TITLE_CONFIG_H__
