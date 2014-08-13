#define USE_IPV6
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned int   UINT;
typedef unsigned int   size_t;
typedef long           LONG;
typedef unsigned long  ULONG;
typedef unsigned long  DWORD;
typedef unsigned char  BYTE;
typedef unsigned char* PBYTE;
typedef unsigned char* LPBYTE;
typedef int            BOOL;
typedef char           TCHAR;
typedef char*          LPSTR;
typedef const char*    LPCSTR;
typedef char*          LPTSTR;
typedef const char*    LPCTSTR;
typedef long           SOCKET;
typedef sockaddr_storage SOCKADDR_STORAGE;
typedef sockaddr_storage* LPSOCKADDR_STORAGE;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr *LPSOCKADDR;
typedef struct in6_addr IN6_ADDR;
typedef struct in6_addr *PIN6_ADDR;
typedef struct in6_addr *LPIN6_ADDR;
typedef struct hostent HOSTENT;
typedef struct hostent  *LPHOSTENT;
typedef struct servent  *LPSERVENT;
typedef struct timeval TIMEVAL;
typedef struct addrinfo ADDRINFO;


#ifndef AF_INET6 // not all platform have this
#define AF_INET6        23              /* IP version 6 */
#define PF_INET6        AF_INET6
#endif

#define S_OK                      0
#define S_FALSE                   1
#define SUCCEEDED(hr)             ((hr) >= S_OK)
#define TRUE                      1
#define FALSE                     0
#define SD_BOTH                   2
#define INVALID_SOCKET            0xffffffff
#define SOCKET_ERROR              -1
#define ERROR_SUCCESS             0
#define ERROR_ACCESS_DENIED       5
#define ERROR_INVALID_HANDLE      6
#define ERROR_BAD_ARGUMENTS       160
#define ERROR_INSUFFICIENT_BUFFER 1000
#define ERROR_INVALID_PARAMETER   1001
#define WSAEBADF                  10009
#define WSAENOTSOCK               10038
#define WSAENOPROTOOPT            10042
#define WSAENETDOWN               10050
#define WSAENETUNREACH            10051
#define WSAENETRESET              10052
#define WSAECONNABORTED           10053
#define WSAECONNRESET             10054
#define WSA_INVALID_PARAMETER     10073
#define MAX_PATH                  260
#define TEXT(quote)               quote
#define _ASSERTE                  assert

static const DWORD INFINITE = (DWORD)0xffffffff;
static int _lastError = 0;

inline void SetLastError(int ec)
{
    _lastError = ec;
}

inline int GetLastError()
{
    return _lastError;
}

inline int WSASetLastError(int ec)
{
    _lastError = ec;
}

inline int WSAGetLastError()
{
    return _lastError;
}

inline long StringCbCopyA(char* pDest, int size, const char* pSrc)
{
    strcpy(pDest, pSrc);
    pDest[size-1] = 0;
    return 0;
}

inline long StringCbLengthA(const char* pSrc, int, size_t* cb)
{
    *cb = strlen(pSrc);
    return 0;
}

inline long StringCbPrintfA(char* pBuf, int size, const char* pFmt, ...)
{
    va_list args;
    va_start(args, pFmt);
    vprintf(pFmt, args);
    va_end(args);
    pBuf[size-1] = 0;
    return 0;
}

inline long InterlockedExchange(long* p, long v)
{
    long pr = (*p);
    *p = v;
    return pr;
}

inline int closesocket(SOCKET s)
{
    return close(static_cast<int>(s) );
}

#define StringCchPrintf    StringCbPrintfA
#define StringCchLength    StringCbLengthA
#define _istdigit          isdigit
#define _ttoi              atoi
