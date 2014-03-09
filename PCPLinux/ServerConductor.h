#ifndef MYCONDUCTOR_H
#define MYCONDUCTOR_H

#include "talk/app/kaerp2p/KaerSession.h"
#include "peer_connection_client.h"
#include <iostream>
#include "streamprocess.h"
#include <list>
#include "peertunnel.h"
namespace kaerp2p {



class ServerConductor:
        public webrtc::CreateSessionDescriptionObserver,
        public PeerTunnelObserver,
        public PeerConnectionClientObserver
{
public:
    enum CallbackID {
        MEDIA_CHANNELS_INITIALIZED = 1,
        PEER_CONNECTION_CLOSED,
        SEND_MESSAGE_TO_PEER,
        PEER_CONNECTION_ERROR,
        NEW_STREAM_ADDED,
        STREAM_REMOVED,
    };

    ServerConductor(PeerConnectionClient* client);
    ~ServerConductor();
    virtual void StartLogin(const std::string& server, int port);
    virtual void DisconnectFromServer();

    virtual void ConnectToPeer(int peer_id);
    virtual void DisconnectFromCurrentPeer();
    virtual StreamProcess * GetStreamProcess();


protected:
    bool InitializePeerConnection();
    void DeletePeerConnection();
    int peer_id_;
    PeerConnectionClient* client_;
    std::deque<std::string*> pending_messages_;
    std::string server_;
    talk_base::scoped_refptr<PeerTunnelInterface> peer_connection_;

    StreamProcess * streamprocess_;
    std::list<int> other_peer_id_list_;
    talk_base::Thread * stream_thread_;
    // CreateSessionDescriptionObserver interface
public:
    void OnSuccess(webrtc::SessionDescriptionInterface* desc);
    void OnFailure(const std::string &error);

public:
    void OnSignedIn();
    void OnDisconnected();
    void OnPeerConnected(int id, const std::string &name);
    void OnPeerDisconnected(int peer_id);
    void OnMessageFromPeer(int peer_id, const std::string &message);
    void OnMessageSent(int err);
    void OnServerConnectionFailure();

    void UIThreadCallback(int msg_id, void* data);


    // PeerTunnelObserver interface
public:
    void OnError();
    void OnRenegotiationNeeded(){}
    void OnIceCandidate(const IceCandidateInterface *candidate);

};


//class ServerFileProcess:public sigslot::has_slots<>
//{
//public:
//    ServerFileProcess():server_(true){}

//    bool ProcessStream(talk_base::StreamInterface* stream,
//                       const std::string& filename, bool send) {
//        //ASSERT(file_);
//        sending_ = send;
//        file_.reset(new talk_base::FileStream);
//        buffer_len_ = 0;
//        int err;
//        if (!file_->Open(filename.c_str(), sending_ ? "rb" : "wb", &err)) {
//            std::cerr << "Error opening <" << filename << ">: "
//                      << std::strerror(err) << std::endl;
//            return false;
//        }
//        stream->SignalEvent.connect(this, &ServerFileProcess::OnStreamEvent);
//        if (stream->GetState() == talk_base::SS_CLOSED) {
//            std::cerr << "Failed to establish P2P tunnel" << std::endl;
//            return false;
//        }
//        if (stream->GetState() == talk_base::SS_OPEN) {
//            OnStreamEvent(stream,
//                          talk_base::SE_OPEN | talk_base::SE_READ | talk_base::SE_WRITE, 0);
//        }
//        return true;
//    }
//    std::string filesize_to_string(unsigned int size) {
//        double size_display;
//        std::string format;
//        std::stringstream ret;

//        // the comparisons to 1000 * (2^(n10)) are intentional
//        // it's so you don't see something like "1023 bytes",
//        // instead you'll see ".9 KB"

//        if (size < 1000) {
//            format = "Bytes";
//            size_display = size;
//        } else if (size < 1000 * 1024) {
//            format = "KiB";
//            size_display = (double)size / 1024.0;
//        } else if (size < 1000 * 1024 * 1024) {
//            format = "MiB";
//            size_display = (double)size / (1024.0 * 1024.0);
//        } else {
//            format = "GiB";
//            size_display = (double)size / (1024.0 * 1024.0 * 1024.0);
//        }

//        ret << std::setprecision(1) << std::setiosflags(std::ios::fixed) << size_display << " " << format;
//        return ret.str();
//    }
//    void OnStreamEvent(talk_base::StreamInterface* stream, int events,
//                       int error) {
//        if (events & talk_base::SE_CLOSE) {
//            if (error == 0) {
//                std::cout << "Tunnel closed normally" << std::endl;
//            } else {
//                std::cout << "Tunnel closed with error: " << error << std::endl;
//            }
//            Cleanup(stream);
//            return;
//        }
//        if (events & talk_base::SE_OPEN) {
//            //std::cout << "Tunnel connected" << std::endl;
//            LOG(INFO)<<__FUNCTION__<<"Tunnel Connected";
//        }
//        talk_base::StreamResult result;
//        size_t count;
//        if (sending_ && (events & talk_base::SE_WRITE)) {
//            LOG(LS_VERBOSE) << "Tunnel SE_WRITE";
//            while (true) {
//                size_t write_pos = 0;
//                while (write_pos < buffer_len_) {
//                    result = stream->Write(buffer_ + write_pos, buffer_len_ - write_pos,
//                                           &count, &error);
//                    if (result == talk_base::SR_SUCCESS) {
//                        write_pos += count;
//                        continue;
//                    }
//                    if (result == talk_base::SR_BLOCK) {
//                        buffer_len_ -= write_pos;
//                        memmove(buffer_, buffer_ + write_pos, buffer_len_);
//                        LOG(LS_VERBOSE) << "Tunnel write block";
//                        return;
//                    }
//                    if (result == talk_base::SR_EOS) {
//                        std::cout << "Tunnel closed unexpectedly on write" << std::endl;
//                    } else {
//                        std::cout << "Tunnel write error: " << error << std::endl;
//                    }
//                    Cleanup(stream);
//                    return;
//                }
//                buffer_len_ = 0;
//                while (buffer_len_ < sizeof(buffer_)) {
//                    result = file_->Read(buffer_ + buffer_len_,
//                                         sizeof(buffer_) - buffer_len_,
//                                         &count, &error);
//                    if (result == talk_base::SR_SUCCESS) {
//                        buffer_len_ += count;
//                        continue;
//                    }
//                    if (result == talk_base::SR_EOS) {
//                        if (buffer_len_ > 0)
//                            break;
//                        LOG(INFO) << "End of file";
//                        // A hack until we have friendly shutdown
//                        Cleanup(stream, true);
//                        return;
//                    } else if (result == talk_base::SR_BLOCK) {
//                        LOG(WARNING)<< "File blocked unexpectedly on read" ;
//                    } else {
//                        LOG(WARNING)<< "File read error: " << error ;
//                    }
//                    Cleanup(stream);
//                    return;
//                }
//            }
//        }
//        if (!sending_ && (events & talk_base::SE_READ)) {
//            LOG(LS_VERBOSE) << "Tunnel SE_READ";
//            while (true) {
//                buffer_len_ = 0;
//                while (buffer_len_ < sizeof(buffer_)) {
//                    result = stream->Read(buffer_ + buffer_len_,
//                                          sizeof(buffer_) - buffer_len_,
//                                          &count, &error);
//                    if (result == talk_base::SR_SUCCESS) {
//                        buffer_len_ += count;
//                        continue;
//                    }
//                    if (result == talk_base::SR_BLOCK) {
//                        if (buffer_len_ > 0)
//                            break;
//                        LOG(LS_VERBOSE) << "Tunnel read block";
//                        return;
//                    }
//                    if (result == talk_base::SR_EOS) {
//                        std::cout << "Tunnel closed unexpectedly on read" << std::endl;
//                    } else {
//                        std::cout << "Tunnel read error: " << error << std::endl;
//                    }
//                    Cleanup(stream);
//                    return;
//                }
//                size_t write_pos = 0;
//                while (write_pos < buffer_len_) {
//                    result = file_->Write(buffer_ + write_pos, buffer_len_ - write_pos,
//                                          &count, &error);
//                    if (result == talk_base::SR_SUCCESS) {
//                        write_pos += count;
//                        continue;
//                    }
//                    if (result == talk_base::SR_EOS) {
//                        std::cout << "File closed unexpectedly on write" << std::endl;
//                    } else if (result == talk_base::SR_BLOCK) {
//                        std::cout << "File blocked unexpectedly on write" << std::endl;
//                    } else {
//                        std::cout << "File write error: " << error << std::endl;
//                    }
//                    Cleanup(stream);
//                    return;
//                }
//            }
//        }
//    }
//    void Cleanup(talk_base::StreamInterface* stream, bool delay = false) {
//        LOG(INFO) << "Closing";
//        //LOG(INFO) << "Closing";
//        stream->Close();
//        file_.reset();
//    }

//private:
//    bool server_, sending_;
//    talk_base::scoped_ptr<talk_base::FileStream> file_;
//    char buffer_[1024 * 64];
//    size_t buffer_len_;
//};

}
#endif // MYCONDUCTOR_H
