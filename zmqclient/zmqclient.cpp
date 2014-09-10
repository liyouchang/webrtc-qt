#include <iostream>


#include "talk/base/thread.h"
#include "talk/base/logging.h"
#include "talk/base/json.h"

#include "CameraClient.h"
#include "libjingle_app/jsonconfig.h"
#include "libjingle_app/defaults.h"
#include "libjingle_app/p2pconductor.h"
#include "libjingle_app/peerterminal.h"

#ifndef ARM
#else
#include "kesdkdevice.h"
//#include "HisiMediaDevice.h"
#endif//arm


int kVersion = 60;


int main()
{
    //read config
    JsonConfig::Instance()->FromFile(kaerp2p::GetAppFilePath("config.json"));
    Json::Value mac_value = JsonConfig::Instance()->Get("camera.mac","");
    Json::Value dealer_value = JsonConfig::Instance()->Get("dealerId","");
    Json::Value router_value =
            JsonConfig::Instance()->Get("routerUrl","tcp://192.168.40.191:5555");
    Json::Value logParamsValue =
            JsonConfig::Instance()->Get("logParams","tstamp thread info debug");
    Json::Value jservers = JsonConfig::Instance()->Get("servers","");
    //Json::Value jclarity = JsonConfig::Instance()->Get("clarity",2);
    Json::Value jlogsaveFile = JsonConfig::Instance()->Get("logSaveFile","");

    talk_base::LogMessage::ConfigureLogging(logParamsValue.asString().c_str(),
                                            jlogsaveFile.asString().c_str());

    //Json::Value jntp = JsonConfig::Instance()->Get("ntpconfig","");
    LOG(INFO)<<"json config : "<<JsonConfig::Instance()->ToString();
    std::string clientVer = kaerp2p::ToStringVersion(kVersion);
    LOG(INFO)<<"zmqclient current version is "<<clientVer;

    std::string serversStr = JsonValueToString(jservers);
    kaerp2p::P2PConductor::AddIceServers(serversStr);

    std::string strDealerId;
    std::string strMac;
    GetStringFromJson(mac_value,&strMac);
    GetStringFromJson(dealer_value,&strDealerId);

    bool isSetNet;
    GetBoolFromJson(JsonConfig::Instance()->Get("setNet",false),&isSetNet);


#ifndef ARM

#else
    KeSdkDevice * device = new KeSdkDevice();
    //HisiMediaDevice * device = new HisiMediaDevice();

    if (isSetNet) {
        device->SetNetInfo();
    }

    if (strMac.empty()) {
        strMac = device->GetMacAddress();
    }


    CameraClient client(strMac,clientVer);
    client.Connect(router_value.asString(),strDealerId);
    client.Login();

    client.SignalNtpSet.connect(device,&KeSdkDevice::SetNtp);
    device->SignalNetStatusChange.connect(&client,&CameraClient::Reconnect);

//    AlarmNotify::Instance()->SignalTerminalAlarm.connect(
//                &client,&CameraClient::SendAlarm);

    kaerp2p::PeerTerminal * terminal = new kaerp2p::PeerTerminal(&client);
//    kaerp2p::LocalUdpTerminal * terminal = new kaerp2p::LocalUdpTerminal();
//    terminal->Initialize("0.0.0.0:12345");

    device->Init(terminal);

    talk_base::Thread::Current()->Run();
    delete device;
    delete terminal;

#endif //arm
    return 0;
}

