#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <WinSock.h>
using namespace std;

class CInitSock
{
public:
    CInitSock(){
        WORD sockVersion = MAKEWORD(2,2);
        WSADATA wsaData = {0};
        if(WSAStartup(sockVersion,&wsaData)){
            cout <<"initlization failed! "<<endl;
            exit(0);
        }
    }
    ~CInitSock(){
        WSACleanup();
    }
};

CInitSock mysock;

int main()
{
    SOCKET s = 0;
    sockaddr_in sin = {0};

    sin.sin_family = AF_INET;
    sin.sin_port = htons(4567);
    sin.sin_addr.S_un.S_addr = INADDR_ANY;
    s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    if(bind(s,(LPSOCKADDR)&sin,sizeof(sin)) == SOCKET_ERROR){
        cout <<"bind failed "<<endl;
        return 0;
    }
    char buffer[1600] = "\0";
    sockaddr_in addr = {0};
    int nLen = 0;

    nLen = sizeof(sockaddr_in);
    cout <<"-------------now service start--------"<<endl;
    while(true){
        int nRecv = 0;
        //cout <<"wait to receive data ......"<<endl;
        nRecv = recvfrom(s,buffer,1600,0,(sockaddr*)&addr ,&nLen);
        if(nRecv > 0){
            buffer[nRecv] = '\0';
            //cout << "recve buffer: " <<buffer <<endl;
        }
    }

    closesocket(s);

    return 0;
}

