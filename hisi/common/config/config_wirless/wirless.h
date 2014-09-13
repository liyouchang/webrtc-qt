#ifndef __MY_WIRELESS_CONFIG_H__
#define __MY_WIRELESS_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int wirlessFileRead(struct DEVHARDPARAM * dev);
extern int wirlessFileGet(char *wirless);
extern int wirlessFileSet(char *wirless);
extern CONFIG_CALLBACK wirlessCallBack;

#endif	//__MY_WIRELESS_CONFIG_H__
