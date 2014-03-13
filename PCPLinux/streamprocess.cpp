#include "streamprocess.h"
#include <iostream>
#include "talk/base/timeutils.h"
namespace kaerp2p {

StreamProcess::StreamProcess(    talk_base::Thread * stream_thread)
{
    stream_ = NULL;
    this->stream_thread_ = stream_thread;
    totalread = 0;
}

bool StreamProcess::ProcessStream(talk_base::StreamInterface *stream)
{

    stream->SignalEvent.connect(this, &StreamProcess::OnStreamEvent);
    if (stream->GetState() == talk_base::SS_CLOSED) {
        std::cerr << "Failed to establish P2P tunnel" << std::endl;
        return false;
    }
    stream_ = stream;
    if (stream->GetState() == talk_base::SS_OPEN) {
        OnStreamEvent(stream,
                      talk_base::SE_OPEN | talk_base::SE_READ | talk_base::SE_WRITE, 0);
    }
    return true;
}

void StreamProcess::OnStreamEvent(talk_base::StreamInterface *stream,
                                  int events, int error) {
    if (events & talk_base::SE_CLOSE) {
        if (error == 0) {
            std::cout << "Tunnel closed normally" << std::endl;
        } else {
            std::cout << "Tunnel closed with error: " << error << std::endl;
        }
        Cleanup(stream);
        return;
    }
    if (events & talk_base::SE_OPEN) {
        //std::cout << "Tunnel connected" << std::endl;
        LOG(INFO)<<__FUNCTION__<<"Tunnel Connected";
    }
    talk_base::StreamResult result;
    size_t count;
    if (events & talk_base::SE_WRITE) {
        //LOG(LS_VERBOSE) << "Tunnel SE_WRITE";
        //LOG(LS_INFO) << "Tunnel SE_WRITE writeQueue_ size "<<writeQueue_.size();

        if (!this->writeQueue_.empty()) {
            talk_base::Buffer  & writeBuf = this->writeQueue_.front();
            size_t write_pos = 0;
            const char * buffer = writeBuf.data();
            size_t buffer_len = writeBuf.length();
            static size_t totalWrite = 0;
            while (write_pos < buffer_len) {
                result = stream->Write(buffer + write_pos, buffer_len - write_pos,
                                       &count, &error);
                if (result == talk_base::SR_SUCCESS) {
                    write_pos += count;
                    totalWrite += count;
                    //LOG(LS_INFO) << "Tunnel write continue count  "<<count;

                    continue;
                }
                if (result == talk_base::SR_BLOCK) {
                    talk_base::Buffer leftBuf(buffer + write_pos, buffer_len - write_pos);
                    writeBuf = leftBuf;
                    static size_t totalLeft = 0;
                    totalLeft += buffer_len - write_pos;
                    LOG(LS_VERBOSE) << "Tunnel write block";
                    //LOG(LS_INFO) << "Tunnel write block left size "<<leftBuf.length()<<" total left"<<totalLeft;
                    break;
                }
                if (result == talk_base::SR_EOS) {
                    std::cout << "Tunnel closed unexpectedly on write" << std::endl;
                } else {
                    std::cout << "Tunnel write error: " << error << std::endl;
                }
                Cleanup(stream);
                //write failed
                return;
            }
            if(write_pos == buffer_len){
                //LOG(INFO)<<"total write success len "<< totalWrite;
                this->writeQueue_.pop();
            }else{//write blocked
            }
        }
        if(!this->writeQueue_.empty()){
            stream_->PostEvent(talk_base::SE_WRITE, 0);
        }
        //LOG(LS_INFO) << "Tunnel SE_WRITE end writeQueue_ size "<<writeQueue_.size();

    }
    if ( events & talk_base::SE_READ) {
        //LOG(LS_VERBOSE) << "Tunnel SE_READ";
        talk_base::Buffer readBuf;
        char buffer[2048];
        while (true) {
            size_t buffer_len = 0;
            while (buffer_len < sizeof(buffer)) {
                result = stream->Read(buffer + buffer_len,
                                      sizeof(buffer) - buffer_len,
                                      &count, &error);
                if (result == talk_base::SR_SUCCESS) {
                    buffer_len += count;
                    continue;
                }
                if (result == talk_base::SR_BLOCK) {
                    LOG(LS_VERBOSE) << "Tunnel read block";
                    break;
                }
                if (result == talk_base::SR_EOS) {
                    std::cout << "Tunnel closed unexpectedly on read" << std::endl;
                } else {
                    std::cout << "Tunnel read error: " << error << std::endl;
                }
                Cleanup(stream);
                break;
            }
            if(buffer_len > 0){
                readBuf.AppendData(buffer,buffer_len);
                //LOG(INFO)<<"total read "<<totalRead;
                //
            }else{
                break;
            }
        }
        if(readBuf.length() > 0)
            OnReadBuffer(readBuf);

    }
}

void StreamProcess::Cleanup(talk_base::StreamInterface *stream, bool delay) {
    LOG(INFO) << "Closing";
    stream->Close();
}

bool StreamProcess::WriteData(const char *data, int len)
{
    if(stream_ == NULL)
        return false;

//    if (stream_->GetState() == talk_base::SS_CLOSED) {
//        return false;
//    }
    talk_base::Buffer buffer(data,len);
    talk_base::TypedMessageData<talk_base::Buffer> *msgData =
            new talk_base::TypedMessageData<talk_base::Buffer>(buffer);

    stream_thread_->Post(this,MSG_DATAWRITE,msgData);
    return true;
}

bool StreamProcess::WriteBuffer(const talk_base::Buffer &buffer)
{
    if(stream_ == NULL)
        return false;

//    if (stream_->GetState() == talk_base::SS_CLOSED) {
//        return false;
//    }
    talk_base::TypedMessageData<talk_base::Buffer> *msgData =
            new talk_base::TypedMessageData<talk_base::Buffer>(buffer);

    stream_thread_->Post(this,MSG_DATAWRITE,msgData);
    return true;
}

void StreamProcess::Cleanup()
{
    Cleanup(stream_);
}

void StreamProcess::OnReadBuffer(talk_base::Buffer &buffer)
{
//    std::string readStr(buffer.data(),buffer.length());
//    LOG(INFO) << "read data : "<<readStr;

    if(buffer.length() == 0){
        LOG(INFO)<<"get zero length";
    }
    else{
       totalread += buffer.length();
        LOG(INFO)<<"read length "<<totalread  ;
}

}

void StreamProcess::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_DATAWRITE:
    {
        talk_base::TypedMessageData<talk_base::Buffer> *msgData =
                static_cast< talk_base::TypedMessageData<talk_base::Buffer> *>(msg->pdata);
        writeQueue_.push(msgData->data());
        if(stream_->GetState() == talk_base::SS_OPEN && writeQueue_.size()==1){
            OnStreamEvent(stream_,talk_base::SE_WRITE, 0);
        }
    }
        break;
    default:
        break;
    }

}

}
