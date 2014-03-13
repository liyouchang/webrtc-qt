#include "p2pthread.h"
#include <iostream>
#include <vector>
#include "talk/base/thread.h"
#include "peer_connection_client.h"
#include "talk/base/logging.h"

namespace  kaerp2p {


P2PThread::P2PThread()
{
}


uint32 Loop(const std::vector<uint32>& ids) {
    talk_base::Message msg;
    while (talk_base::Thread::Current()->Get(&msg)) {
        if (msg.phandler == NULL) {
            if (std::find(ids.begin(), ids.end(), msg.message_id) != ids.end()){
                return msg.message_id;
            }
            std::cout << "orphaned message: " << msg.message_id;
            continue;
        }
        talk_base::Thread::Current()->Dispatch(&msg);
    }
    return 0;
}

void P2PThread::Run()
{
    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);
//#ifdef WIN32
//    talk_base::EnsureWinsockInit();
//        talk_base::Win32Thread w32_thread;
//        talk_base::ThreadManager::Instance()->SetCurrentThread(&w32_thread);
//#endif

    PeerConnectionClient client;
    this->client_ = &client;
    talk_base::scoped_refptr<kaerp2p::ServerConductor> conductor(
                new talk_base::RefCountedObject<kaerp2p::ServerConductor>(&client));

    this->conductor_ = conductor;
    std::string serverIp = "192.168.40.195";
    conductor->StartLogin(serverIp,8888);
    LOG(INFO)<<"connectting";

    std::vector<uint32> ids;
    ids.push_back(kaerp2p::MSG_DONE);
    ids.push_back(kaerp2p::MSG_LOGIN_FAILED);
    //Loop(ids);
    talk_base::Message msg;
    while (talk_base::Thread::Current()->Get(&msg)) {
        if (msg.phandler == NULL) {
            if (std::find(ids.begin(), ids.end(), msg.message_id) != ids.end()){
                //return msg.message_id;
                break;
            }
            std::cout << "orphaned message: " << msg.message_id;
            //continue;
        }
        talk_base::Thread::Current()->Dispatch(&msg);
    }

    conductor->DisconnectFromServer();
    std::cout << "thread end"<<std::endl;
}

void P2PThread::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_CONNECT_TO_PEER:
    {
        talk_base::TypedMessageData<int> *msgData =
                static_cast< talk_base::TypedMessageData<int> *>(msg->pdata);

        int peer_id = msgData->data();
        conductor_->ConnectToPeer(peer_id);
    }
        break;
    case MSG_SEND_STRING:
    {
        talk_base::TypedMessageData<std::string> *msgData =
                static_cast< talk_base::TypedMessageData<std::string> *>(msg->pdata);

        std::string data = msgData->data();
        StreamProcess * stream = conductor_->GetStreamProcess();
        //stream->WriteData(data.c_str(),data.length());
    }
        break;
    case MSG_WRITE_BUFFER:
    {
        talk_base::scoped_ptr<talk_base::TypedMessageData<talk_base::Buffer> >msgData (
                static_cast< talk_base::TypedMessageData<talk_base::Buffer> *>(msg->pdata));

        StreamProcess * stream = conductor_->GetStreamProcess();
        //stream->WriteBuffer(msgData->data());
    }
        break;
    case MSG_SEND_TO_PEER:
    {
        talk_base::scoped_ptr<talk_base::TypedMessageData<std::string> >msgData (
                static_cast< talk_base::TypedMessageData<std::string> *>(msg->pdata));
         conductor_->UIThreadCallback(ServerConductor::SEND_MESSAGE_TO_PEER,&msgData->data());
        break;
    }
    case MSG_SEND_STREAM:
    {
        talk_base::TypedMessageData<int> *msgData =
                static_cast< talk_base::TypedMessageData<int> *>(msg->pdata);
        int buffer_len = msgData->data();
        char * buffer = new char[buffer_len];
        for(int i=0;i<buffer_len;i++){
            buffer[i] = static_cast<char>(rand());
        }
        talk_base::Buffer sendBuffer(buffer,buffer_len);
        StreamProcess * stream = conductor_->GetStreamProcess();

       // stream->WriteBuffer(sendBuffer);
        this->PostDelayed(10,this,MSG_SEND_STREAM,msgData);
    }
        break;
    default:
        break;
    }
}

}
