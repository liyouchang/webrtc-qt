#include "recordreader.h"
#include "talk/base/thread.h"
#include "talk/base/stream.h"
#include "talk/base/scoped_ptr.h"

#include "libjingle_app/KeMessage.h"

void RecordReaderInterface::OnMessage(talk_base::Message *msg)
{
    switch(msg->message_id){
    case MSG_RECORD_SEND:{
        talk_base::Buffer * send_buffer = this->ReadRecordFrame();
        if(send_buffer == NULL){
            this->CloseRecord();
            break;
        }
        this->SignalRecordData(send_buffer->data(),send_buffer->length());
        delete send_buffer;
        file_thread_->PostDelayed(frame_internal_,this,MSG_RECORD_SEND);
        break;
    }
    default:
        break;
    }
}

bool RecordReaderInterface::StartPlayRecord(int internal,talk_base::Thread *thread)
{
    ASSERT(internal>0);
    frame_internal_ = internal;
    if(thread == NULL){
        file_thread_ = talk_base::Thread::Current();
    }else{
        file_thread_ = thread;
    }
    bool ret = OpenRecord();
    if(!ret){
        return ret;
    }
    file_thread_->PostDelayed(frame_internal_,this,MSG_RECORD_SEND);
    return ret;
}

RecordReaderInterface::RecordReaderInterface():
    file_thread_(NULL),frame_internal_(0)
{

}


FileRecordReader::FileRecordReader(std::string filename):
    filename_(filename),file_stream_(NULL)
{
}

talk_base::Buffer *FileRecordReader::ReadRecordFrame()
{
    ASSERT(file_stream_);

    talk_base::scoped_ptr<talk_base::Buffer> buffer;
    buffer.reset(new talk_base::Buffer);

//    if(fileBufPos > (video_data_.length() - sizeof(KEFrameHead))){
//        fileBufPos = 0;
//    }
//    KEFrameHead * pHead = (KEFrameHead *)(video_data_.data() + fileBufPos);
//    int frameLen = pHead->frameLen + sizeof(KEFrameHead);
//    if( frameLen < 0 || fileBufPos + frameLen > video_data_.length()  ){
//        fileBufPos = 0;
//        media_thread_->Post(this,MSG_SENDFILEVIDEO);
//        return;
//    }
//    //send media
//    this->SendMediaMsg(video_data_.data() + fileBufPos,frameLen);

//    fileBufPos += frameLen;

//    if(lastFrameNo == 0){
//        lastFrameNo = pHead->frameNo;
//    }

//    if(lastFrameNo != pHead->frameNo){
//        lastFrameNo = pHead->frameNo;
//        media_thread_->PostDelayed(40,this ,MSG_SENDFILEVIDEO);
//    }
//    else{
//        media_thread_->Post(this,MSG_SENDFILEVIDEO);
//    }


    return buffer.release();
}

bool FileRecordReader::OpenRecord()
{
    talk_base::scoped_ptr<talk_base::FileStream> stream;
    stream.reset(new talk_base::FileStream());
    bool ret = stream->Open(filename_,"r+",NULL);
    if(!ret){
        stream.reset();
    }
    file_stream_ = stream.release();
    return ret;


}

void FileRecordReader::CloseRecord()
{
    if(file_stream_){
        file_stream_->Close();
        delete file_stream_;
        file_stream_ = NULL;
    }
}


