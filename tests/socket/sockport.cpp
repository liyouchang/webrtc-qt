/* Simple Socket Client  */
#include <stdlib.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include "SocketHandle.h"

using namespace std;

void *SimpleClient(void *threadp);

pthread_mutex_t mutexlck;
CSocketHandle sockHandle;

void LockMutex(bool lock)
{
    if ( lock ) {
        pthread_mutex_lock (&mutexlck);
    } else {
        pthread_mutex_unlock (&mutexlck);
    }
}

int main(int argc, char* argv[])
{
    pthread_t threads;
    void *status;
    pthread_mutex_init(&mutexlck, NULL);
    // runas: sockclient <hostname> <port>
    if ( argc < 3 ) {
        cout << "Runas: sockclient <hostname-or-ipaddress> <port>\n";
        pthread_mutex_destroy(&mutexlck);
        exit(1);
    }

    if (sockHandle.ConnectTo(NULL, argv[1], argv[2], AF_INET, SOCK_STREAM))
    {
        LockMutex(true);
        cout << "Connection established with Server." << std::endl << std::flush;
        int rc = pthread_create(&threads, NULL, SimpleClient, NULL);
        if (rc) {
            cout << "ERROR; return code from pthread_create() is " << rc << std::endl;
            LockMutex(false);
            exit(-1);
        }
        string input;
        cout << "Enter QUIT or 'Q' to quit." << std::endl << std::flush;
        LockMutex(false);

        while( true )
        {
            getline(cin, input);
            if ( input == "Q" || input == "QUIT" )
            {
                cout << "Quit!" << std::endl << std::flush;
                break;
            }
            sockHandle.Write((const LPBYTE)(input.c_str()), input.size(), NULL);
        }
        sockHandle.Close();
        rc = pthread_join(threads, &status);
    }
    else
    {
        cout << "Failed to connect to server." << std::endl;
    }
    pthread_mutex_destroy(&mutexlck);
    pthread_exit(NULL);
}

void *SimpleClient(void *threadp)
{
    char szIPAddr[40] = { 0 };

    SockAddrIn addrIn;
    sockHandle.GetPeerName( addrIn );
    CSocketHandle::FormatIP(szIPAddr, 40, addrIn.GetIPAddr(), false);
    LockMutex(true);
    cout << "Connection with server: " << szIPAddr << ":" << ntohs(addrIn.GetPort()) << std::endl << std::flush;
    LockMutex(false);

    DWORD dwRead;
    BYTE buffer[256];
    while (sockHandle.IsOpen())
    {
        dwRead = sockHandle.Read(buffer, 255, NULL);
        if ( dwRead == 0xffffffff )
        {
            cout << "Connection lost (thread exit)..." << std::endl << std::flush;
            break;
        }
        if ( dwRead > 0 )
        {
            buffer[dwRead] = 0;
            LockMutex(true);
            cout << ">>" << reinterpret_cast<char*>(buffer) << std::endl << std::flush;
            LockMutex(false);
        }
    }
    pthread_exit(NULL);
}
