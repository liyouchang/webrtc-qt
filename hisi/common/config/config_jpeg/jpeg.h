#ifndef __MY_JPEG_CONFIG_H__
#define __MY_JPEG_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int jpegFileRead(struct DEVHARDPARAM * dev);
extern int jpegFileGet(struct JPEGPARAM *jpeg);
extern int jpegFileSet(struct JPEGPARAM *jpeg);
extern CONFIG_CALLBACK jpegCallBack;

#endif	//__MY_JPEG_CONFIG_H__
