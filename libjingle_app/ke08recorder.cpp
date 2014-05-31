#include "ke08recorder.h"

#include "talk/base/pathutils.h"
#include "talk/base/bind.h"
#include "talk/base/logging.h"
#include "talk/base/stream.h"
#include "talk/base/fileutils.h"

#include "libjingle_app/KeMessage.h"


namespace  kaerp2p {

Ke08RecordReader::Ke08RecordReader()
{
    media_thread_ = new talk_base::Thread();
    media_thread_->Start();
}

Ke08RecordReader::~Ke08RecordReader()
{
    delete media_thread_;
}

void Ke08RecordReader::OnMessage(talk_base::Message *msg)
{
    static int fileBufPos = 0;
    static int lastFrameNo = 0;
    if(fileBufPos > (video_data_.length() - sizeof(KEFrameHead))){
        fileBufPos = 0;
    }
    KEFrameHead * pHead = (KEFrameHead *)(video_data_.data() + fileBufPos);
    int frameLen = pHead->frameLen + sizeof(KEFrameHead);
    if( frameLen < 0 || fileBufPos + frameLen > video_data_.length() ){
        fileBufPos = 0;
        media_thread_->Post(this);
        return;
    }
    //send media
    this->SendMediaMsg(video_data_.data() + fileBufPos,frameLen);
    fileBufPos += frameLen;
    if(lastFrameNo == 0){
        lastFrameNo = pHead->frameNo;
    }
    if(lastFrameNo != pHead->frameNo){
        lastFrameNo = pHead->frameNo;
        media_thread_->PostDelayed(40,this);
    }
    else{
        media_thread_->Post(this);
    }
}

bool Ke08RecordReader::StartRead(const std::string &filename)
{
    talk_base::Pathname path;
    bool result = talk_base::Filesystem::GetAppPathname(&path);
    if(!result){
        return false;
    }
    path.AppendPathname(filename);
    LOG(INFO)<<"file name is "<<path.pathname();
    talk_base::scoped_ptr<talk_base::FileStream> stream;
    stream.reset( talk_base::Filesystem::OpenFile(path,"r")) ;
    if(stream == NULL){
        LOG(WARNING) << "open video file error";
        return false;
    }
    size_t fileSize;
    result = stream->GetSize(&fileSize);
    if(!result){
        return false;
    }
    video_data_.SetLength(fileSize);
    size_t readData;
    int error;
    talk_base::StreamResult sr = stream->Read(video_data_.data(),fileSize,&readData,&error);
    if(sr != talk_base::SR_SUCCESS){
        return false;
    }
    //start read file cycle
    media_thread_->Post(this);
    return true;
}

bool Ke08RecordReader::StopRead()
{
    return false;
}

void Ke08RecordReader::SendMediaMsg(const char *data, int len)
{

}


Ke08RecordSaver::Ke08RecordSaver()
{
    saveFile = new talk_base::FileStream();
}

Ke08RecordSaver::~Ke08RecordSaver()
{
    delete saveFile;
}

bool Ke08RecordSaver::StartSave(const std::string &fileName)
{
    LOG(INFO)<<"Ke08RecordSaver::StartSave---"<<fileName;
    if(!saveFile->Open(fileName,"wb",NULL)){
        LOG(WARNING)<<"Ke08RecordSaver::StartSave---"<<
                      "open file error "<<fileName;
        return false;
    }
    return true;
}

bool Ke08RecordSaver::StopSave()
{
    saveFile->Close();
    return true;
}

void Ke08RecordSaver::OnVideoData(const std::string &peerId, const char *data, int len)
{
    size_t written;
    talk_base::StreamResult result = saveFile->WriteAll(
                data,len,&written,NULL);
    if(result != talk_base::SR_SUCCESS){
        LOG(WARNING)<<"Ke08RecordSaver::OnVideoData---"<<"write file error ";
    }
    this->savedSize += written;
}

void Ke08RecordSaver::OnAudioData(const std::string &peerId, const char *data, int len)
{
    size_t written;
    talk_base::StreamResult result = saveFile->WriteAll(
                data,len,&written,NULL);
    if(result != talk_base::SR_SUCCESS){
        LOG(WARNING)<<"Ke08RecordSaver::OnAudioData---"<<"write file error ";
    }
}

}
