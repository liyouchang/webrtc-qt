#ifndef __MY_STORE_CONFIG_H__
#define __MY_STORE_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int storeFileRead(struct DEVHARDPARAM * dev);
extern int storeFileGet(struct STORPARAM *store);
extern int storeFileSet(struct STORPARAM *store);
extern CONFIG_CALLBACK storeCallBack;

#endif	//__MY_STORE_CONFIG_H__
