#include <QCoreApplication>

#include "p2pthread.h"
#include <iostream>
int main(int argc,char *argv[])
{

        QCoreApplication a(argc,argv);
        P2PThread p2p;
        p2p.start();
        int b;
        std::cin>>b;
        std::cout<<b;
//    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);
//    PeerConnectionClient client;
//    talk_base::scoped_refptr<kaerp2p::ServerConductor> conductor(
//                new talk_base::RefCountedObject<kaerp2p::ServerConductor>(&client));

//    std::string serverIp = "192.168.40.195";
//    conductor->StartLogin(serverIp,8888);
//    std::cout<<"connect end"<<std::endl;

//    std::vector<uint32> ids;
//    ids.push_back(kaerp2p::MSG_DONE);
//    ids.push_back(kaerp2p::MSG_LOGIN_FAILED);
//    Loop(ids);
    return a.exec();
}

