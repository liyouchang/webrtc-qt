#include <iostream>


#include "talk/base/thread.h"
#include "talk/base/logging.h"
#include "talk/base/json.h"
#include "jsonconfig.h"
#include "CameraClient.h"

#include "libjingle_app/defaults.h"

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

std::string ReadConfigFile();

int main()
{


    //read config
    JsonConfig::Instance()->FromFile(GetAppFilePath("config.json"));


    Json::Value mac_value = JsonConfig::Instance()->Get("camera.mac","");
    Json::Value dealer_value = JsonConfig::Instance()->Get("dealer_id","");
    Json::Value router_value = JsonConfig::Instance()->Get("router_url","");
    Json::Value log_params_value = JsonConfig::Instance()->Get("log_params","");

    talk_base::LogMessage::ConfigureLogging(log_params_value.asString().c_str(),NULL);

    LOG(INFO)<<"json config : "<<JsonConfig::Instance()->ToString();


    CameraClient client(mac_value.asString());

#ifndef ARM
    client.Connect(router_value.asString(),dealer_value.asString());
    client.Login();
    KeVideoSimulator * simulator = new KeVideoSimulator();
    simulator->Init(&client);
    simulator->ReadVideoData("video.h264");
#else
    client.Connect("tcp://192.168.40.191:5555","1234567");
    client.Login();
    HisiMediaDevice * device = new HisiMediaDevice();
    device->Init(&client);
    //device->SetVideoResolution("704,576");
#endif //arm

    talk_base::Thread::Current()->Run();

    return 0;
}

