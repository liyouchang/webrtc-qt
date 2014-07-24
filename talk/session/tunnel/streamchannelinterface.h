//streamchannelinterface.h
//created by lht

#ifndef STREAMCHANNELINTERFACE_H
#define STREAMCHANNELINTERFACE_H

#include "talk/base/stream.h"
#include "talk/p2p/base/session.h"

namespace cricket {

class StreamChannelInterface:public sigslot::has_slots<>
{
public:
    StreamChannelInterface(talk_base::Thread* stream_thread, BaseSession* session);

    virtual bool Connect(const std::string& content_name,
                         const std::string& channel_name,
                         int component) = 0;
    virtual talk_base::StreamInterface* GetStream();
    const std::string& content_name() { return content_name_; }

    sigslot::signal1<StreamChannelInterface*> SignalChannelClosed;

    // Stream thread methods
    virtual talk_base::StreamState GetState() const = 0;
    virtual talk_base::StreamResult Read(void* buffer, size_t buffer_len,
                                 size_t* read, int* error) = 0;
    virtual talk_base::StreamResult Write(const void* data, size_t data_len,
                                  size_t* written, int* error) = 0;
    virtual void Close() = 0;

    virtual void CreateChannel_w(const std::string& content_name,
                         const std::string& channel_name,int component);

protected:
    // Signal thread methods
    virtual void OnChannelDestroyed(TransportChannel* channel) = 0;

    // Worker thread methods
    virtual void OnChannelWritableState(TransportChannel* channel) = 0;
    virtual void OnChannelRead(TransportChannel* channel, const char* data, size_t size,
                       const talk_base::PacketTime& packet_time, int flags) = 0;
    virtual void OnChannelConnectionChanged(TransportChannel* channel,
                                    const Candidate& candidate) = 0;


    class InternalStream : public talk_base::StreamInterface {
    public:
      InternalStream(StreamChannelInterface* parent);
      virtual ~InternalStream();

      virtual talk_base::StreamState GetState() const;
      virtual talk_base::StreamResult Read(void* buffer, size_t buffer_len,
                                           size_t* read, int* error);
      virtual talk_base::StreamResult Write(const void* data, size_t data_len,
                                            size_t* written, int* error);
      virtual void Close();

    private:
      // parent_ is accessed and modified exclusively on the event thread, to
      // avoid thread contention.  This means that the PseudoTcpChannel cannot go
      // away until after it receives a Close() from TunnelStream.
      StreamChannelInterface* parent_;
    };
    friend class InternalStream;

    BaseSession* session_;
    TransportChannel* channel_;
    std::string content_name_;
    std::string channel_name_;
    talk_base::StreamInterface* stream_;
    talk_base::Thread * stream_thread_;
    talk_base::Thread* signal_thread_, * worker_thread_;

};

}

#endif // STREAMCHANNELINTERFACE_H
