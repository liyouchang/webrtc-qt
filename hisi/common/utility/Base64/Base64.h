/*******************************************************************************/
/*					 			Base64.h									   */
/* 					 Description : BASE64 encoding and decoding				   */
/*******************************************************************************/
#ifndef __BASE_H__
#define __BASE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern char *base64_encode_kaer(const unsigned char *data, int len);
extern int base64_decode_kaer(unsigned char *buf, const char *bdata);

/////////////////////////////////////////////////////////////////////////////
#endif
