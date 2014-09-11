#ifndef __MY_UART_MODULE_H__
#define __MY_UART_MODULE_H__

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "../common_define.h"

struct UART_MODULE
{
	int iFd;
	st_uart_attr_t attr;
	st_485_ctrl_t ctrl;
	UART_RECEIVE_CALLBACK receive;
	UART_PROTOCOL_CALLBACK protocol;
	
	int iRcvThread;
	pthread_t thread_recv;
}	;

#endif	//__MY_UART_MODULE_H__

