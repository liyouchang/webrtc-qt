#include "CameraClient.h"
#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"

#include "libjingle_app/defaults.h"

const int kHeartInterval = 60000;//ms
CameraClient::CameraClient(std::string mac):
    mac_(mac),messageServer("Backstage"),alarmServer("Alarmstage")
{
    comm_thread_ = talk_base::Thread::Current();
}

void CameraClient::Login()
{
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = "Terminal_Login";
    jmessage["MAC"] = mac_;
    std::string msg = writer.write(jmessage);

    this->SendToPeer(messageServer,msg);
    comm_thread_->PostDelayed(kHeartInterval,this,MSG_LOGIN_TIMEOUT);
}

void CameraClient::SendAlarm(int alarmType, std::string alarmInfo,
                             std::string picture)
{
    Json::StyledWriter writer;
    Json::Value jmessage;

    jmessage["type"] = "Terminal_Alarm";
    jmessage["MAC"] = mac_;
    jmessage["AlarmType"] = alarmType;
    jmessage["AlarmInfo"] = alarmInfo;
    jmessage["Picture"] = picture;
    jmessage["DateTime"] = GetCurrentDatetime("%F %T");

    std::string msg = writer.write(jmessage);
    this->SendToPeer(alarmServer,msg);
}

void CameraClient::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_LOGIN_TIMEOUT:{
        //LOG(INFO)<<"Log in timeout,relogin";
        this->Login();
        break;
    }
    default:
        break;
    }
}

void CameraClient::OnMessageFromPeer(const std::string &peer_id,
                                     const std::string &message)
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
    }else{
        SignalMessageFromPeer(peer_id,message);
    }
}
