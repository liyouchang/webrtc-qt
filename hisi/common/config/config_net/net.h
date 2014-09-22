#ifndef __MY_NET_CONFIG_H__
#define __MY_NET_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int netFileRead(struct DEVHARDPARAM *dev);
extern int netFileGet(struct NETPARAM *net);
extern int netFileSet(struct NETPARAM *net);
extern CONFIG_CALLBACK netCallBack;

#endif	//__MY_NET_CONFIG_H__
