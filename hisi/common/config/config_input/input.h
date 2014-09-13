#ifndef __MY_INPUT_CONFIG_H__
#define __MY_INPUT_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int inputFileRead(struct DEVHARDPARAM * dev);
extern int inputFileGet(struct ALMINPUT *input);
extern int inputFileSet(struct ALMINPUT *input);
extern CONFIG_CALLBACK inputCallBack;

#endif	//__MY_INPUT_CONFIG_H__
