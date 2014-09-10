#include "KeVideoSimulator.h"

#include "talk/base/pathutils.h"
#include "talk/base/bind.h"
#include "talk/base/logging.h"
#include "talk/base/stream.h"
#include "talk/base/thread.h"
#include "talk/base/fileutils.h"
#include "talk/base/buffer.h"
#include "talk/base/json.h"

#include "libjingle_app/defaults.h"
#include "libjingle_app/KeMessage.h"

KeVideoSimulator::KeVideoSimulator(const std::string &fileName):
    fileName(fileName)
{
    reader = new kaerp2p::RecordReaderAvi(20);
    //reader = new kaerp2p::FakeRecordReaderAvi(10);
    reader->SignalVideoData.connect(this,&KeVideoSimulator::OnFileVideoData);
    reader->SignalAudioData.connect(this,&KeVideoSimulator::OnFileAudioData);
    reader->SignalRecordEnd.connect(this,&KeVideoSimulator::OnFileReadEnd);

    context = new zmq::context_t();
    publisher = new zmq::socket_t(*context,ZMQ_PUB);
    publisher->bind("tcp://*:5556");
}

KeVideoSimulator::~KeVideoSimulator()
{
    delete reader;
    delete publisher;
    delete context;
}

bool KeVideoSimulator::Init(kaerp2p::PeerTerminalInterface *t)
{
    if(!KeTunnelCamera::Init(t)){
        return  false;
    }
    bool ret = reader->StartRead(fileName);
    return ret;
//    return true;
}

void KeVideoSimulator::GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info)
{
    info->frameRate = 15;
    info->frameResolution = kaerp2p::kFrame720P;
}

//restart again
void KeVideoSimulator::OnFileReadEnd(kaerp2p::RecordReaderInterface *)
{
    LOG_F(INFO)<<"read end restart";
    reader->StopRead();
    reader->StartRead(fileName);
}

void KeVideoSimulator::OnFileVideoData(const char *data, int len)
{
    this->SignalVideoData1(data,len);
    this->SignalVideoData2(data,len);
    this->SignalVideoData3(data,len);
    publisher->send(data,len);
}

void KeVideoSimulator::OnFileAudioData(const char *data, int len)
{
    this->SignalAudioData(data,len);
//    publisher->send(data,len);
}


void KeVideoSimulator::OnCommandJsonMsg(const std::string &peerId, Json::Value &jmessage)
{
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage,kaerp2p::kKaerMsgCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error-"<<command<<" from"<<peerId ;
        return;
    }
//    LOG_F(INFO)<<"receive command "<<command;
    if(command.compare("ptz") == 0){
        std::string ptz_control;
        int param;
        GetStringFromJsonObject(jmessage,"control",&ptz_control);
        GetIntFromJsonObject(jmessage,"param",&param);
    }
    else if(command.compare("query_record") == 0){
        Json::Value jcondition;
        if(!GetValueFromJsonObject(jmessage, "condition", &jcondition))
        {
            LOG(WARNING)<<"get query_record value error from" << peerId ;
            return;
        }
        int totalNum = 10;
        Json::Value jrecordList(Json::arrayValue);
        for (int i = 0 ; i < 10 ; i++) {
            Json::Value jrecord;
            char timebuf[64];
            jrecord["fileName"] = "Sample.avi";
            sprintf(timebuf,"2014/9/11 %02d:00:00",i);

            jrecord["fileEndTime"] = timebuf;
            jrecord["fileSize"] = 256;
            jrecordList.append(jrecord);
        }
        jmessage["result"] = true;
        jmessage["totalNum"] = totalNum;
        jmessage["recordList"] = jrecordList;
        this->ReportJsonMsg(peerId,jmessage);
    }
    else if(command.compare("wifi_info") == 0){
        Json::Value jresult;
        jresult["type"] = "tunnel";
        jresult["command"] = "wifi_info";

        this->ReportJsonMsg(peerId,jresult);
    }
    else if(command.compare("set_wifi") == 0){
        Json::Value jwifiParam;
        if(!GetValueFromJsonObject(jmessage, "param", &jwifiParam)){
            LOG(WARNING)<<"get set_wifi value error from"<<peerId;
            return;
        }
        this->ReportResult(peerId,command,true);
    }
    else if(command.compare("rename") == 0){
        std::string name;
        if(!GetStringFromJsonObject(jmessage,"name", &name)){
            LOG_F(WARNING) <<" receive rename msg error ";
            return;
        }
        this->ReportResult(peerId,command,true);
    }else  if(command.compare("arming_status") == 0){
        jmessage["value"] = 1;
        this->ReportJsonMsg(peerId,jmessage);
    }
    else{
        kaerp2p::KeTunnelCamera::OnCommandJsonMsg(peerId,jmessage);
    }

}
