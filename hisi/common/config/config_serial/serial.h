#ifndef __MY_SERIAL_CONFIG_H__
#define __MY_SERIAL_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int serialFileRead(struct DEVHARDPARAM * dev);
extern int serialFileGet(struct SERIAL *serial);
extern int serialFileSet(struct SERIAL *serial);
extern CONFIG_CALLBACK serialCallBack;

#endif	//__MY_SERIAL_CONFIG_H__
