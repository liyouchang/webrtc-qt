#include "streamprocess.h"
#include <iostream>
StreamProcess::StreamProcess()
{
    stream_ = NULL;
    this->workThread_ = talk_base::Thread::Current();
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
        LOG(LS_INFO) << "Tunnel SE_WRITE";
        while (!this->writeQueue_.empty()) {
            talk_base::Buffer  & writeBuf = this->writeQueue_.front();
            size_t write_pos = 0;
            const char * buffer = writeBuf.data();
            size_t buffer_len = writeBuf.length();
            while (write_pos < buffer_len) {
                result = stream->Write(buffer + write_pos, buffer_len - write_pos,
                                       &count, &error);
                if (result == talk_base::SR_SUCCESS) {
                    write_pos += count;
                    continue;
                }
                if (result == talk_base::SR_BLOCK) {
                    talk_base::Buffer leftBuf(buffer + write_pos, buffer_len - write_pos);
                    writeBuf = leftBuf;
                    //LOG(LS_VERBOSE) << "Tunnel write block";
                    LOG(LS_INFO) << "Tunnel write block";
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
            if(write_pos = buffer_len){
                LOG(INFO)<<"write success len"<< buffer_len;
                this->writeQueue_.pop();
            }else{//write blocked
                break;
            }
        }
    }
    if ( events & talk_base::SE_READ) {
        LOG(LS_VERBOSE) << "Tunnel SE_READ";

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
                LOG(INFO) << "read data "<<readBuf.length();

            }else{
                break;
            }
        }

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

    if (stream_->GetState() == talk_base::SS_CLOSED) {
        return false;
    }
    talk_base::Buffer buffer(data,len);
    talk_base::TypedMessageData<talk_base::Buffer> *msgData =
            new talk_base::TypedMessageData<talk_base::Buffer>(buffer);

    workThread_->Post(this,MSG_DATAWRITE,msgData);

    //    talk_base::StreamResult result;
    //    size_t count;
    //    size_t write_pos = 0;
    //    while (write_pos < len){
    //        result = stream_->Write(data+write_pos, len- write_pos,&count, &error);
    //        if (result == talk_base::SR_SUCCESS) {
    //            write_pos += count;
    //            continue;
    //        }
    //        if (result == talk_base::SR_BLOCK) {
    //            toWrite = len - write_pos;
    //            talk_base::ByteBuffer aBuff(data + write_pos, toWrite);
    //            writeQueue_.push(aBuff);
    //            LOG(LS_VERBOSE) << "Tunnel write block";
    //            return true;
    //        }
    //        if (result == talk_base::SR_EOS) {
    //            std::cout << "Tunnel closed unexpectedly on write" << std::endl;
    //        } else {
    //            std::cout << "Tunnel write error: " << error << std::endl;
    //        }
    //        Cleanup(stream_);
    //        return false;
    //    }
    return true;
}

bool StreamProcess::WriteBuffer(talk_base::ByteBuffer & buffer)
{
    if (stream_ == NULL)
        return -1;
    if (stream_->GetState() != talk_base::SS_OPEN) {
        return false;
    }

}

void StreamProcess::OnMessage(talk_base::Message *msg)
{
    if(msg->message_id == MSG_DATAWRITE)
    {
        talk_base::TypedMessageData<talk_base::Buffer> *msgData =
                static_cast< talk_base::TypedMessageData<talk_base::Buffer> *>(msg->pdata);
        writeQueue_.push(msgData->data());
        if(stream_->GetState() == talk_base::SS_OPEN){
            OnStreamEvent(stream_,talk_base::SE_WRITE, 0);
        }
    }
}
