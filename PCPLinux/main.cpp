//#include <QCoreApplication>

#include "p2pthread.h"
#include <iostream>


// Split the message into two parts by the first delimiter.
static bool SplitByDelimiter(const std::string& message,
                             const char delimiter,
                             std::string* field1,
                             std::string* field2) {
    // Find the first delimiter
    size_t pos = message.find(delimiter);
    if (pos == std::string::npos) {
        return false;
    }
    *field1 = message.substr(0, pos);
    // The rest is the value.
    *field2 = message.substr(pos + 1);
    return true;
}

int main(int argc,char *argv[])
{

    //QCoreApplication a(argc,argv);
    kaerp2p::P2PThread p2p;
    p2p.Start();
    while(true){
        std::string msg;
        std::cout<<"input command"<<std::endl;
        std::cin>>msg;

        std::string cmd;
        std::string data;
        if(!SplitByDelimiter(msg,':',&cmd,&data)){
            std::cout<<msg<<std::endl;
        }
        else{
            if(cmd.compare("stop")==0){
                p2p.Post(NULL,kaerp2p::MSG_DONE);
            }else if(cmd.compare("connect")==0){
                int peer_id = atoi(data.c_str());
                talk_base::TypedMessageData<int> *msgData =
                        new  talk_base::TypedMessageData<int>(peer_id);
                p2p.Post(&p2p,kaerp2p::MSG_CONNECT_TO_PEER,msgData);
            }else if(cmd.compare("send")==0){
                talk_base::TypedMessageData<std::string> *msgData =
                        new  talk_base::TypedMessageData<std::string>(data);
                p2p.Post(&p2p,kaerp2p::MSG_SEND_STRING,msgData);
            }else{
                std::cout << "not support cmd";
            }


        }

        talk_base::Thread::SleepMs(10);
    }
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
    //   return a.exec();
    return 0;
}

