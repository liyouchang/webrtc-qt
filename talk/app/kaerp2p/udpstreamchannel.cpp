#include "udpstreamchannel.h"

#include "talk/base/bind.h"
#include "talk/p2p/base/transportchannel.h"

using namespace talk_base;

namespace kaerp2p {

UdpStreamChannel::UdpStreamChannel(talk_base::Thread* stream_thread,
                                   cricket::BaseSession* session):
    cricket::StreamChannelInterface(stream_thread,session),
    ready_to_connect_(false),m_rbuf(128*1024)
{
}

bool UdpStreamChannel::Connect(const std::string &content_name, const std::string &channel_name, int component)
{
    if (channel_)
        return false;

    ASSERT(session_ != NULL);
    worker_thread_ = session_->worker_thread();
    content_name_ = content_name;

    if (session_->initiator()) {
        // Since we may try several protocols and network adapters that won't work,
        // waiting until we get our first writable notification before initiating
        // TCP negotiation.
        ready_to_connect_ = true;
    }

    worker_thread_->Invoke<void>(
                talk_base::Bind(&StreamChannelInterface::CreateChannel_w,
                                (StreamChannelInterface *)this,
                                content_name, channel_name, component));

    return true;
}

talk_base::StreamState UdpStreamChannel::GetState() const
{
    ASSERT(stream_ != NULL && stream_thread_->IsCurrent());
    //CritScope lock(&cs_);
    if (!session_ || !channel_)
        return SS_CLOSED;
    if (channel_->writable())
        return SS_OPEN;
    if(ready_to_connect_){
        return SS_OPENING;
    }
    return SS_CLOSED;
}

StreamResult UdpStreamChannel::Read(void *buffer, size_t buffer_len, size_t *read, int *error)
{
    StreamResult result =  m_rbuf.Read(buffer,buffer_len,read,error);
    if(result == SR_SUCCESS){
        if(*read ==  buffer_len){
            stream_->PostEvent(stream_thread_,SE_READ,0);
        }
    }
    return result;
}

StreamResult UdpStreamChannel::Write(const void *data, size_t data_len, size_t *written, int *error)
{
    talk_base::PacketOptions option;
    int result = channel_->SendPacket(static_cast<const char*>(data),
                                      data_len,option);
    if (result > 0) {
        if (written)
            *written = result;
        return SR_SUCCESS;
    } else if (IsBlockingError(channel_->GetError())) {
        return SR_BLOCK;
    } else {
        if (error)
            *error = channel_->GetError();
        return SR_ERROR;
    }
}

void UdpStreamChannel::OnChannelDestroyed(cricket::TransportChannel *channel)
{
    LOG_F(LS_INFO) << "(" << channel->component() << ")";
    ASSERT(signal_thread_->IsCurrent());
    CritScope lock(&cs_);
    ASSERT(channel == channel_);
    session_ = NULL;
    channel_ = NULL;
    if (stream_ != NULL){
        stream_->PostEvent(stream_thread_,SE_CLOSE,0);
    }
    this->SignalChannelClosed(this);
    delete this;
}

void UdpStreamChannel::OnChannelWritableState(cricket::TransportChannel *channel)
{
    LOG_F(LS_VERBOSE) << "[" << channel_name_ << "]";
    ASSERT(worker_thread_->IsCurrent());
    if (!channel_) {
        LOG_F(LS_WARNING) << "NULL channel";
        return;
    }
    ASSERT(channel == channel_);
    if (!ready_to_connect_ || !channel->writable())
        return;

    ready_to_connect_ = false;
}

void UdpStreamChannel::OnChannelRead(cricket::TransportChannel *channel,
                                     const char *data, size_t size,
                                     const PacketTime &packet_time, int flags)
{
    ASSERT(worker_thread_->IsCurrent());
    CritScope lock(&cs_);
    if (!channel_) {
        LOG_F(LS_WARNING) << "NULL channel";
        return;
    }
    ASSERT(channel == channel_);

    size_t written;
    int error;
    //m_rbuf.GetWriteRemaining(&written);
    m_rbuf.Write(data,size,&written,&error);
    if(written < size){
        LOG_T_F(WARNING)<<" not enough space to read "<< size <<" written "<< written;
    }
    stream_->PostEvent(stream_thread_,SE_READ,0);
}

void UdpStreamChannel::OnChannelConnectionChanged(cricket::TransportChannel *channel, const cricket::Candidate &candidate)
{
    LOG_F(LS_VERBOSE) << "[" << channel_name_ << "]";
    ASSERT(worker_thread_->IsCurrent());
    CritScope lock(&cs_);
    if (!channel_) {
        LOG_F(LS_WARNING) << "NULL channel";
        return;
    }
    ASSERT(channel == channel_);
}

}
