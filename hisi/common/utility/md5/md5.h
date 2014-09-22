#ifndef __KAER_MD5_H__
#define __KAER_MD5_H__
/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
//typedef unsigned long int UINT4;
typedef unsigned int UINT4;
/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

extern unsigned char digest[16]; 
/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} KAER_MD5_CTX;
//void kaer_MD5Init PROTO_LIST ((KAER_MD5_CTX *));
//void kaer_MD5Update PROTO_LIST ((KAER_MD5_CTX *, unsigned char *, unsigned int));
//void kaer_MD5Final PROTO_LIST ((unsigned char [16], KAER_MD5_CTX *));
extern void MDString (unsigned char *Src, unsigned char *dst, int len);
#endif
