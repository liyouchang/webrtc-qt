#include "streamchannelinterface.h"

#include "talk/base/basictypes.h"
#include "talk/base/common.h"
#include "talk/base/logging.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/stringutils.h"
#include "talk/p2p/base/candidate.h"
#include "talk/p2p/base/transportchannel.h"
using namespace talk_base;

namespace cricket {


StreamChannelInterface::InternalStream::InternalStream(
        StreamChannelInterface *parent):parent_(parent)
{
}

StreamChannelInterface::InternalStream::~InternalStream()
{
    Close();
}

StreamState StreamChannelInterface::InternalStream::GetState() const
{
    if (!parent_)
        return SS_CLOSED;
    return parent_->GetState();
}

StreamResult StreamChannelInterface::InternalStream::Read(
        void *buffer, size_t buffer_len, size_t *read, int *error)
{
    if (!parent_) {
        if (error)
            *error = ENOTCONN;
        return SR_ERROR;
    }
    return parent_->Read(buffer, buffer_len, read, error);
}

StreamResult StreamChannelInterface::InternalStream::Write(
        const void *data, size_t data_len, size_t *written, int *error)
{
    if (!parent_) {
        if (error)
            *error = ENOTCONN;
        return SR_ERROR;
    }
    return parent_->Write(data, data_len, written, error);
}

void StreamChannelInterface::InternalStream::Close()
{
    if (!parent_)
        return;
    parent_->Close();
    parent_ = NULL;
}

StreamChannelInterface::StreamChannelInterface(
        Thread* stream_thread, BaseSession* session):
    stream_thread_(stream_thread),worker_thread_(NULL),signal_thread_(NULL),
    session_(session), channel_(NULL), stream_(NULL)
{
    signal_thread_ = session->signaling_thread();
}

talk_base::StreamInterface *StreamChannelInterface::GetStream()
{
    ASSERT(NULL != session_);
    if (!stream_)
        stream_ = new InternalStream(this);
    return stream_;
}

void StreamChannelInterface::CreateChannel_w(const std::string &content_name,
                                             const std::string &channel_name,
                                             int component)
{
    channel_ = session_->CreateChannel(content_name, channel_name, component);
    channel_name_ = channel_name;
    channel_->SetOption(Socket::OPT_DONTFRAGMENT, 1);
    channel_->SignalDestroyed.connect(this,&StreamChannelInterface::OnChannelDestroyed);
    channel_->SignalWritableState.connect(this,&StreamChannelInterface::OnChannelWritableState);
    channel_->SignalReadPacket.connect(this,&StreamChannelInterface::OnChannelRead);
    channel_->SignalRouteChange.connect(this,&StreamChannelInterface::OnChannelConnectionChanged);
}


}
