#ifndef __MY_CONFIG_OVERLAY_H__
#define __MY_CONFIG_OVERLAY_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int overlayFileRead(struct DEVHARDPARAM * dev);
extern int overlayFileGet(struct OVALM *overlay);
extern int overlayFileSet(struct OVALM *overlay);
extern CONFIG_CALLBACK overlayCallBack[2];

#endif	//__MY_CONFIG_OVERLAY_H__
