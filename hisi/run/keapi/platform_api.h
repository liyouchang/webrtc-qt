#ifndef __PLATFORM_API_H__
#define __PLATFORM_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "platform_define.h"

#ifdef __cplusplus
extern "C" {
#endif

int PLATFORM_Tools_Open(void);
int PLATFORM_Tools_Close(void);

int PLATFORM_Update_Open(void);
int PLATFORM_Update_Close(void);

#ifdef __cplusplus
}
#endif

#endif	//__PLATFORM_API_H__
