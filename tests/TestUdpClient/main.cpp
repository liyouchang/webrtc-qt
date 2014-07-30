#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>//for sockaddr_in
#include <arpa/inet.h>//for socket

using namespace std;

int main()
{
    int fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd==-1)
    {
        perror("socket create error!\n");
        exit(-1);
    }
    printf("socket fd=%d\n",fd);


    struct sockaddr_in addr_to;//目标服务器地址
    addr_to.sin_family=AF_INET;
    addr_to.sin_port=htons(4567);
    addr_to.sin_addr.s_addr=inet_addr("192.168.40.195");

    struct sockaddr_in addr_from;
    addr_from.sin_family=AF_INET;
    addr_from.sin_port=htons(0);//获得任意空闲端口
    addr_from.sin_addr.s_addr=htons(INADDR_ANY);//获得本机地址

    int r;
    r=bind(fd,(struct sockaddr*)&addr_from,sizeof(addr_from));
    if(r==-1)
    {
        printf("Bind error!\n");
        close(fd);
        exit(-1);
    }
    printf("Bind successfully.\n");


    char buf[1400];
    int len = 1400;
    while(1)
    {
//        r=read(0,buf,sizeof(buf));
//        if(r<0)
//        {
//            break;
//        }
        len=sendto(fd,buf,1400,0,(struct sockaddr*)&addr_to,sizeof(addr_to));
        if(len==-1)
        {
            printf("send falure!\n");
        }
        else
        {
            //printf("%d bytes have been sended successfully!\n",len);
        }
    }
    close(fd);
    return 0;
}
