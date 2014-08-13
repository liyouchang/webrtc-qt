/*
** Copyright 2003-2009, Ernest Laurentin (http://www.ernzo.com/)
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
**
** File:        SocketHandle.cpp
** Version:     1.4 - IPv6 support
**              1.3 - Update for Asynchronous mode / Linux port
**              1.2 - Update interface for TCP remote connection
**              1.1 - Added multicast support
*/
#include "stdafx.h"
#ifdef WIN32
#include <stdlib.h>
#ifndef UNDER_CE
#include <crtdbg.h>
#endif
#include <strsafe.h>
#endif
#include "SocketHandle.h"


#ifndef BUFFER_SIZE
#define BUFFER_SIZE     64*1024
#endif
#ifndef SOCKHANDLE_TTL
#define SOCKHANDLE_TTL 5
#endif
#ifndef SOCKHANDLE_HOPS
#define SOCKHANDLE_HOPS 10
#endif
#define HOSTNAME_SIZE   MAX_PATH
#define STRING_LENGTH   40



#ifdef WIN32
#ifndef UNDER_CE
#pragma comment(lib, "ws2_32.lib")
#else
#pragma comment(lib, "Ws2.lib")
#endif
#endif


///////////////////////////////////////////////////////////////////////////////
// SockAddrIn Struct
SockAddrIn SockAddrIn::NULLAddr;

///////////////////////////////////////////////////////////////////////////////
// Constructs

SockAddrIn::SockAddrIn()
{
    Clear();
}

SockAddrIn::SockAddrIn(const SockAddrIn& sin)
{
    Copy( sin );
}

SockAddrIn::~SockAddrIn()
{
}

///////////////////////////////////////////////////////////////////////////////
// Clear
void SockAddrIn::Clear()
{
    memset(this, 0, sizeof(sockaddr_storage));
}

///////////////////////////////////////////////////////////////////////////////
// Copy
SockAddrIn& SockAddrIn::Copy(const SockAddrIn& sin)
{
    ss_family = sin.ss_family;
    memcpy(this, &sin, Size());
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// IsEqual
bool SockAddrIn::IsEqual(const SockAddrIn& sin) const
{
    // Is it Equal? - ignore 'sin_zero'
    if ( ss_family == AF_INET ) {
        return (memcmp(this, &sin, Size()-8) == 0);
    }
    return (memcmp(this, &sin, Size()) == 0);
}

///////////////////////////////////////////////////////////////////////////////
// CreateFrom
bool SockAddrIn::CreateFrom(LPCTSTR pszAddr, LPCTSTR pszService, int nFamily /*=AF_INET*/)
{
    Clear();
    CSocketHandle::GetAddressInfo(pszAddr, pszService, nFamily, *this);
    return !IsNull();
}

///////////////////////////////////////////////////////////////////////////////
// CreateFrom
bool SockAddrIn::CreateFrom(ULONG lIPAddr, USHORT nPort, int nFamily /*= AF_INET*/, bool bFmtHost /*= true*/)
{
    Clear();
    _ASSERTE( nFamily == AF_INET ); // supports IPv4 only
    SOCKADDR_IN* psin = reinterpret_cast<SOCKADDR_IN*>(this);
    psin->sin_family = static_cast<short>(nFamily);
    if ( bFmtHost )
    {
        psin->sin_addr.s_addr = htonl( lIPAddr );
        psin->sin_port = htons( nPort );
    }
    else
    {
        psin->sin_addr.s_addr = lIPAddr;
        psin->sin_port = nPort;
    }
    return !IsNull();
}


///////////////////////////////////////////////////////////////////////////////
// CSocketHandle
CSocketHandle::CSocketHandle()
: m_hSocket(INVALID_SOCKET)
{
}

CSocketHandle::~CSocketHandle()
{
    Close();
}

///////////////////////////////////////////////////////////////////////////////
// IsOpen
bool CSocketHandle::IsOpen() const 
{
    return ( INVALID_SOCKET != m_hSocket );
}

///////////////////////////////////////////////////////////////////////////////
// GetSocket
SOCKET CSocketHandle::GetSocket() const
{
    return m_hSocket;
}

///////////////////////////////////////////////////////////////////////////////
// GetSocketType
int CSocketHandle::GetSocketType() const
{
    int type = -1;
    if ( INVALID_SOCKET != m_hSocket ) {
        socklen_t optlen = sizeof(int);
        if ( getsockopt(GetSocket(), SOL_SOCKET, SO_TYPE, reinterpret_cast<char*>(&type),
                        &optlen) == SOCKET_ERROR)
        {
            SetLastError( WSAGetLastError() );
        }
    }
    return type;
}

///////////////////////////////////////////////////////////////////////////////
// Attach
bool CSocketHandle::Attach(SOCKET sock)
{
    if ( INVALID_SOCKET == m_hSocket )
    {
        m_hSocket = sock;
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// Detach
SOCKET CSocketHandle::Detach()
{
    SOCKET sock = m_hSocket;
    ::InterlockedExchange(reinterpret_cast<long*>(&m_hSocket), INVALID_SOCKET);
    return sock;
}

///////////////////////////////////////////////////////////////////////////////
// GetSockName
bool CSocketHandle::GetSockName(SockAddrIn& saddr_in) const
{
    _ASSERTE( IsOpen() );
    if (IsOpen()) {
        socklen_t namelen = (socklen_t)saddr_in.Size();
        if (SOCKET_ERROR != getsockname(GetSocket(), saddr_in, &namelen))
        {
            return true;
        }
        SetLastError( WSAGetLastError() );
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// GetPeerName
bool CSocketHandle::GetPeerName(SockAddrIn& saddr_in) const
{
    _ASSERTE( IsOpen() );
    if (IsOpen()) {
        socklen_t namelen = (socklen_t)saddr_in.Size();
        if (SOCKET_ERROR != getpeername(GetSocket(), saddr_in, &namelen))
        {
            return true;
        }
        SetLastError( WSAGetLastError() );
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// Close
void CSocketHandle::Close()
{
    if ( IsOpen() )
    {
        ShutdownConnection(static_cast<SOCKET>(
            ::InterlockedExchange((LONG*)&m_hSocket, INVALID_SOCKET)
            ));
    }
}

///////////////////////////////////////////////////////////////////////////////
// AddMembership
bool CSocketHandle::AddMembership(LPCTSTR pszIPAddr, LPCTSTR pszNIC)
{
    _ASSERTE( IsOpen() );
    if ( IsOpen() )
    {
        int nType = 0;
        socklen_t nOptLen = sizeof(int);
        if ( SOCKET_ERROR != getsockopt(m_hSocket, SOL_SOCKET, SO_TYPE, (char*)&nType, &nOptLen))
        {
            if ( nType == SOCK_DGRAM )
            {
                // Setup interface for multicast traffic
                SockAddrIn mcastAddr;
                if (GetAddressInfo(pszIPAddr, NULL, AF_UNSPEC, mcastAddr))
                {
                    SockAddrIn interfAddr;
                    GetAddressInfo(pszNIC, NULL, mcastAddr.ss_family, interfAddr);
                    if ( mcastAddr.ss_family == AF_INET )
                    {
                        int nTTL = SOCKHANDLE_TTL;
                        if ( SOCKET_ERROR != setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&nTTL, sizeof(nTTL)))
                        {
                            ULONG ulNIC = interfAddr.GetIPAddr();
                            if ( SOCKET_ERROR != setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_IF,(char *) &ulNIC, sizeof(ulNIC)))
                            {
                                ip_mreq mreq = { 0 };
                                mreq.imr_multiaddr.s_addr = mcastAddr.GetIPAddr();
                                mreq.imr_interface.s_addr = ulNIC;
                                if ( SOCKET_ERROR != setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    else if ( mcastAddr.ss_family == AF_INET6 )
                    {
                        int nTTL = SOCKHANDLE_HOPS;
                        if ( SOCKET_ERROR != setsockopt(m_hSocket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const char*)&nTTL, sizeof(nTTL)))
                        {
                            ipv6_mreq mreq6 = { 0 };
                            IN6_ADDR mcin6 = ((sockaddr_in6*)&mcastAddr)->sin6_addr;
                            memcpy(&(mreq6.ipv6mr_multiaddr), &mcin6, sizeof(IN6_ADDR));
                            if ( SOCKET_ERROR != setsockopt(m_hSocket, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (const char*)&mreq6, sizeof(mreq6)))
                            {
                                return true;
                            }
                        }
                    }
                    else
                    {
                        // invalid socket option
                        WSASetLastError(WSAENOPROTOOPT);
                    }
                }
            }
            else
            {
                // invalid socket option
                WSASetLastError(WSAENOPROTOOPT);
            }
        }
        SetLastError( WSAGetLastError() );
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// DropMembership
bool CSocketHandle::DropMembership(LPCTSTR pszIPAddr, LPCTSTR pszNIC)
{
    _ASSERTE( IsOpen() );
    if ( IsOpen() )
    {
        int nType = 0;
        socklen_t nOptLen = sizeof(int);
        if ( SOCKET_ERROR != getsockopt(m_hSocket, SOL_SOCKET, SO_TYPE, (char*)&nType, &nOptLen))
        {
            if ( nType == SOCK_DGRAM )
            {
                SockAddrIn mcastAddr;
                if (GetAddressInfo(pszIPAddr, NULL, AF_UNSPEC, mcastAddr))
                {
                    SockAddrIn interfAddr;
                    GetAddressInfo(pszNIC, NULL, mcastAddr.ss_family, interfAddr);
                    if ( mcastAddr.ss_family == AF_INET )
                    {
                        ip_mreq mreq;
                        mreq.imr_multiaddr.s_addr = mcastAddr.GetIPAddr();
                        mreq.imr_interface.s_addr = interfAddr.GetIPAddr();;
                        if ( SOCKET_ERROR != setsockopt(m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)))
                        {
                            return true;
                        }
                    }
                    else if ( mcastAddr.ss_family == AF_INET6 )
                    {
                        ipv6_mreq mreq6 = { 0 };
                        IN6_ADDR mcin6 = ((sockaddr_in6*)&mcastAddr)->sin6_addr;
                        memcpy(&(mreq6.ipv6mr_multiaddr), &mcin6, sizeof(IN6_ADDR));
                        if ( SOCKET_ERROR != setsockopt(m_hSocket, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (const char*)&mreq6, sizeof(mreq6)))
                        {
                            return true;
                        }
                    }
                    else
                    {
                        // invalid socket option
                        WSASetLastError(WSAENOPROTOOPT);
                    }
                }
            }
            else
            {
                // invalid socket option
                WSASetLastError(WSAENOPROTOOPT);
            }
        }
        SetLastError( WSAGetLastError() );
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// CreateSocket
bool CSocketHandle::CreateSocket(LPCTSTR pszHostName, LPCTSTR pszServiceName,
                                int nFamily, int nType, UINT uOptions /* = 0 */)
{
    // Socket is already opened
    if ( IsOpen() ) {
        SetLastError(ERROR_ACCESS_DENIED);
        return false;
    }

    // Create a Socket that is bound to a specific service provider
    // nFamily: (AF_INET, AF_INET6)
    // nType: (SOCK_STREAM, SOCK_DGRAM)
#ifdef SOCKHANDLE_USE_OVERLAPPED
    SOCKET sock = WSASocket(nFamily, nType, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
    SOCKET sock = socket(nFamily, nType, IPPROTO_IP);
#endif
    if (INVALID_SOCKET != sock)
    {
        if (uOptions & SO_REUSEADDR)
        {
            // Inform Windows Sockets provider that a bind on a socket should not be disallowed
            // because the desired address is already in use by another socket
            BOOL optval = TRUE;
            if ( SOCKET_ERROR == setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof( BOOL ) ) )
            {
                SetLastError( WSAGetLastError() );
                closesocket( sock );
                return false;
            }
        }

        if (nType == SOCK_DGRAM)
        {
            if ((uOptions & SO_BROADCAST) && (nFamily == AF_INET))
            {
                // Inform Windows Sockets provider that broadcast messages are allowed
                BOOL optval = TRUE;
                if ( SOCKET_ERROR == setsockopt( sock, SOL_SOCKET, SO_BROADCAST, (char *) &optval, sizeof( BOOL ) ) )
                {
                    SetLastError( WSAGetLastError() );
                    closesocket( sock );
                    return false;
                }
            }
#ifdef SOCKHANDLE_CONFIGBUF
            // configure buffer size
            socklen_t rcvbuf = BUFFER_SIZE;
            if ( SOCKET_ERROR == setsockopt( sock, SOL_SOCKET, SO_RCVBUF, (char *) &rcvbuf, sizeof( int ) ) )
            {
                SetLastError( WSAGetLastError() );
                closesocket( sock );
                return false;
            }
#endif
        }
        // Associate a local address with the socket
        SockAddrIn sockAddr;
		//use INADDR_ANY replace  by lht
		if (pszHostName == NULL)
		{
			sockAddr.CreateFrom(INADDR_ANY,0,nFamily);
		}
		else
		{
			sockAddr.CreateFrom(pszHostName, pszServiceName, nFamily);
		}

        //sockAddr.CreateFrom(pszHostName, pszServiceName, nFamily);

        if ( SOCKET_ERROR == bind(sock, sockAddr, (int)sockAddr.Size()))
        {
            SetLastError( WSAGetLastError() );
            closesocket( sock );
            return false;
        }

        // Listen to the socket, only valid for connection socket (TCP)
        if (SOCK_STREAM == nType)
        {
            if ( SOCKET_ERROR == listen(sock, SOMAXCONN))
            {
                SetLastError( WSAGetLastError() );
                closesocket( sock );
                return false;
            }
        }

        // Success, now we may save this socket
        m_hSocket = sock;
    }

    return (INVALID_SOCKET != sock);
}


bool ConnectSelect(SOCKET sockfd, SockAddrIn sockAddr,int timeout)
{
	

	unsigned long ul = 1;
	ioctlsocket(sockfd, FIONBIO, &ul); //设置为非阻塞模式 
	bool ret = false;
	int error = -1;
	int len = sizeof(int);
	fd_set set;

	if( SOCKET_ERROR == connect( sockfd, sockAddr, (int)sockAddr.Size()))
	{
		timeval tm;
		tm.tv_sec  = timeout;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
		if( select(sockfd+1, NULL, &set, NULL, &tm) > 0)
		{
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&error, /*(socklen_t *)*/&len);
			if(error == 0) 
				ret = true;
			else 
				ret = false;
		} 
		else 
			ret = false;
	}
	else 
	{
		ret = true;
	}
	ul = 0;
	ioctlsocket(sockfd, FIONBIO, &ul); //设置为阻塞模式

	return ret;


}

///////////////////////////////////////////////////////////////////////////////
// ConnectTo
bool CSocketHandle::ConnectTo(LPCTSTR pszHostName, LPCTSTR pszRemote,
                              LPCTSTR pszServiceName, int nFamily, int nType)
{
    // Socket is already opened
    if ( IsOpen() ) {
        SetLastError(ERROR_ACCESS_DENIED);
        return false;
    }

    // Create a Socket that is bound to a specific service provider
    // nFamily: (AF_INET, AF_INET6)
    // nType: (SOCK_STREAM, SOCK_DGRAM)
#ifdef SOCKHANDLE_USE_OVERLAPPED
    SOCKET sock = WSASocket(nFamily, nType, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
    SOCKET sock = socket(nFamily, nType, IPPROTO_IP);
#endif
    if (INVALID_SOCKET != sock)
    {
        // Associate a local address with the socket but let provider assign a port number
        SockAddrIn sockAddr;
//         if (false == sockAddr.CreateFrom(pszHostName, TEXT("0"), nFamily))
//         {
//             SetLastError( WSAGetLastError() );
//             closesocket( sock );
//             return false;
//         }
		//use INADDR_ANY replace  by lht
		if (pszHostName == NULL)
		{
			sockAddr.CreateFrom(INADDR_ANY,0,nFamily);
		}
		else
		{
			if (false == sockAddr.CreateFrom(pszHostName, TEXT("0"), nFamily))
			{
			             SetLastError( WSAGetLastError() );
			             closesocket( sock );
			             return false;
			}
		}

        if ( SOCKET_ERROR == bind(sock, sockAddr, (int)sockAddr.Size()))
        {
            SetLastError( WSAGetLastError() );
            closesocket( sock );
            return false;
        }

#ifdef SOCKHANDLE_CONFIGBUF
        if (nType == SOCK_DGRAM)
        {
            // configure buffer size
            socklen_t rcvbuf = BUFFER_SIZE;
            if ( SOCKET_ERROR == setsockopt( sock, SOL_SOCKET, SO_RCVBUF, (char *) &rcvbuf, sizeof( int ) ) )
            {
                SetLastError( WSAGetLastError() );
                closesocket( sock );
                return false;
            }
        }
#endif
        // Now get destination address & port
        sockAddr.CreateFrom( pszRemote, pszServiceName, nFamily );

        // try to connect - if fail, server not ready
//         if (SOCKET_ERROR == connect( sock, sockAddr, (int)sockAddr.Size()))
//         {
//             SetLastError( WSAGetLastError() );
//             closesocket( sock );
//             return false;
//         }

	//socket select by lht
		if (!ConnectSelect(sock,sockAddr,1))
		{
			SetLastError( WSAGetLastError() );
			closesocket( sock );
			return false;
		}
		
	


        // Success, now we may save this socket
        m_hSocket = sock;
    }
    return (INVALID_SOCKET != sock);
}

///////////////////////////////////////////////////////////////////////////////
// Read
DWORD CSocketHandle::Read(LPBYTE lpBuffer, DWORD dwSize, LPSOCKADDR lpAddrIn,
                         DWORD dwTimeout)
{
    _ASSERTE( IsOpen() );
    _ASSERTE( lpBuffer != NULL );

    if (!IsOpen() || lpBuffer == NULL || dwSize < 1L)
        return (DWORD)-1L;

    fd_set  fdRead  = { 0 };
    TIMEVAL stTime;
    TIMEVAL *pstTime = NULL;

    if ( INFINITE != dwTimeout ) {
        stTime.tv_sec = dwTimeout/1000;
        stTime.tv_usec = (dwTimeout%1000)*1000;
        pstTime = &stTime;
    }

    SOCKET s = GetSocket();

    // Set Descriptor
    FD_SET( s, &fdRead );

    // Select function set read timeout
    DWORD dwBytesRead = 0L;
    int res = 1;
    if ( pstTime != NULL )
        res = select((int)s, &fdRead, NULL, NULL, pstTime );
    if ( res > 0)
    {
        if (lpAddrIn)
        {
            // UDP
            socklen_t fromlen = sizeof(SOCKADDR_STORAGE);
            res = recvfrom(s, reinterpret_cast<LPSTR>(lpBuffer), dwSize, 0, lpAddrIn, &fromlen);
        }
        else
        {
            // TCP
            res = recv(s, reinterpret_cast<LPSTR>(lpBuffer), dwSize, 0);
        }
        if ( res == 0 ) {
            WSASetLastError(WSAECONNRESET);
            res = SOCKET_ERROR;
        }
    }
    if ( res == SOCKET_ERROR )
    {
        //SetLastError( WSAGetLastError() );
    }
    dwBytesRead = (DWORD)((res >= 0)?(res) : (-1));

    return dwBytesRead;
}

#ifdef WIN32
///////////////////////////////////////////////////////////////////////////////
// ReadEx
DWORD CSocketHandle::ReadEx(LPBYTE lpBuffer, DWORD dwSize, LPSOCKADDR lpAddrIn,
                         LPWSAOVERLAPPED lpOverlapped, LPWSACOMPLETIONROUTINE lpCompletionRoutine)
{
    _ASSERTE( IsOpen() );
    _ASSERTE( lpBuffer != NULL );

    if (!IsOpen() || lpBuffer == NULL || dwSize < 1L)
        return (DWORD)-1L;

    SOCKET s = GetSocket();
    // Send message to peer
    WSABUF wsabuf;
    wsabuf.buf = (char FAR*)lpBuffer;
    wsabuf.len = dwSize;

    // Select function set read timeout
    DWORD dwBytesRead = 0L;
    DWORD dwFlags = 0L;
    int res = 0;
    if (lpAddrIn)
    {
        // UDP
        socklen_t fromlen = sizeof(SOCKADDR_STORAGE);
        res = WSARecvFrom( s, &wsabuf, 1, &dwBytesRead, &dwFlags, lpAddrIn, &fromlen, lpOverlapped, lpCompletionRoutine);
    }
    else
    {
        // TCP
        res = WSARecv( s, &wsabuf, 1, &dwBytesRead, &dwFlags, lpOverlapped, lpCompletionRoutine);
    }
    if ( res == SOCKET_ERROR )
    {
        res = WSAGetLastError();
        if ( res != WSA_IO_PENDING )
        {
            dwBytesRead = (DWORD)-1L;
            SetLastError( res );
        }
    }

    return dwBytesRead;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Write
DWORD CSocketHandle::Write(const LPBYTE lpBuffer, DWORD dwCount,
                          const LPSOCKADDR lpAddrIn, DWORD dwTimeout)
{
    _ASSERTE( IsOpen() );
    _ASSERTE( NULL != lpBuffer );

    // validate params
    if (!IsOpen() || NULL == lpBuffer)
        return (DWORD)-1L;

    fd_set  fdWrite  = { 0 };
    TIMEVAL stTime;
    TIMEVAL *pstTime = NULL;

    if ( INFINITE != dwTimeout ) {
        stTime.tv_sec = dwTimeout/1000;
        stTime.tv_usec = (dwTimeout%1000)*1000;
        pstTime = &stTime;
    }

    SOCKET s = GetSocket();

    // Set Descriptor
    FD_SET( s, &fdWrite );

    // Select function set write timeout
    DWORD dwBytesWritten = 0L;
    int res = 1;
    if ( pstTime != NULL )
        res = select((int)s, NULL, &fdWrite, NULL, pstTime );
    if ( res > 0)
    {
        // Send message to peer
        if (lpAddrIn)
        {
            // UDP
            res = sendto( s, reinterpret_cast<LPCSTR>(lpBuffer), dwCount, 0, lpAddrIn, sizeof(SOCKADDR_STORAGE));
        }
        else
        {
            // TCP
            res = send( s, reinterpret_cast<LPCSTR>(lpBuffer), dwCount, 0);
        }
    }
    if ( res == SOCKET_ERROR )
    {
        SetLastError( WSAGetLastError() );
    }
    dwBytesWritten = (DWORD)((res >= 0)?(res) : (-1));

    return dwBytesWritten;
}

#ifdef WIN32
///////////////////////////////////////////////////////////////////////////////
// WriteEx
DWORD CSocketHandle::WriteEx(const LPBYTE lpBuffer, DWORD dwCount,
                            const LPSOCKADDR lpAddrIn,
                            LPWSAOVERLAPPED lpOverlapped, LPWSACOMPLETIONROUTINE lpCompletionRoutine)
{
    _ASSERTE( IsOpen() );
    _ASSERTE( NULL != lpBuffer );

    // validate params
    if (!IsOpen() || NULL == lpBuffer)
        return (DWORD)-1L;

    SOCKET s = GetSocket();

    // Select function set write timeout
    DWORD dwBytesWritten = 0L;
    int res = 0;
    // Send message to peer
    WSABUF wsabuf;
    wsabuf.buf = (char FAR*) lpBuffer;
    wsabuf.len = dwCount;
    if (lpAddrIn)
    {
        // UDP
        res = WSASendTo( s, &wsabuf, 1, &dwBytesWritten, 0, lpAddrIn, sizeof(SOCKADDR_STORAGE),
            lpOverlapped, lpCompletionRoutine);
    }
    else // TCP
        res = WSASend( s, &wsabuf, 1, &dwBytesWritten, 0, lpOverlapped, lpCompletionRoutine);

    if ( res == SOCKET_ERROR )
    {
        res = WSAGetLastError();
        if ( res != WSA_IO_PENDING )
        {
            dwBytesWritten = (DWORD)-1L;
            SetLastError( res );
        }
    }

    return dwBytesWritten;
}
#endif

#ifdef WIN32
///////////////////////////////////////////////////////////////////////////////
// IOControl
bool CSocketHandle::IOControl(DWORD dwIoCode, LPBYTE lpInBuffer, DWORD cbInBuffer,
                              LPBYTE lpOutBuffer, DWORD cbOutBuffer,
                              LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped,
                              LPWSACOMPLETIONROUTINE lpCompletionRoutine)
{
    _ASSERTE( IsOpen() );
    // validate params
    if ( !IsOpen() ) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }
    int res;
    SOCKET s = GetSocket();
    res = WSAIoctl(s, dwIoCode, lpInBuffer, cbInBuffer, lpOutBuffer, cbOutBuffer,
                   lpcbBytesReturned, lpOverlapped, lpCompletionRoutine);
    if ( res == SOCKET_ERROR )
    {
        SetLastError( WSAGetLastError() );
    }
    return ( res != SOCKET_ERROR );
}

///////////////////////////////////////////////////////////////////////////////
// GetTransferOverlappedResult
bool CSocketHandle::GetTransferOverlappedResult(LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer,
                                 bool bWait /*= true*/, LPDWORD lpdwFlags /*= NULL*/)
{
    _ASSERTE( IsOpen() );
    _ASSERTE( NULL != lpOverlapped );

    // validate params
    if (!IsOpen() || NULL == lpOverlapped) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

    SOCKET s = GetSocket();
    DWORD dwFlags = 0;
    if ( lpdwFlags == NULL )
        lpdwFlags = &dwFlags;
    BOOL bRet = WSAGetOverlappedResult( s, lpOverlapped, lpcbTransfer, bWait, lpdwFlags );
    if ( !bRet )
    {
        SetLastError( WSAGetLastError() );
    }
    return (bRet != FALSE);
}
#endif


///////////////////////////////////////////////////////////////////////////////
// Utility functions

///////////////////////////////////////////////////////////////////////////////
// InitLibrary
bool CSocketHandle::InitLibrary(WORD wVersion)
{
#ifdef WIN32
    WSADATA WSAData = { 0 };
    return ( 0 == WSAStartup( wVersion, &WSAData ) );
#else
    return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// ReleaseLibrary
bool CSocketHandle::ReleaseLibrary()
{
#ifdef WIN32
    return ( 0 == WSACleanup() );
#else
    return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// WaitForConnection
SOCKET CSocketHandle::WaitForConnection(SOCKET sock)
{
    return accept(sock, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
// ShutdownConnection
bool CSocketHandle::ShutdownConnection(SOCKET sock)
{
    shutdown(sock, SD_BOTH);
    return ( 0 == closesocket( sock ));
}

static unsigned char chMinClassA_IP [] = { 1,   0,   0,   0   } ;
static unsigned char chMinClassD_IP [] = { 224, 0,   0,   0   } ;
static unsigned char chMaxClassD_IP [] = { 239, 255, 255, 255 } ;

///////////////////////////////////////////////////////////////////////////////
// IsUnicastIP
bool CSocketHandle::IsUnicastIP( ULONG ulAddr )
{
    return (((unsigned char *) & ulAddr) [0] >= chMinClassA_IP [0] &&
            ((unsigned char *) & ulAddr) [0] < chMinClassD_IP [0]) ;
}

///////////////////////////////////////////////////////////////////////////////
// IsMulticastIP
bool CSocketHandle::IsMulticastIP( ULONG ulAddr )
{
    return (((unsigned char *) & ulAddr) [0] >= chMinClassD_IP [0] &&
            ((unsigned char *) & ulAddr) [0] <= chMaxClassD_IP [0]) ;
}

///////////////////////////////////////////////////////////////////////////////
// FormatIP
bool CSocketHandle::FormatIP(LPTSTR pszIPAddr, UINT nSize, ULONG ulAddr, bool bFmtHost)
{
    if ( pszIPAddr && nSize > 8)
    {
        if ( bFmtHost )
            ulAddr = htonl( ulAddr );
        // Create Address string
        return (SUCCEEDED(StringCchPrintf(pszIPAddr, nSize, TEXT("%u.%u.%u.%u"),
                            (UINT)(((PBYTE) &ulAddr)[0]),
                            (UINT)(((PBYTE) &ulAddr)[1]),
                            (UINT)(((PBYTE) &ulAddr)[2]),
                            (UINT)(((PBYTE) &ulAddr)[3]))));
    }
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// FormatIP
bool CSocketHandle::FormatIP(LPTSTR pszIPAddr, UINT nSize, const SockAddrIn& addrIn)
{
    if ( pszIPAddr && nSize > 8)
    {
        const void* addr;
        char szIPAddr[MAX_PATH] = { 0 };
        if (addrIn.ss_family == AF_INET) {
            addr = &((const sockaddr_in*)&addrIn)->sin_addr;
        } else {
            addr = &((const sockaddr_in6*)&addrIn)->sin6_addr;
        }
        if (inet_ntop(addrIn.ss_family, (void *)addr, szIPAddr, MAX_PATH) != NULL)
        {
#ifdef _UNICODE
            return (0 != MultiByteToWideChar(CP_ACP, 0, szIPAddr, -1, pszIPAddr, nSize ));
#else
            ::StringCbCopyA(pszIPAddr, nSize, szIPAddr);
            return true;
#endif
        }
    }
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// GetPortNumber
USHORT CSocketHandle::GetPortNumber( LPCTSTR pszServiceName )
{
    LPSERVENT   lpservent;
    USHORT      nPort = 0;

    if ( _istdigit( pszServiceName[0] ) ) {
        nPort = (USHORT) _ttoi( pszServiceName );
    }
    else {
#ifdef _UNICODE
        char pstrService[HOSTNAME_SIZE] = { 0 };
        WideCharToMultiByte(CP_ACP, 0, pszServiceName, -1, pstrService, sizeof(pstrService), NULL, NULL );
#else
        LPCTSTR pstrService = pszServiceName;
#endif
        // Convert network byte order to host byte order
        if ( (lpservent = getservbyname( pstrService, NULL )) != NULL )
            nPort = ntohs( lpservent->s_port );
    }

    return nPort;
}

///////////////////////////////////////////////////////////////////////////////
// GetIPAddress
ULONG CSocketHandle::GetIPAddress( LPCTSTR pszHostName )
{
    LPHOSTENT   lphostent;
    ULONG       uAddr = INADDR_NONE;
    TCHAR       szLocal[HOSTNAME_SIZE] = { 0 };

    // if no name specified, get local
    if ( NULL == pszHostName || !pszHostName[0])
    {
        GetLocalName(szLocal, HOSTNAME_SIZE);
        pszHostName = szLocal;
    }

#ifdef _UNICODE
    char pstrHost[HOSTNAME_SIZE] = { 0 };
    WideCharToMultiByte(CP_ACP, 0, pszHostName, -1, pstrHost, sizeof(pstrHost), NULL, NULL );
#else
    LPCTSTR pstrHost = pszHostName;
#endif

    // Check for an Internet Protocol dotted address string
    uAddr = inet_addr( pstrHost );

    if ( (INADDR_NONE == uAddr) && (strcmp( pstrHost, "255.255.255.255" )) )
    {
        // It's not an address, then try to resolve it as a hostname
        if ( (lphostent = gethostbyname( pstrHost )) != NULL )
            uAddr = *((ULONG *) lphostent->h_addr_list[0]);
    }
    
    return ntohl( uAddr );
}

///////////////////////////////////////////////////////////////////////////////
// GetLocalName
bool CSocketHandle::GetLocalName(LPTSTR pszName, UINT nSize)
{
    if (pszName != NULL && nSize > 0)
    {
        char szHost[HOSTNAME_SIZE] = { 0 };

        // get host name, if fail, SetLastError is set
        if (SOCKET_ERROR != gethostname(szHost, sizeof(szHost)))
        {
            struct hostent* hp;
            hp = gethostbyname(szHost);
            if (hp != NULL) {
                ::StringCbCopyA(szHost, HOSTNAME_SIZE, hp->h_name);
            }

            // check if user provide enough buffer
            size_t cbLength = 0;
            ::StringCbLengthA(szHost, HOSTNAME_SIZE, &cbLength);
            if ( cbLength > nSize )
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return false;
            }

            // Unicode conversion
#ifdef _UNICODE
            return (0 != MultiByteToWideChar(CP_ACP, 0, szHost, -1, pszName, nSize ));
#else
            ::StringCbCopyA(pszName, nSize, szHost);
            return true;
#endif
        }
        else
            SetLastError( WSAGetLastError() );
    }
    else
        SetLastError(ERROR_INVALID_PARAMETER);
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// GetLocalAddress
bool CSocketHandle::GetLocalAddress(LPTSTR pszAddress, UINT nSize, int nFamily /*= AF_INET*/)
{
    if (pszAddress != NULL && nSize > 0)
    {
        TCHAR szHost[HOSTNAME_SIZE] = { 0 };

        // Get computer local address
        // get host name, if fail, SetLastError is set
        if (GetLocalName(szHost, HOSTNAME_SIZE))
        {
            char szAddress[MAX_PATH] = { 0 };

#ifdef _UNICODE
            char pstrHost[HOSTNAME_SIZE] = { 0 };
            WideCharToMultiByte(CP_ACP, 0, szHost, -1, pstrHost, sizeof(pstrHost), NULL, NULL );
#else
            LPCTSTR pstrHost = szHost;
#endif
            // get address info
            sockaddr_storage addr_store = { 0 };
            addr_store.ss_family = static_cast<short>(nFamily);
            inet_pton(nFamily, pstrHost, &addr_store);
            const void* addr;
            if (addr_store.ss_family == AF_INET) {
                addr = &((const sockaddr_in*)&addr_store)->sin_addr;
            } else {
                addr = &((const sockaddr_in6*)&addr_store)->sin6_addr;
            }
            if (inet_ntop(addr_store.ss_family,(void *) addr, szAddress, MAX_PATH) != NULL)
            {
                // Unicode conversion
#ifdef _UNICODE
                return (0 != MultiByteToWideChar(CP_ACP, 0, szAddress, -1, pszAddress, nSize ));
#else
                ::StringCbCopyA(pszAddress, nSize, szAddress);
                return true;
#endif
            }
            else
                SetLastError( WSAGetLastError() );
        }
    }
    else
        SetLastError(ERROR_INVALID_PARAMETER);
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// GetAddressInfo
bool CSocketHandle::GetAddressInfo(LPCTSTR pszHostName, LPCTSTR pszServiceName,
                                   int nFamily, SockAddrIn& sockAddr)
{
    const TCHAR szZERO[] = TEXT("0");
    ADDRINFO aiHints;
    ADDRINFO *aiList = NULL;
    memset(&aiHints, 0, sizeof(aiHints));
    aiHints.ai_flags = AI_ADDRCONFIG;
    aiHints.ai_family = static_cast<short>(nFamily);

    TCHAR szLocal[HOSTNAME_SIZE] = { 0 };
    // if no name specified, get local
    if ( NULL == pszHostName || !pszHostName[0] )
    {
        GetLocalName(szLocal, HOSTNAME_SIZE);
        pszHostName = szLocal;
    }
    if ( NULL == pszServiceName || !pszServiceName[0] )
    {
        pszServiceName = szZERO;
    }

#ifdef _UNICODE
    char pstrHost[HOSTNAME_SIZE] = { 0 };
    WideCharToMultiByte(CP_ACP, 0, pszHostName, -1, pstrHost, sizeof(pstrHost), NULL, NULL );
    char pstrService[HOSTNAME_SIZE] = { 0 };
    WideCharToMultiByte(CP_ACP, 0, pszServiceName, -1, pstrService, sizeof(pstrService), NULL, NULL );
#else
    LPCTSTR pstrHost = pszHostName;
    LPCTSTR pstrService = pszServiceName;
#endif
    if ( SOCKET_ERROR != getaddrinfo(pstrHost, pstrService, &aiHints, &aiList) && ( aiList != 0 ))
    {
        ADDRINFO ai = { 0 };
        ai.ai_addr = sockAddr;
        memcpy(ai.ai_addr, aiList->ai_addr, aiList->ai_addrlen);
        freeaddrinfo( aiList );
        return true;
    }
    SetLastError( WSAGetLastError() );
    return false;
}

bool CSocketHandle::GetLocalNameA( char * pszName, UINT nSize )
{
	if (pszName != NULL && nSize > 0)
	{
		char szHost[HOSTNAME_SIZE] = { 0 };

		// get host name, if fail, SetLastError is set
		if (SOCKET_ERROR != gethostname(szHost, sizeof(szHost)))
		{
			struct hostent* hp;
			hp = gethostbyname(szHost);
			if (hp != NULL) {
				::StringCbCopyA(szHost, HOSTNAME_SIZE, hp->h_name);
			}

			// check if user provide enough buffer
			size_t cbLength = 0;
			::StringCbLengthA(szHost, HOSTNAME_SIZE, &cbLength);
			if ( cbLength > nSize )
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				return false;
			}

			::StringCbCopyA(pszName, nSize, szHost);
			return true;
		}
		else
			SetLastError( WSAGetLastError() );
	}
	else
		SetLastError(ERROR_INVALID_PARAMETER);
	return false;
}


///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////
#if !defined(PLATFORM_HAS_INETFUNC)
///////////////////////////////////////////////////////////////////////////////
// inet_ntop
const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
    if ( dst != NULL)
    {
        dst[0] = 0;
        if (af == AF_INET)
        {
            sockaddr_in in;
            memset(&in, 0, sizeof(in));
            in.sin_family = AF_INET;
            memcpy(&in.sin_addr, src, sizeof(in_addr));
            getnameinfo((sockaddr *)&in, sizeof(sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
            return dst;
        }
        else if (af == AF_INET6)
        {
            sockaddr_in6 in;
            memset(&in, 0, sizeof(in));
            in.sin6_family = AF_INET6;
            memcpy(&in.sin6_addr, src, sizeof(in6_addr));
            getnameinfo((sockaddr *)&in, sizeof(sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
            return dst;
        }
    }
    WSASetLastError(WSA_INVALID_PARAMETER);
    return dst;
}

///////////////////////////////////////////////////////////////////////////////
// inet_pton
int inet_pton(int af, const char *src, void *dst)
{
    int result = SOCKET_ERROR;
    addrinfo aiHints, *aiList = NULL;
    memset(&aiHints, 0, sizeof(aiHints));
    aiHints.ai_family = af;

    if ( SOCKET_ERROR != getaddrinfo(src, NULL, &aiHints, &aiList) && (aiList != NULL))
    {
        memcpy(dst, aiList->ai_addr, aiList->ai_addrlen);
        freeaddrinfo(aiList);
        result = 0;
    }
    return result;
}


#endif
