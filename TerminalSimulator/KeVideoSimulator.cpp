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

    zmqContext = new zmq::context_t(1,2);
    publisher = new zmq::socket_t(*zmqContext,ZMQ_PUB);
    publisher->bind("tcp://*:5556");

    repSocket = new zmq::socket_t(*zmqContext,ZMQ_REP);
    repSocket->bind("tcp://*:22555");
    //        int reconnectInterval = 10000;
    //        repSocket->setsockopt(ZMQ_RECONNECT_IVL,&reconnectInterval,sizeof(reconnectInterval));
    int highwater = 100;
    repSocket->setsockopt(ZMQ_SNDHWM,&highwater,sizeof(highwater));
    repSocket->setsockopt(ZMQ_RCVHWM,&highwater,sizeof(highwater));

    publisher->setsockopt(ZMQ_SNDHWM,&highwater,sizeof(highwater));
    publisher->setsockopt(ZMQ_RCVHWM,&highwater,sizeof(highwater));


    int ligger = 100;
    repSocket->setsockopt(ZMQ_LINGER,&ligger,sizeof(ligger));

    publisher->setsockopt(ZMQ_LINGER,&ligger,sizeof(ligger));


    zmqThread = new talk_base::Thread();
    zmqThread->Start();

    zmqThread->Post(this,MSG_ZMQ_RECV);

}

KeVideoSimulator::~KeVideoSimulator()
{
    delete reader;
    delete publisher;
    delete zmqContext;
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

void KeVideoSimulator::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_ZMQ_RECV:{
        LOG_F(INFO) << " start recv rep";
        zmq::message_t zmsg;
        repSocket->recv(&zmsg);
        std::string strMsg((char *)zmsg.data(),zmsg.size());
        Json::Reader reader;
        LOG_F(INFO)<<"rep msg "<<strMsg;
        Json::Value jmessage;
        if (!reader.parse(strMsg, jmessage)) {
            LOG(WARNING) << "Received unknown message. ";
            strMsg = "unknown message";
            repSocket->send(strMsg.c_str(),strMsg.length());
        }else{
            Json::Value jresult;

            this->OnCommandJsonMsg(jmessage,&jresult);

            if(!jresult.isNull()){
                Json::StyledWriter writer;
                strMsg = writer.write(jresult);
                LOG(LS_VERBOSE)<<"send msg is "<< msg;
                repSocket->send(strMsg.c_str(),strMsg.length());
            }
        }
        zmqThread->Post(this,MSG_ZMQ_RECV);
    }
        break;
    default:
        break;
    }

}


void KeVideoSimulator::OnCommandJsonMsg(const  Json::Value &jmessage,Json::Value *jresult)
{
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage,kaerp2p::kKaerMsgCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error-"<<command ;
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
            LOG(WARNING)<<"get query_record value error from" ;
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
        (*jresult) = jmessage;
        (*jresult)["result"] = true;
        (*jresult)["totalNum"] = totalNum;
        (*jresult)["recordList"] = jrecordList;
    }
    else if(command.compare("wifi_info") == 0){
        (*jresult)["type"] = "tunnel";
        (*jresult)["command"] = "wifi_info";
    }
    else if(command.compare("set_wifi") == 0){
        Json::Value jwifiParam;
        if(!GetValueFromJsonObject(jmessage, "param", &jwifiParam)){
            LOG(WARNING)<<"get set_wifi value error from";
            return;
        }
        (*jresult) = this->GetResultMsg(command,true);
    }
    else if(command.compare("rename") == 0){
        std::string name;
        if(!GetStringFromJsonObject(jmessage,"name", &name)){
            LOG_F(WARNING) <<" receive rename msg error ";
            return;
        }
        (*jresult) = this->GetResultMsg(command,true);
    }else  if(command.compare("arming_status") == 0){
        (*jresult) = jmessage;
        (*jresult)["value"] = 1;
    }
    else{
        kaerp2p::KeTunnelCamera::OnCommandJsonMsg(jmessage,jresult);
    }

}
