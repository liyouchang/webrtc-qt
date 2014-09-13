#ifndef __MY_DDNS_H__
#define __MY_DDNS_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>
#include <net/if.h>
#include <pthread.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../../../common_api.h"
#include "../../../common_define.h"

#include "phglobal/phglobal.h"
#include "phupdate/phupdate.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

typedef struct dns_server
{
	int iSocket;
	int serverIP;
	short usPort;
}	st_dns_server_t;

extern int isWanLinked;
extern void ddns_create(void);
extern void ddns_destroy(void);
extern int gethostbyname_my(char * host,char * ip);

#endif
