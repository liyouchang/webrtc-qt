#ifndef UDPSTREAMCHANNEL_H
#define UDPSTREAMCHANNEL_H


#include "talk/base/messagequeue.h"
#include "talk/base/stream.h"
#include "talk/session/tunnel/streamchannelinterface.h"
namespace kaerp2p {

class UdpStreamChannel:public cricket::StreamChannelInterface
{
public:
    UdpStreamChannel(talk_base::Thread* stream_thread,
                     cricket::BaseSession* session);
    bool Connect(const std::string& content_name,
                 const std::string& channel_name,
                 int component);

    talk_base::StreamState GetState() const;
    talk_base::StreamResult Read(void *buffer, size_t buffer_len,
                                 size_t *read, int *error);
    talk_base::StreamResult Write(const void *data, size_t data_len,
                                  size_t *written, int *error);
    void Close();

protected:
    void OnChannelDestroyed(cricket::TransportChannel *channel);
    void OnChannelWritableState(cricket::TransportChannel *channel);
    void OnChannelRead(cricket::TransportChannel *channel,
                       const char *data, size_t size,
                       const talk_base::PacketTime &packet_time, int flags);
    void OnChannelConnectionChanged(cricket::TransportChannel *channel,
                                    const cricket::Candidate &candidate);
    talk_base::FifoBuffer m_rbuf;
    bool ready_to_connect_;
    mutable talk_base::CriticalSection cs_;

};
}
#endif // UDPSTREAMCHANNEL_H
