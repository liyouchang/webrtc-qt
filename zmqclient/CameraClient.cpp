#include "CameraClient.h"
#include "talk/base/json.h"
#include "talk/base/logging.h"
CameraClient::CameraClient()
{
    comm_thread_ = talk_base::Thread::Current();
}

void CameraClient::Login()
{
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = "Terminal_Login";
    jmessage["MAC"] = "54321";
    jmessage["DealerName"] = "123456";
    std::string msg = writer.write(jmessage);

    this->SendToPeer("Backstage",msg);
    comm_thread_->PostDelayed(60000,this,MSG_LOGIN_TIMEOUT);
}

void CameraClient::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_LOGIN_TIMEOUT:{
        //LOG(INFO)<<"Log in timeout,relogin";
        //this->Login();
        break;
    }
    default:
        break;
    }
}

void CameraClient::OnMessageFromPeer(const std::string &peer_id, const std::string &message)
{
    if(peer_id.compare("Backstage") == 0){
        Json::Reader reader;
        Json::Value jmessage;
        if (!reader.parse(message, jmessage)) {
            LOG(WARNING) << "Received unknown message. " << message;
            return;
        }
        std::string type;
        GetStringFromJsonObject(jmessage, "type", &type);
        if(type.compare("Terminal_Login") == 0){
            int result;
            GetIntFromJsonObject(jmessage, "Result", &result);
            if(result == 0){
                LOG(INFO) <<"Login success";
                comm_thread_->Clear(this,MSG_LOGIN_TIMEOUT);
            }
        }
    }
    else{
        SignalMessageFromPeer(peer_id,message);
    }
}
