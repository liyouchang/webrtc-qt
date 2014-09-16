#ifndef __UNICODE_H__
#define __UNICODE_H__

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	short start;
	short end;
}   key_index_t;

extern void unicode2asc(const unsigned short *uni,unsigned char *str);
extern short asc2unicode(unsigned char *dest,short dest_size,unsigned char *src);
extern int g2u(unsigned char *resultbuf, const unsigned char *inbuf);
extern int u2g(unsigned char *resultbuf, const unsigned char *inbuf);

#ifdef __cplusplus
}
#endif

#endif

