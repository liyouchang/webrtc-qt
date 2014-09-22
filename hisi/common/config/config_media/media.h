#ifndef __MY_MEDIA_CONFIG_H__
#define __MY_MEDIA_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int mediaFileRead(struct DEVHARDPARAM * dev);
extern int mediaFileGet(struct MEDIAPARAM *media);
extern int mediaFileSet(struct MEDIAPARAM *media);
extern CONFIG_CALLBACK mediaCallBack;

#endif	//__MY_MEDIA_CONFIG_H__
