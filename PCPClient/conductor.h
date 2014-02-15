/*
 * libjingle
 * Copyright 2012, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
#define PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>

#include "main_wnd.h"
#include "peer_connection_client.h"
#include "talk/app/kaerp2p/KaerSession.h"
#include "peer_connection_client.h"
#include <iostream>
#include "talk/app/webrtc/peerconnectioninterface.h"

class ClientFileProcess;

class Conductor
        :public kaerp2p::IceObserver,
        public webrtc::CreateSessionDescriptionObserver,
        public PeerConnectionClientObserver,
        public MainWndCallback {
public:
    enum CallbackID {
        MEDIA_CHANNELS_INITIALIZED = 1,
        PEER_CONNECTION_CLOSED,
        SEND_MESSAGE_TO_PEER,
        PEER_CONNECTION_ERROR,
        NEW_STREAM_ADDED,
        STREAM_REMOVED,
    };

    Conductor(PeerConnectionClient* client, MainWindowInterface* main_wnd);


    virtual void Close();

protected:
    ~Conductor();
    bool InitializePeerConnection();
    void DeletePeerConnection();


    //
    // IceObserver implementation.
    //
    void OnIceConnectionChange(IceConnectionState new_state){};
    void OnIceGatheringChange(IceGatheringState new_state){};
    void OnIceCandidate(const IceCandidateInterface *candidate);
    void OnIceComplete(){};
    //
    // PeerConnectionClientObserver implementation.
    //

    virtual void OnSignedIn();

    virtual void OnDisconnected();

    virtual void OnPeerConnected(int id, const std::string& name);

    virtual void OnPeerDisconnected(int id);

    virtual void OnMessageFromPeer(int peer_id, const std::string& message);

    virtual void OnMessageSent(int err);

    virtual void OnServerConnectionFailure();

    //
    // MainWndCallback implementation.
    //

    virtual void StartLogin(const std::string& server, int port);

    virtual void DisconnectFromServer();

    virtual void ConnectToPeer(int peer_id);

    virtual void DisconnectFromCurrentPeer();

    virtual void UIThreadCallback(int msg_id, void* data);

    // CreateSessionDescriptionObserver implementation.
    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
    virtual void OnFailure(const std::string& error);

protected:
    // Send a message to the remote peer.
    void SendUIMessage(const std::string& json_object);

    int peer_id_;
    PeerConnectionClient* client_;
    MainWindowInterface* main_wnd_;
    std::deque<std::string*> pending_messages_;
    std::string server_;

    talk_base::scoped_ptr<kaerp2p::KaerSession> session_;
    talk_base::BasicNetworkManager network_manager_;

    talk_base::scoped_refptr<PortAllocatorFactoryInterface> allocator_factory_;

    talk_base::scoped_ptr<cricket::PortAllocator> allocator_;

    ClientFileProcess * fileProcess;
};


enum {
    MSG_LOGIN_COMPLETE = 1,
    MSG_LOGIN_FAILED,
    MSG_DONE,
};

class ClientFileProcess:public sigslot::has_slots<>
{
public:
    ClientFileProcess():server_(false){}

    bool ProcessStream(talk_base::StreamInterface* stream,
                       const std::string& filename, bool send);
    std::string filesize_to_string(unsigned int size) {
        double size_display;
        std::string format;
        std::stringstream ret;

        // the comparisons to 1000 * (2^(n10)) are intentional
        // it's so you don't see something like "1023 bytes",
        // instead you'll see ".9 KB"

        if (size < 1000) {
            format = "Bytes";
            size_display = size;
        } else if (size < 1000 * 1024) {
            format = "KiB";
            size_display = (double)size / 1024.0;
        } else if (size < 1000 * 1024 * 1024) {
            format = "MiB";
            size_display = (double)size / (1024.0 * 1024.0);
        } else {
            format = "GiB";
            size_display = (double)size / (1024.0 * 1024.0 * 1024.0);
        }

        ret << std::setprecision(1) << std::setiosflags(std::ios::fixed) << size_display << " " << format;
        return ret.str();
    }
    void OnStreamEvent(talk_base::StreamInterface* stream, int events,
                       int error) {
        if (events & talk_base::SE_CLOSE) {
            if (error == 0) {
                LOG(INFO) << "Tunnel closed normally" << std::endl;
            } else {
                LOG(INFO) << "Tunnel closed with error: " << error << std::endl;
            }
            Cleanup(stream);
            return;
        }
        if (events & talk_base::SE_OPEN) {
            //std::cout << "Tunnel connected" << std::endl;
            LOG(INFO)<<__FUNCTION__<<" ---- Tunnel Connected";
        }
        talk_base::StreamResult result;
        size_t count;
        if (sending_ && (events & talk_base::SE_WRITE)) {
            LOG(LS_VERBOSE) << "Tunnel SE_WRITE";
            while (true) {
                size_t write_pos = 0;
                while (write_pos < buffer_len_) {
                    result = stream->Write(buffer_ + write_pos, buffer_len_ - write_pos,
                                           &count, &error);
                    if (result == talk_base::SR_SUCCESS) {
                        write_pos += count;
                        continue;
                    }
                    if (result == talk_base::SR_BLOCK) {
                        buffer_len_ -= write_pos;
                        memmove(buffer_, buffer_ + write_pos, buffer_len_);
                        LOG(LS_VERBOSE) << "Tunnel write block";
                        return;
                    }
                    if (result == talk_base::SR_EOS) {
                        std::cout << "Tunnel closed unexpectedly on write" << std::endl;
                    } else {
                        std::cout << "Tunnel write error: " << error << std::endl;
                    }
                    Cleanup(stream);
                    return;
                }
                buffer_len_ = 0;
                while (buffer_len_ < sizeof(buffer_)) {
                    result = file_->Read(buffer_ + buffer_len_,
                                         sizeof(buffer_) - buffer_len_,
                                         &count, &error);
                    if (result == talk_base::SR_SUCCESS) {
                        buffer_len_ += count;
                        continue;
                    }
                    if (result == talk_base::SR_EOS) {
                        if (buffer_len_ > 0)
                            break;
                        std::cout << "End of file" << std::endl;
                        // A hack until we have friendly shutdown
                        Cleanup(stream, true);
                        return;
                    } else if (result == talk_base::SR_BLOCK) {
                        std::cout << "File blocked unexpectedly on read" << std::endl;
                    } else {
                        std::cout << "File read error: " << error << std::endl;
                    }
                    Cleanup(stream);
                    return;
                }
            }
        }
        if (!sending_ && (events & talk_base::SE_READ)) {
            LOG(LS_VERBOSE) << "Tunnel SE_READ";
            while (true) {
                buffer_len_ = 0;
                while (buffer_len_ < sizeof(buffer_)) {
                    result = stream->Read(buffer_ + buffer_len_,
                                          sizeof(buffer_) - buffer_len_,
                                          &count, &error);
                    if (result == talk_base::SR_SUCCESS) {
                        buffer_len_ += count;

                        continue;
                    }
                    if (result == talk_base::SR_BLOCK) {
                        if (buffer_len_ > 0)
                            break;
                        LOG(LS_VERBOSE) << "Tunnel read block";
                        return;
                    }
                    if (result == talk_base::SR_EOS) {
                        LOG(INFO) << "Tunnel closed unexpectedly on read";
                    } else {
                        LOG(WARNING) << "Tunnel read error: " << error << std::endl;
                    }
                    Cleanup(stream);
                    return;
                }
                size_t write_pos = 0;
                while (write_pos < buffer_len_) {
                    result = file_->Write(buffer_ + write_pos, buffer_len_ - write_pos,
                                          &count, &error);
                    if (result == talk_base::SR_SUCCESS) {
                        write_pos += count;
                        size_t fileSize;
                        file_->GetSize(&fileSize);
                        LOG(INFO) <<filesize_to_string(fileSize);
                        continue;
                    }
                    if (result == talk_base::SR_EOS) {
                        std::cout << "File closed unexpectedly on write" << std::endl;
                    } else if (result == talk_base::SR_BLOCK) {
                        std::cout << "File blocked unexpectedly on write" << std::endl;
                    } else {
                        std::cout << "File write error: " << error << std::endl;
                    }
                    Cleanup(stream);
                    return;
                }
            }
        }
    }
    void Cleanup(talk_base::StreamInterface* stream, bool delay = false) {
        //LOG(LS_VERBOSE) << "Closing";
        LOG(INFO) << "Closing";
        stream->Close();
        file_.reset();
        if (!server_) {
            if (delay)
                talk_base::Thread::Current()->PostDelayed(2000, NULL, MSG_DONE);
            else
                talk_base::Thread::Current()->Post(NULL, MSG_DONE);
        }
    }

private:
    bool server_, sending_;
    talk_base::scoped_ptr<talk_base::FileStream> file_;
    char buffer_[1024 * 64];
    size_t buffer_len_;
};
#endif  // PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
