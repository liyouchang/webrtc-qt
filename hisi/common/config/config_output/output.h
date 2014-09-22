#ifndef __MY_OUTPUT_CONFIG_H__
#define __MY_OUTPUT_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int outputFileRead(struct DEVHARDPARAM * dev);
extern int outputFileGet(struct ALMOUTPUT *output);
extern int outputFileSet(struct ALMOUTPUT *output);
extern CONFIG_CALLBACK outputCallBack;

#endif	//__MY_OUTPUT_CONFIG_H__
