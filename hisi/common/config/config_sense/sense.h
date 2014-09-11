#ifndef __MY_SENSE_CONFIG_H__
#define __MY_SENSE_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int senseFileRead(struct DEVHARDPARAM * dev);
extern int senseFileGet(struct SENSE *sense);
extern int senseFileSet(struct SENSE *sense);
extern CONFIG_CALLBACK senseCallBack[2];

#endif	//__MY_SENSE_CONFIG_H__
