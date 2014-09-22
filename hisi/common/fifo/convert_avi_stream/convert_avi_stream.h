#ifndef __MY_CONVERT_AVI_H__
#define __MY_CONVERT_AVI_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "../fifo.h"
#include "../../common_api.h"
#include "../../common_define.h"

#define AVIF_HASINDEX       0x00000010  // Index at end of file?
#define AVIF_ISINTERLEAVED  0x00000100
#define AVIF_TRUSTCKTYPE    0x00000800  // Use CKType to find key frames?

#define AVIIF_KEYFRAME      0x00000010L /* this frame is a key frame.*/

extern void *convert_avi_stream(void * cvt);

#endif	//__MY_CONVERT_AVI_H__
