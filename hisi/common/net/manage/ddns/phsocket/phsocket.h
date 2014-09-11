#ifndef __PHSOCKET__H__
#define __PHSOCKET__H__

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "../ddns.h"
#endif
#ifndef BOOL
#define BOOL unsigned int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

extern int phReadOneLine(int m_hSocket, char * lpszBuf, int nBufLen);
extern BOOL	phAccept(int m_hSocket, int *rSocket, struct sockaddr_in* lpSockAddr, int* lpSockAddrLen);
extern BOOL	phBind(int m_hSocket, unsigned short nSocketPort, char *lpszSocketAddress);
extern void	phClose(int *m_hSocket);
extern BOOL	phConnect(int m_hSocket, char *lpszHostAddress, unsigned short nHostPort, int *nAddressIndex, char *szSelectedAddress);
extern BOOL	phListen(int m_hSocket, int nConnectionBacklog);
extern int		phReceive(int m_hSocket, void* lpBuf, int nBufLen, int nFlag);
extern int		phSend(int m_hSocket, void* lpBuf, int nBufLen, int nFlag);
extern BOOL	phCreate(int *m_hSocket, unsigned short nSocketPort, int nSocketType, char *lpszSocketAddress);
extern int		phDataReadable(int m_hSocket, int nTimeout); //in Seconds

#endif

