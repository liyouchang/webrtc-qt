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
}

KeVideoSimulator::~KeVideoSimulator()
{
    delete reader;
}

bool KeVideoSimulator::Init(kaerp2p::PeerTerminalInterface *t)
{
    if(!KeTunnelCamera::Init(t)){
        return  false;
    }
    return reader->StartRead(fileName);
}

void KeVideoSimulator::GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info)
{
    info->frameRate = 15;
    info->frameResolution = kaerp2p::kFrame720P;
}

//restart again
void KeVideoSimulator::OnFileReadEnd(kaerp2p::RecordReaderInterface *)
{
    reader->StopRead();
    reader->StartRead(fileName);
}

void KeVideoSimulator::OnFileVideoData(const char *data, int len)
{
    this->SignalVideoData1(data,len);
    this->SignalVideoData2(data,len);
    this->SignalVideoData3(data,len);
}

void KeVideoSimulator::OnFileAudioData(const char *data, int len)
{
    this->SignalAudioData(data,len);
}

void KeVideoSimulator::OnRecvRecordQuery(std::string peer_id, std::string condition)
{
    LOG(INFO)<<"KeVideoSimulator::OnRecvRecordQuery ---"<<
               peer_id<<" query:"<<condition;
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = "tunnel";
    jmessage["command"] = "queryRecord";
    jmessage["condition"] = condition;
    jmessage["totalNum"] = 10;
    Json::Value jrecord;
    jmessage["recordList"].append(jrecord);
    std::string msg = writer.write(jmessage);
    this->terminal_->SendByRouter(peer_id,msg);
}
