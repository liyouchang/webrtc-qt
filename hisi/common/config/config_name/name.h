#ifndef __MY_NAME_CONFIG_H__
#define __MY_NAME_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int nameFileRead(struct DEVHARDPARAM * dev);
extern int nameFileGet(struct NAMEPARAM *name);
extern int nameFileSet(struct NAMEPARAM *name);
extern CONFIG_CALLBACK nameCallBack;

#endif	//__MY_NAME_CONFIG_H__
