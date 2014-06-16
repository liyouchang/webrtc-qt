#include <iostream>


#include "talk/base/thread.h"
#include "talk/base/logging.h"
#include "talk/base/json.h"

#include "CameraClient.h"
#include "libjingle_app/jsonconfig.h"
#include "libjingle_app/defaults.h"
#include "libjingle_app/p2pconductor.h"

#ifndef ARM
#include "KeVideoSimulator.h"
#else
#include "HisiMediaDevice.h"

#endif//arm


std::string ReadConfigFile();
const char * kVersion = "V0.31";

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

    LOG(INFO)<<"json config : "<<JsonConfig::Instance()->ToString();
    LOG(INFO)<<"zmqclient current version is "<<kVersion;

    std::string serversStr  = JsonValueToString(jservers);
    kaerp2p::P2PConductor::AddIceServers(serversStr);

    std::string strDealerId;
    std::string strMac;
    GetStringFromJson(mac_value,&strMac);
    GetStringFromJson(dealer_value,&strDealerId);

#ifndef ARM
    CameraClient client(strMac);
    client.Connect(router_value.asString(),strDealerId);
    //client.Login();
    Json::Value jsampleFile =
            JsonConfig::Instance()->Get("sampleFileName","sample.avi");
    std::string sampleFileName;
    if(!GetStringFromJson(jsampleFile,&sampleFileName)){
        return 2;
    }
    KeVideoSimulator * simulator = new KeVideoSimulator(sampleFileName);
    if(!simulator->Init(&client)){
        return 1;
    }
#else
    HisiMediaDevice * device = new HisiMediaDevice();
    if(strMac.empty()) {
        strMac = device->GetHardwareId();
    }

    CameraClient client(strMac);
    client.Connect(router_value.asString(),strDealerId);
    //client.Login();
    device->SignalNetStatusChange.connect(&client,&CameraClient::Reconnect);

    AlarmNotify::Instance()->SignalTerminalAlarm.connect(
                &client,&CameraClient::SendAlarm);

    device->Init(&client);

#endif //arm
    talk_base::Thread::Current()->Run();
    return 0;
}

