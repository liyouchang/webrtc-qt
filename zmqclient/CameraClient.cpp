#include "CameraClient.h"

#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"

#include "libjingle_app/defaults.h"

const int kHeartInterval = 60000;//ms
//const int kHeartInterval = 3000;//ms
const int kHeartReconnect = 1;//1次没有收到心跳则重连
CameraClient::CameraClient(std::string mac,std::string ver):
    mac_(mac),messageServer("Backstage"),alarmServer("Alarmstage"),
    heartCount(0),clientVersion(ver),oldNetType(-1),oldIp(-1)
{
    comm_thread_ = talk_base::Thread::Current();
}

void CameraClient::Login()
{
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = "Terminal_Login";
    jmessage["MAC"] = mac_;
    jmessage["Version"] = clientVersion;
    std::string msg = writer.write(jmessage);
    this->SendToPeer(messageServer,msg);
}

void CameraClient::SendAlarm(int alarmType, const std::string &alarmInfo,
                             const std::string &picture)
{
    Json::StyledWriter writer;
    Json::Value jmessage;

    jmessage["type"] = "Terminal_Alarm";
    jmessage["MAC"] = mac_;
    jmessage["AlarmType"] = alarmType;
    jmessage["AlarmInfo"] = alarmInfo;
    jmessage["Picture"] = picture;
    jmessage["DateTime"] = kaerp2p::GetCurrentDatetime("%F %T");

    std::string msg = writer.write(jmessage);
    this->SendToPeer(alarmServer,msg);
}

bool CameraClient::Connect(const std::string &router, const std::string &id)
{
    std::string strDealerId = id;
    if(strDealerId.empty()) {
        strDealerId = mac_ + "-" + kaerp2p::GetRandomString();
    }
    if(PeerConnectionClientDealer::Connect(router,strDealerId)){
        comm_thread_->Post(this,MSG_LOGIN_HEART);
        return true;
    }
    return false;

}

void CameraClient::Reconnect()
{
    std::string strDealerId = mac_ + "-" + kaerp2p::GetRandomString();
    std::string oldAddr = dealer_->addr();
    LOG(INFO)<<"CameraClient::Reconnect---with id "<<strDealerId<<" to addr "<<
               oldAddr;

//    this->Connect(oldAddr,"");

    dealer_->disconnect();
    dealer_->connect(strDealerId,oldAddr);
    this->Login();
}



void CameraClient::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_LOGIN_HEART: {
        if(++heartCount > kHeartReconnect) {
            LOG(INFO)<<"heart count is "<<heartCount << " reconnect";
            this->Reconnect();
            heartCount = 0;
        }
        this->Login();
        comm_thread_->PostDelayed(kHeartInterval,this,MSG_LOGIN_HEART);
        break;
    }
    case MSG_RECEIVE_HEART:{
        //LOG(INFO)<<"receive heart at count "<<heartCount;
        heartCount = 0;
        break;
    }
    case MSG_RECONNECT:{
        break;
    }
    default:
        break;
    }
}

void CameraClient::OnMessageFromPeer(const std::string &peer_id,
                                     const std::string &message)
{
    if(peer_id.compare("Backstage") == 0)
    {
        Json::Reader reader;
        Json::Value jmessage;
        if (!reader.parse(message, jmessage)) {
            LOG(WARNING) << "Received unknown message. " << message;
            return;
        }
        std::string type;
        GetStringFromJsonObject(jmessage, "type", &type);
        if(type.compare("Terminal_Login") == 0)
        {
            int result;
            GetIntFromJsonObject(jmessage, "Result", &result);
            if(result == 0){

            }
            comm_thread_->Post(this,MSG_RECEIVE_HEART);
        }
        else if(type.compare("Terminal_NTP") == 0)
        {
            LOG_F(INFO)<<"get ntp msg "<< message;
            std::string ntpIp;
            std::string timezone;
            int port;
            GetStringFromJsonObject(jmessage,"IP",&ntpIp);
            GetIntFromJsonObject(jmessage,"port",&port);
            GetStringFromJsonObject(jmessage,"TimeZone",&timezone);
            SignalNtpSet(ntpIp,port,timezone);
        }else{
            LOG(WARNING)<<"no support backstage message";
        }
    }else{
        SignalMessageFromPeer(peer_id,message);
    }
}
