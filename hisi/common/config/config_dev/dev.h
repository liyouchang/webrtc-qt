#ifndef __MY_DEV_CONFIG_H__
#define __MY_DEV_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int devFileRead(struct DEVHARDPARAM **dev);
extern int devFileGet(struct DEVHARDPARAM *dev);

#endif
