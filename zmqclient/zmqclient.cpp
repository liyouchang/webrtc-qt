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

int main()
{
    //read config
    JsonConfig::Instance()->FromFile(GetAppFilePath("config.json"));



    Json::Value mac_value = JsonConfig::Instance()->Get("camera.mac","");
    Json::Value dealer_value = JsonConfig::Instance()->Get("dealerId","");
    Json::Value router_value = JsonConfig::Instance()->Get("routerUrl","tcp://192.168.40.191:5555");
    Json::Value log_params_value = JsonConfig::Instance()->Get("logParams","tstamp thread info debug");
    Json::Value jservers = JsonConfig::Instance()->Get("servers","");
    Json::Value jclarity = JsonConfig::Instance()->Get("clarity",2);

    talk_base::LogMessage::ConfigureLogging(log_params_value.asString().c_str(),NULL);
    LOG(INFO)<<"json config : "<<JsonConfig::Instance()->ToString();

    kaerp2p::P2PConductor::AddIceServers(jservers.asString());

    CameraClient client(mac_value.asString());
    client.Connect(router_value.asString(),dealer_value.asString());
    client.Login();

#ifndef ARM
    KeVideoSimulator * simulator = new KeVideoSimulator();
    simulator->Init(&client);
    simulator->ReadVideoData("video.h264");
#else

    HisiMediaDevice * device = new HisiMediaDevice();
    int clarity = jclarity.asInt();

    device->Init(&client);
    //device->SetVideoClarity(clarity);
    //device->SetVideoResolution("704,576");
#endif //arm

    talk_base::Thread::Current()->Run();

    return 0;
}

