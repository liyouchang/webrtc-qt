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
** File:        SocketHandle.h
** Version:     1.4 - IPv6 support
**              1.3 - Update for Asynchronous mode / Linux port
**              1.2 - Update interface for TCP remote connection
**              1.1 - Added multicast support
*/
#ifndef SOCKETHANDLE_H
#define SOCKETHANDLE_H
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#elif BSD_SOCKET
#include "platform.h"
#endif


#if !defined(PLATFORM_HAS_INETFUNC)
const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
int inet_pton(int af, const char *src, void *dst);
#endif

/**
 * @defgroup _SockHandle Socket Communication Classes
 * @{
 */

/**
 * SockAddrIn structure
 * Encapsulate SOCKADDR_STORAGE (IPv4: SOCKADDR_IN, IPv6: SOCKADDR_IN6) structures
 */
struct SockAddrIn : public sockaddr_storage {
public:
    /**
     * Default constructor
     */
    SockAddrIn();

    /**
     * Copy constructor
     * @param sin reference object
     */
    SockAddrIn(const SockAddrIn& sin);

    /**
     * Destructor
     */
    ~SockAddrIn();

    /**
     * Copy function
     * @param sin reference object
     * @return reference of 'this' object
     */
    SockAddrIn& Copy(const SockAddrIn& sin);

    /**
     * Clear struct
     */
    void    Clear();

    /**
     * Compare NetworkAddr object
     * @param sin reference object
     * @return true if equal, otherwise false
     */
    bool    IsEqual(const SockAddrIn& sin) const;

    /**
     * Check if NULL
     * @return true if object is 'NULL' - not valid
     */
    bool    IsNull() const { return IsEqual(NULLAddr); }

    /**
     * Get Socket address family
     * @return Socket address family (IPv4: AF_INET, IPv6: AF_INET6)
     */
    short GetFamily () const { return ss_family; }

    /**
     * Get IP address (IPv4)
     * @return IPv4 network address in network format
     */
	ULONG   GetIPAddr() const { return ((SOCKADDR_IN*)this)->sin_addr.s_addr; }

    /**
     * Get Port
     * @return Port number in network format
     */
    short   GetPort() const { return ((SOCKADDR_IN*)this)->sin_port; }

    /**
     * Create from string. Service is service name or port number.
     * @param pszAddr Hostname or IP address
     * @param pszService Service name or port
     * @param nFamily Socket address family (default: AF_INET, AF_INET6)
     * @return true if object is now valid
     */
    bool    CreateFrom(LPCTSTR pszAddr, LPCTSTR pszService, int nFamily = AF_INET);

    /**
     * Create from number. Initialize object from host or network format values (IPv4 only).
     * @param lIPAddr IP Address
     * @param nPort Port number
     * @param nFamily Socket address family (default: AF_INET)
     * @param bFmtHost flag to indicate that IP address and port are in host (true) or network format (false)
     * @return true if object is now valid
     */
    bool    CreateFrom(ULONG lIPAddr, USHORT nPort, int nFamily = AF_INET, bool bFmtHost = true);

    /**
     * Object copy operator
     * @param sin reference object to copy from
     * @return reference of 'this' object
     */
    SockAddrIn& operator=(const SockAddrIn& sin) { return Copy( sin ); }

    /**
     * Equality operator
     * @param sin reference object to test
     * @return true if object is equal
     */
    bool    operator==(const SockAddrIn& sin) const { return IsEqual( sin ); }
	
    /**
     * Not Equal operator
     * @param sin reference object to test
     * @return true if object is different
     */
    bool    operator!=(const SockAddrIn& sin) const { return !IsEqual( sin ); }
	
    /**
     * SOCKADDR conversion. Return SOCKADDR of this object.
     */
    operator LPSOCKADDR() { return reinterpret_cast<LPSOCKADDR>(this); }

    /**
     * PIN6_ADDR conversion. Return const PIN6_ADDR of this object.
     */
    operator const IN6_ADDR*() const { return reinterpret_cast<const IN6_ADDR*>(this); }

    /**
     * PIN6_ADDR conversion. Return PIN6_ADDR of this object.
     */
    operator PIN6_ADDR() { return reinterpret_cast<PIN6_ADDR>(this); }

    /**
     * Size of this struct (SOCKADDR_IN) or (SOCKADDR_STORAGE) depending on address family
     * @return size of this structure (SOCKADDR_IN) or (SOCKADDR_STORAGE)
     */
    size_t  Size() const { return (ss_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_storage); }

    /**
     * Initialize this object from SOCKADDR_IN struct.
     * @param psin SOCKADDR_IN object pointer
     */
    void    SetAddr(const sockaddr_in* psin) { SetAddr(reinterpret_cast<const sockaddr_storage*>(psin)); }

    /**
     * Initialize this object from SOCKADDR_IN6 struct.
     * @param psin SOCKADDR_IN6 object pointer
     */
    void    SetAddr(const sockaddr_in6* psin) { SetAddr(reinterpret_cast<const sockaddr_storage*>(psin)); }

    /**
     * Initialize this object from SOCKADDR_STORAGE struct.
     * @param pss SOCKADDR_STORAGE object pointer
     */
    void    SetAddr(const sockaddr_storage* pss) { ss_family = pss->ss_family; memcpy(this, pss, Size()); }

    static SockAddrIn NULLAddr;    ///< Null Address
};

#ifdef WIN32
typedef LPWSAOVERLAPPED_COMPLETION_ROUTINE LPWSACOMPLETIONROUTINE;
#endif

/**
 * CSocketHandle class
 * Socket communication class
 */
class CSocketHandle
{
public:
    /**
     * Default constructor
     */
    CSocketHandle();

    /**
     * Destructor
     */
    ~CSocketHandle();

    /**
     * Check if socket is opened
     * @return true if this is opened (valid)
     */
    bool IsOpen() const;

    /**
     * Get SOCKET handle
     * @return SOCKET handle (handle is INVALID_SOCKET if object is closed)
     */
    SOCKET GetSocket() const;

    /**
     * Get SOCKET type
     * @return SOCKET type (-1 if not a valid socket)
     */
    int GetSocketType() const;

    /**
     * Attach a socket handle
     * This function may fail is socket handle is not valid or object already in used. Call Detach or Close to release object.
     * @param sock Socket handle to attach to this class
     * @return true if successful, otherwise false
     */
    bool Attach(SOCKET sock);

    /**
     * Detach a socket handle
     * @return previous socket handle or INVALID_SOCKET
     */
    SOCKET Detach();

    /**
     * Create a Socket - Server side.
     * @param pszHost hostname or IP address of adapter
     * @param pszServiceName Network service name or port number
     * @param nFamily address family to use (AF_INET, AF_INET6)
     * @param nType type of socket to create (SOCK_STREAM, SOCK_DGRAM)
     * @param uOptions Additional options (SO_BROADCAST, SO_REUSEADDR)
     * @return true if successful, otherwise false (call WSAGetLastError() to retrieve latest error)
     * @sa InitLibrary, ConnectTo, IsOpen
     */
    bool CreateSocket(LPCTSTR pszHost, LPCTSTR pszServiceName, int nFamily, int nType, UINT uOptions = 0);

    /**
     * Create a socket, connect to a server - Client side.
     * @param pszHostName Hostname or NIC address
     * @param pszRemote Remote network address
     * @param pszServiceName Network service name or port number
     * @param nFamily address family to use (AF_INET, AF_INET6)
     * @param nType type of socket to create (SOCK_STREAM, SOCK_DGRAM)
     * @return true if successful, otherwise false (call WSAGetLastError() to retrieve latest error)
     * @sa InitLibrary, CreateSocket, IsOpen
     */
    bool ConnectTo(LPCTSTR pszHostName, LPCTSTR pszRemote, LPCTSTR pszServiceName, int nFamily, int nType);

    /**
     * Close Socket
     * @sa InitLibrary, CreateSocket, ConnectTo, IsOpen
     */
    void Close();

    /**
     * Read from socket
     * @param lpBuffer Buffer to receive data
     * @param dwSize Size of buffer in bytes
     * @param lpAddrIn Peer address for UDP - this must be NULL for TCP
     * @param dwTimeout Read timeout in milliseconds
     * @return number of bytes read or (-1L) if fail
     * @sa InitLibrary, CreateSocket, ConnectTo, IsOpen, ReadEx, Write, WriteEx
     */
    DWORD Read(LPBYTE lpBuffer, DWORD dwSize, LPSOCKADDR lpAddrIn = NULL, DWORD dwTimeout = INFINITE);

#ifdef WIN32
    /**
     * Read from socket (asynchronous mode).
     * @param lpBuffer Buffer to receive data
     * @param dwSize Size of buffer in bytes
     * @param lpAddrIn SockAddrIn for UDP - this must be NULL for TCP
     * @param lpOverlapped Windows Overlapped structure (required)
     * @param lpCompletionRoutine Winsock Completion routine (required)
     * @return number of bytes read, overlapped operation is pending or (-1L) if fail
     * @sa InitLibrary, CreateSocket, ConnectTo, IsOpen, Read, Write, WriteEx, IOControl, GetTransferOverlappedResult
     */
    DWORD ReadEx(LPBYTE lpBuffer, DWORD dwSize, LPSOCKADDR lpAddrIn,
                LPWSAOVERLAPPED lpOverlapped, LPWSACOMPLETIONROUTINE lpCompletionRoutine);
#endif
    /**
     * Write to a destination socket
     * @param lpBuffer Buffer to send
     * @param dwCount Number of bytes to send
     * @param lpAddrIn Peer address for UDP - this must be NULL for TCP
     * @param dwTimeout Write timeout in milliseconds
     * @return number of bytes sent or (-1L) if fail
     * @sa InitLibrary, CreateSocket, ConnectTo, IsOpen, Read, ReadEx, WriteEx
     */
    DWORD Write(const LPBYTE lpBuffer, DWORD dwCount, const LPSOCKADDR lpAddrIn = NULL, DWORD dwTimeout = INFINITE);

#ifdef WIN32
    /**
     * Write to a destination socket (asynchronous mode).
     * @param lpBuffer Buffer to send
     * @param dwCount Number of bytes to send
     * @param lpAddrIn SockAddrIn for UDP - this must be NULL for TCP
     * @param lpOverlapped Windows Overlapped structure (required)
     * @param lpCompletionRoutine Winsock Completion routine (required)
     * @return number of bytes read, overlapped operation is pending or (-1L) if fail
     * @sa InitLibrary, CreateSocket, ConnectTo, IsOpen, Read, ReadEx, Write, IOControl, GetTransferOverlappedResult
     */
    DWORD WriteEx(const LPBYTE lpBuffer, DWORD dwCount, const LPSOCKADDR lpAddrIn,
                LPWSAOVERLAPPED lpOverlapped, LPWSACOMPLETIONROUTINE lpCompletionRoutine);
#endif

#ifdef WIN32
    /**
     * Control the mode of a socket (asynchronous mode)
     * @param dwIoCode Control code of operation to perform
     * @param lpInBuffer Pointer to the input buffer
     * @param cbInBuffer Size of the input buffer, in bytes
     * @param lpOutBuffer Pointer to the output buffer
     * @param cbOutBuffer Size of the output buffer, in bytes
     * @param lpcbBytesReturned Pointer to actual number of bytes of output
     * @param lpOverlapped Winsock Overlapped structure
     * @param lpCompletionRoutine Winsock Completion routine
     * @return true if successful, otherwise false (call WSAGetLastError() to retrieve latest error)
     * @sa InitLibrary, CreateSocket, ConnectTo, IsOpen, ReadEx, WriteEx, GetTransferOverlappedResult
     */
    bool IOControl(DWORD dwIoCode, LPBYTE lpInBuffer, DWORD cbInBuffer,
               LPBYTE lpOutBuffer, DWORD cbOutBuffer,
               LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped,
               LPWSACOMPLETIONROUTINE lpCompletionRoutine);

    /**
     * Get Overlapped result (asynchronous mode)
     * @param lpOverlapped Windows Overlapped structure (required)
     * @param lpcbTransfer Pointer to get number of bytes transferred
     * @param bWait Force wait for overlapped operation to complete
     * @param lpdwFlags Optional flags (see MSDN on WSARecv API)
     * @sa InitLibrary, CreateSocket, ConnectTo, IsOpen, ReadEx, WriteEx, IOControl
     */
    bool GetTransferOverlappedResult(LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer, bool bWait = true, LPDWORD lpdwFlags = 0);
#endif

    /**
     * Get Socket name - get local address
     * @param saddr_in current local address and port when connected
     * @return true if successful
     * @sa InitLibrary
     */
    bool GetSockName(SockAddrIn& saddr_in) const;

    /**
     * Get Peer Socket name - get peer address
     * @param saddr_in peer address and port (use only with TCP or client mode UDP)
     * @return true if successful
     * @sa InitLibrary
     */
    bool GetPeerName(SockAddrIn& saddr_in) const;

    /**
     * Register to multicast address
     * @param pszIPAddr multicast IP group
     * @param pszNIC interface IP address
     * @return true if successful
     * @sa InitLibrary
     */
    bool AddMembership(LPCTSTR pszIPAddr, LPCTSTR pszNIC);

    /**
     * Unregister from a multicast address
     * @param pszIPAddr multicast IP group
     * @param pszNIC interface IP address
     * @return true if successful
     * @sa InitLibrary
     */
    bool DropMembership(LPCTSTR pszIPAddr, LPCTSTR pszNIC);

    /**
     * Initialize Winsock library. This function calls WSAStartup.
     * @param wVersion Winsock version use MAKEWORD macro if possible (e.g.: MAKEWORD(2,2))
     * @return true if successful
     * @sa ReleaseLibrary
     */
    static bool InitLibrary(WORD wVersion);

    /**
     * Release Winsock library
     * @return true if successful
     * @sa InitLibrary
     */
    static bool ReleaseLibrary();

    /**
     * Wait for a new connection
     * @param sock A TCP socket handle. A new socket is return returned.
     * @return A new socket when a new client connects
     * @sa GetSocket, CreateSocket
     */
    static SOCKET WaitForConnection(SOCKET sock);

    /**
     * Shutdown a connection
     * @param sock Socket to shutdown communication
     * @return true if successful
     */
    static bool ShutdownConnection(SOCKET sock);

    /**
     * Check if IP address is unicast (network order).
     * @param ulAddr IP address (expected valid unicast address)
     * @return true if successful
     */
    static bool IsUnicastIP( ULONG ulAddr );

    /**
     * Check if IP address is multicast (network order).
     * @param ulAddr IP address (expected valid multicast address)
     * @return true if successful
     */
    static bool IsMulticastIP( ULONG ulAddr );

    /**
     * Format IP address to string
     * @param pszIPAddr Buffer to hold string
     * @param nSize Size of buffer in characters
     * @param ulAddr IP Address to format
     * @param bFmtHost Specify if address (ulAddr) is in host (true) or network format (false)
     * @return true if successful. Possible error could be INSUFFICIENT_BUFFER
     */
    static bool FormatIP(LPTSTR pszIPAddr, UINT nSize, ULONG ulAddr, bool bFmtHost);

    /**
     * Format IP address to string
     * @param pszIPAddr Buffer to hold string
     * @param nSize Size of buffer in characters
     * @param addrIn IP Address to format
     * @return true if successful. Possible error could be INSUFFICIENT_BUFFER
     */
    static bool FormatIP(LPTSTR pszIPAddr, UINT nSize, const SockAddrIn& addrIn);

    /**
     * Get service port number
     * @param pszServiceName Network service name (e.g.: "ftp", "telnet") or port number
     * @return port number
     */
    static USHORT GetPortNumber( LPCTSTR pszServiceName );

    /**
     * Get IP address of a host
     * @param pszHostName host name or IP address
     * @return Host IP address in host format
     * @sa GetAddressInfo
     */
    static ULONG GetIPAddress( LPCTSTR pszHostName );

    /**
     * Get current localname for this machine
     * @param pszName Buffer to receive host name
     * @param nSize Size of this buffer in character
     * @return true if successful
     */
    static bool GetLocalName(LPTSTR pszName, UINT nSize);
	static bool GetLocalNameA(char * pszName, UINT nSize);
    /**
     * Get current (default) IP address for this machine
     * @param pszAddress Buffer to receive IP address (IPv4, IPv6 format)
     * @param nSize Size of this buffer in character
     * @param nFamily address family to use (AF_INET, AF_INET6)
     * @return true if successful
     */
    static bool GetLocalAddress(LPTSTR pszAddress, UINT nSize, int nFamily = AF_INET);

    /**
     * Get IP address info of a host (Supports: IPv4 and IPv6)
     * @param pszHostName host name or IP address
     * @param pszServiceName pszServiceName Network service name (e.g.: "ftp", "telnet") or port number
     * @param nFamily address family to use (AF_INET, AF_INET6)
     * @param sockAddr Socket address to fill in
     * @return true if successful
     */
    static bool  GetAddressInfo( LPCTSTR pszHostName, LPCTSTR pszServiceName, int nFamily, SockAddrIn& sockAddr);

// CSocketHandle - data
protected:
    SOCKET      m_hSocket;  ///< socket handle
};

/** @}*/

#endif //SOCKETHANDLE_H
