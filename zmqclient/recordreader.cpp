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
            break;
        }
        this->SignalRecordData(send_buffer->data(),send_buffer->length());
        delete send_buffer;
        file_thread_->PostDelayed(frameInternal_,this,MSG_RECORD_SEND);
        break;
    }
    default:
        break;
    }
}

bool RecordReaderInterface::StartPlayRecord(int internal,talk_base::Thread *thread)
{
    ASSERT(internal>0);
    frameInternal_ = internal;
    if(thread == NULL){
        file_thread_ = talk_base::Thread::Current();
    }else{
        file_thread_ = thread;
    }
    bool ret = OpenRecord();
    if(!ret){
        return ret;
    }
    file_thread_->PostDelayed(frameInternal_,this,MSG_RECORD_SEND);
    return ret;
}


RecordReaderInterface::RecordReaderInterface(int frame_internal):
    file_thread_(NULL),frameInternal_(frame_internal),useInterval()
{

}


FileRecordReader::FileRecordReader(std::string filename,int frame_internal):
    RecordReaderInterface(frame_internal), filename_(filename),file_stream_(NULL)
{
    sendFrameHead_ = NULL;

}

FileRecordReader::~FileRecordReader()
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
    talk_base::StreamResult result;

    if(sendFrameHead_){
        int framelen = sendFrameHead_->frameLen;
        char * framedata = new char[framelen];
        result =  file_stream_->Read(framedata,framelen,NULL,NULL);
        if(result == talk_base::SR_SUCCESS){
            buffer->AppendData(reinterpret_cast<void *>(sendFrameHead_),sizeof(KEFrameHead));
            buffer->AppendData(framedata,framelen);
        }else if(result == talk_base::SR_EOS){
            delete sendFrameHead_;
            sendFrameHead_ = NULL;
            this->CloseRecord();
            return NULL;
        }else{
            LOG(WARNING)<<"FileRecordReader::ReadRecordFrame---read error file:"<<filename_;
            delete sendFrameHead_;
            sendFrameHead_ = NULL;
            this->CloseRecord();
            return NULL;
        }
    }

    talk_base::scoped_ptr<KEFrameHead> frame_head;
    frame_head.reset(new KEFrameHead());
    result =  file_stream_->Read(reinterpret_cast<void *>(frame_head.get()),sizeof(KEFrameHead),NULL,NULL);
    if(result == talk_base::SR_SUCCESS){

    }else if(result == talk_base::SR_EOS){
        this->CloseRecord();
        return NULL;
    }else{
        LOG(WARNING)<<"FileRecordReader::ReadRecordFrame---read error file:"<<filename_;
        return NULL;
    }

    if(send_speed_ > 0 && sendFrameHead_){
        int second = frame_head->second - sendFrameHead_->second ;
        int tenmsecond = frame_head->millisecond - sendFrameHead_->millisecond;
        frameInternal_ = 1000*second + 10*tenmsecond;
    }


    sendFrameHead_ = frame_head.release();

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


