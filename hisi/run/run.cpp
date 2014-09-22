#include <iostream>

#include "keapi/common_api.h"
#include "keapi/media_api.h"
#include "keapi/store_api.h"
#include "keapi/web_api.h"
#include "keapi/alarm_api.h"
#include "keapi/platform_api.h"


#include "zmq.h"
#include "zmq.hpp"
#include "zhelpers.hpp"
#include "zmsg.hpp"

#include "talk/base/json.h"
#include "webrtc/system_wrappers/interface/logging.h"

using namespace std;

zmq::context_t *run_context;


void OnCommandJsonMsg(const Json::Value &jmessage,Json::Value * jresult)
{
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage,"command", &command);
    if(!ret){
        LOG(LS_WARNING)<<"get command error-"<<command<<" from" ;
        return;
    }
    if(command.compare("ptz") == 0){
        std::string ptz_control;
        GetStringFromJsonObject(jmessage,"control",&ptz_control);
        int param;
        GetIntFromJsonObject(jmessage,"param",&param);
//        this->SetPtz(ptz_control,param);
    }
    else if(command.compare("query_record") == 0){
        Json::Value jcondition;
        if(!GetValueFromJsonObject(jmessage, "condition", &jcondition))
        {
            LOG(LS_WARNING)<<"get query_record value error from" ;
            return;
        }
        int totalNum = 0;
        Json::Value jrecordList(Json::arrayValue);
//        bool result =  QueryRecord(jcondition,&jrecordList,&totalNum);

        *jresult = jmessage;
//        (*jresult)["result"] = result;
        (*jresult)["totalNum"] = totalNum;
        (*jresult)["recordList"] = jrecordList;
        //        this->ReportJsonMsg(peerId,jmessage);
    }
    else if( command.compare("wifi_info") == 0 ){
//        Json::Value jwifi = GetWifiJsonArray();
        (*jresult)["type"] = "tunnel";
        (*jresult)["command"] = "wifi_info";
//        (*jresult)["wifis"] = jwifi;
        //        this->ReportJsonMsg(peerId,jresult);
    }
    else if( command.compare("set_wifi") == 0 ){
        Json::Value jwifiParam;
        if(!GetValueFromJsonObject(jmessage, "param", &jwifiParam)){
            LOG(LS_WARNING)<<"get set_wifi value error from";
            return;
        }
//        bool ret = this->SetWifiInfo(jwifiParam);
//        (*jresult) = this->GetResultMsg(command,ret);
    }
    else if( command.compare("wifi_status") == 0 ) {
        int wifiStatus = Net_Get_WifiLink_Status();
        (*jresult) = jmessage;
        (*jresult)["status"] = wifiStatus;
    }
    else if( command.compare("rename") == 0 ) {
        std::string name;
        if(!GetStringFromJsonObject(jmessage,"name", &name)){
            LOG_F(LS_WARNING) <<" receive rename msg error ";
            return;
        }
//        bool r  = SetOsdTitle(name);
//        LOG(LS_INFO)<<"receive rename message ,set device title - "<<name<<
//                   " ;result "<<r;
//        (*jresult) = this->GetResultMsg(command,ret);
    }
    else if( command.compare("arming_status") == 0 ){
//        int status;
//        if( GetIntFromJsonObject(jmessage,"value",&status) &&
//                status != kCommandGetValue ) {//set value
//            SetArmingStatus(status);
//        }
//        (*jresult) = jmessage;
//        (*jresult)["value"] = this->GetArmingStatus();
    }
    else if( command.compare("reboot") == 0 ) {
//        *jresult = this->GetResultMsg(command,ret);
//        //        this->QuitMainThread();
//        SYSTEM_Set_Reboot();
        throw 0;
    }
    else{
        LOG_F(LS_WARNING) <<" not support message";
    }
}



int main()
{
    int clock_handle;
    int ntp_chandle;
    int gpio_handle;

    CONFIG_Initialize();
    gpio_handle = GPIO_Open("/dev/hi_gpio");
    clock_handle = CLOCK_Open(CLOCK_TYPE_HIRTC);

    SYSTEM_Initialize();
    NET_Initialize();
    ntp_chandle = CLOCK_Open(CLOCK_TYPE_NTP);
    MEDIA_Initialize();

    STORE_Initialize();
    ALARM_Initialize();
    WEB_Initialize();

    PLATFORM_Tools_Open();
    PLATFORM_Update_Open();


    try{
        //        context_ = new zmq::context_t(1,5);
        run_context = new zmq::context_t(1,1);


        zmq::socket_t *repSocket = new zmq::socket_t(*run_context,ZMQ_REP);
        repSocket->bind("tcp://*:22555");
        //        int reconnectInterval = 10000;
        //        repSocket->setsockopt(ZMQ_RECONNECT_IVL,&reconnectInterval,sizeof(reconnectInterval));
        int highwater = 100;
        repSocket->setsockopt(ZMQ_SNDHWM,&highwater,sizeof(highwater));
        repSocket->setsockopt(ZMQ_RCVHWM,&highwater,sizeof(highwater));
        int ligger = 100;
        repSocket->setsockopt(ZMQ_LINGER,&ligger,sizeof(ligger));

        while(true){
            zmq::message_t zmsg;
            repSocket->recv(&zmsg);

            std::string strMsg((char *)zmsg.data(),zmsg.size());
            Json::Reader reader;
            LOG_F(LS_INFO)<<"rep msg "<<strMsg;
            Json::Value jmessage;
            if (!reader.parse(strMsg, jmessage)) {
                LOG(LS_WARNING) << "Received unknown message. ";
                break;
            }
            Json::Value jresult;
            OnCommandJsonMsg(jmessage,&jresult);
            Json::StyledWriter writer;
            strMsg = writer.write(jresult);
            LOG(LS_VERBOSE) << "send msg is " << strMsg;
            repSocket->send(strMsg.c_str(),strMsg.length());


        }

    }catch(zmq::error_t e){
        LOG_F(LS_WARNING) <<" failed , enum:"<<e.num()<<" edes:" <<e.what();
    }catch(int ){
        LOG_F(LS_WARNING) <<" catch reboot";
    }


    PLATFORM_Update_Close();

    PLATFORM_Tools_Close();
    WEB_Cleanup();

    ALARM_Cleanup();
    STORE_Cleanup();

    MEDIA_Cleanup();
    CLOCK_Close(ntp_chandle);
    NET_Cleanup();

    SYSTEM_Cleanup();

    CLOCK_Close(clock_handle);

    GPIO_Close(gpio_handle);
    CONFIG_Cleanup();

    cout << "Hello World!" << endl;
    return 0;
}

