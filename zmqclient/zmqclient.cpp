#include <iostream>


#include "zmq.hpp"
#include "zhelpers.hpp"
#include "asyndealer.h"
#include "talk/base/thread.h"

#include "peerconnectionclientdealer.h"
#include "CameraClient.h"
#include "talk/base/logging.h"
#ifndef ARM
#include "KeVideoSimulator.h"
#else
#include "HisiMediaDevice.h"

#endif//arm
using namespace std;



typedef talk_base::TypedMessageData<std::string> StringMsg ;

struct SendInfo{
    std::string peer_id;
    std::string data;
};
typedef talk_base::TypedMessageData<SendInfo> SendMsg ;


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


int main()
{

    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);

    CameraClient client;

    //client.Connect("tcp://192.168.0.182:5555","123456");
    client.Connect("tcp://192.168.40.191:5555","1234567");
    client.Login();

//    talk_base::scoped_ptr<PeerTerminal> terminal;
//    terminal.reset(new PeerTerminal());
//    terminal->Initialize(&client);

#ifndef ARM
    KeVideoSimulator * simulator = new KeVideoSimulator();
    simulator->Initialize(&client);
    simulator->ReadVideoData("video.h264");
#else
    HisiMediaDevice * device = new HisiMediaDevice();
    device->Init(&client);

#endif //arm


    //simulator->ConnectToPeer("444444");
    //simulator->CloseTunnel("444444");

    talk_base::Thread::Current()->Run();
//    while(true){
//        std::string msg;
//        std::cout<<"input command"<<std::endl;
//        std::cin>>msg;

//        std::string cmd;
//        std::string data;
//        if(!SplitByDelimiter(msg,':',&cmd,&data)){
//            std::cout<<msg<<std::endl;
//        }
//        else{
//            if(cmd.compare("stop")==0){
//                //p2p.Post(NULL,kaerp2p::MSG_DONE);
//            }else if(cmd.compare("connect")==0){
//                terminal->ConnectToPeer(data);
//            }else if(cmd.compare("send")==0){
//                terminal->SendByTunnel(data);
//            }else if(cmd.compare("send_router")==0){

//            }
//            else if(cmd.compare("close_tunnel")==0){
//                terminal->CloseTunnel();
//            }else if(cmd.compare("sendstream")==0){
//                std::string strPackSize = data;
//                int buffer_len = atoi(strPackSize.c_str());
//                if(buffer_len == 0){
//                    buffer_len= 1024;
//                }
//                char * buffer = new char[buffer_len];
//                for(int i=0;i<buffer_len;i++){
//                    buffer[i] = static_cast<char>(rand());
//                }
//                std::cout<<"send "<<buffer_len<<" bytes per 40ms"<<std::endl;

//                for(int i=0 ;i< 99999999;i++){
//                    terminal->SendByTunnel(buffer,buffer_len);
//                    talk_base::Thread::SleepMs(40);
//                }
//                delete [] buffer;


//            }else{
//                //StringMsg *msgData = new  StringMsg(data);
//                //control_thread->Post(controll,Controller::CONNECT_TO_PEER,msgData);

//                std::cout << "not support cmd";
//            }
//        }

//        talk_base::Thread::SleepMs(10);
//    }

    return 0;
}

