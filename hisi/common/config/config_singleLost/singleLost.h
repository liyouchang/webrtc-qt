#ifndef __MY_SINGLELOST_CONFIG_H__
#define __MY_SINGLELOST_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int singleLostFileRead(struct DEVHARDPARAM * dev);
extern int singleLostFileGet(struct SINGLELOST *singleLost);
extern int singleLostFileSet(struct SINGLELOST *singleLost);
extern CONFIG_CALLBACK singleLostCallBack;

#endif	//__MY_SINGLELOST_CONFIG_H__
