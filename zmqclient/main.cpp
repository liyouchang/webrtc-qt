#include <iostream>


#include "zmq.hpp"
#include "zhelpers.hpp"
#include "asyndealer.h"
#include "talk/base/thread.h"
#include "peerterminal.h"
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


class Controller : public talk_base::MessageHandler
{
public:
    enum{
        CONTROL_INIT,
        CONNECT_TO_PEER,
        CLIENT_SEND,
        TUNNEL_CLOSE,
        TUNNEL_SEND
    };
    Controller(talk_base::Thread * control_thread){
        control_thread_ = control_thread;
    }
    void init(){
        std::string router = "tcp://192.168.0.182:5555";
        StringMsg *msgData = new  StringMsg(router);
        control_thread_->Post(this,CONTROL_INIT,msgData);
    }

    talk_base::scoped_ptr<PeerTerminal> terminal_;
    talk_base::Thread * control_thread_;
    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg){
        switch(msg->message_id){
        case CONTROL_INIT:{
            talk_base::scoped_ptr<StringMsg> msgData(static_cast<StringMsg*>(msg->pdata));
            terminal_.reset(new PeerTerminal());
            terminal_->Initialize(msgData->data());
        }
            break;
        case CONNECT_TO_PEER:{
            talk_base::scoped_ptr<StringMsg> msgData(
                        static_cast<StringMsg*>(msg->pdata));
            terminal_->ConnectToPeer(msgData->data());

        }
            break;
        case CLIENT_SEND:{
            talk_base::scoped_ptr<StringMsg> msgData(
                        static_cast<StringMsg*>(msg->pdata));

            std::string data = msgData->data();

            std::string id;
            std::string msg;
            SplitByDelimiter(data,'/',&id,&msg);
            terminal_->SendByRouter(id,msg);
        }
            break;
        case TUNNEL_CLOSE:{
            terminal_->CloseTunnel();
            break;
        }
        case TUNNEL_SEND:{
            talk_base::scoped_ptr<StringMsg> msgData(
                        static_cast<StringMsg*>(msg->pdata));
            terminal_->SendByTunnel(msgData->data());
            break;
        }
        default:
            break;
        }
    }
};
int main()
{

    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);

    talk_base::Thread * control_thread = new talk_base::Thread();
    control_thread->Start();
    Controller *controll = new Controller(control_thread);

    controll->init();

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
                //p2p.Post(NULL,kaerp2p::MSG_DONE);
            }else if(cmd.compare("connect")==0){
                StringMsg *msgData = new  StringMsg(data);
                control_thread->Post(controll,Controller::CONNECT_TO_PEER,msgData);
            }else if(cmd.compare("send")==0){
                StringMsg *msgData = new  StringMsg(data);
                control_thread->Post(controll,Controller::TUNNEL_SEND,msgData);
            }else if(cmd.compare("send_router")==0){

            }
            else if(cmd.compare("close_tunnel")==0){
                StringMsg *msgData = new  StringMsg(data);
                control_thread->Post(controll,Controller::TUNNEL_CLOSE,msgData);

            }else if(cmd.compare("sendstream")==0){



            }else{

                std::cout << "not support cmd";
            }


        }

        talk_base::Thread::SleepMs(10);
    }

    return 0;
}

