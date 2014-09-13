#ifndef __MY_VO_CONFIG_H__
#define __MY_VO_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int voFileRead(struct DEVHARDPARAM * dev);
extern int voFileGet(struct VOPARAM *vo);
extern int voFileSet(struct VOPARAM *vo);
extern CONFIG_CALLBACK voCallBack;

#endif	//__MY_VO_CONFIG_H__
