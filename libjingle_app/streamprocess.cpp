#include "streamprocess.h"
#include <iostream>
#include "talk/base/timeutils.h"
namespace kaerp2p {

StreamProcess::StreamProcess(talk_base::Thread * stream_thread):
    read_buf_(128*1024),write_buf_(512*1024)
{
    stream_ = NULL;
    this->stream_thread_ = stream_thread;
    totalread = 0;
}

bool StreamProcess::ProcessStream(talk_base::StreamInterface *stream)
{
    ASSERT(stream_thread_->IsCurrent());
    stream->SignalEvent.connect(this, &StreamProcess::OnStreamEvent);
    if (stream->GetState() == talk_base::SS_CLOSED) {
        LOG(LS_ERROR) << "Failed to establish P2P tunnel";
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
    ASSERT(stream==stream_);
    ASSERT(stream_thread_->IsCurrent());

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
        LOG(INFO)<<__FUNCTION__<<"Tunnel Connected";
        SignalOpened();

    }
    size_t count;
    if (events & talk_base::SE_WRITE) {
        WriteStreamInternel();
    }
    if ( events & talk_base::SE_READ) {
        //LOG(LS_VERBOSE) << "Tunnel SE_READ";
        this->ReadStreamInternel();
    }
}

void StreamProcess::ReadStreamInternel()
{
    size_t toRead = 0;
    void * buffer = this->read_buf_.GetWriteBuffer(&toRead);
    if(buffer != NULL && toRead > 0){
        size_t read = 0;
        int error;
        talk_base::StreamResult result = stream_->Read(buffer,toRead,&read,&error);
        if(read > 0){
            this->read_buf_.ConsumeWriteBuffer(read);
            size_t allRead = 0;
            read_buf_.GetBuffered(&allRead);
            this->SignalReadData(this,allRead);
        }
        //LOG(INFO)<<"read " << read <<" bytes";
    }

}

void StreamProcess::WriteStreamInternel()
{
    size_t toWrite = 0;
    size_t written = 0;
    talk_base::StreamResult result;
    const void * buffer = this->write_buf_.GetReadData(&toWrite);
    if(buffer != NULL && toWrite > 0){
        int error;
        result = stream_->WriteAll(buffer,toWrite,&written,&error);
        if(written > 0){
            this->write_buf_.ConsumeReadData(written);
        }
        if(result == talk_base::SR_SUCCESS ){
            //still have something to write
            size_t leftWrite =0 ;
            write_buf_.GetBuffered(&leftWrite);
            if(leftWrite > 0 ){
                stream_->PostEvent(talk_base::SE_WRITE, 0);
            }
        }
    }
}

void StreamProcess::Cleanup(talk_base::StreamInterface *stream, bool delay) {
    LOG(INFO) << "StreamProcess::Cleanup";
    stream->Close();
    SignalClosed(this);
}

void StreamProcess::Cleanup()
{
    Cleanup(stream_);
}

bool StreamProcess::WriteStream(const char *data, int len)
{
    size_t wlen =0 ;
    int error;
    write_buf_.GetWriteRemaining(&wlen);
    if(len > wlen){
        LOG(INFO) << __FUNCTION__<< "not enough space to write, write len" << len
                  << " ,remainning "<<wlen;
        return false;
    }

    talk_base::StreamResult result = write_buf_.Write(data,len,&wlen,&error);
    if(result != talk_base::SR_SUCCESS){
        LOG(INFO) << __FUNCTION__<< "write_buf_ write buf error ";
        return false;
    }
    stream_->PostEvent(stream_thread_,talk_base::SE_WRITE, 0);
    return true;
}

bool StreamProcess::ReadStream(void *buffer, size_t bytes, size_t *bytes_read)
{
    int error;
    talk_base::StreamResult result =  read_buf_.Read(buffer,bytes,bytes_read ,&error);
    if(result != talk_base::SR_SUCCESS)
    {
        return false;
    }
    return true;
}


}
