/*
 * libjingle
 * Copyright 2004--2008, Google Inc.
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

#ifndef __TUNNELSESSIONCLIENT_H__
#define __TUNNELSESSIONCLIENT_H__

#include <vector>

#include "talk/base/criticalsection.h"
#include "talk/base/stream.h"
#include "talk/p2p/base/constants.h"
#include "talk/p2p/base/pseudotcp.h"
#include "talk/p2p/base/session.h"
#include "talk/p2p/base/sessiondescription.h"
#include "talk/p2p/base/transportdescriptionfactory.h"

namespace cricket {

class TunnelSession;
class TunnelStream;

enum TunnelSessionRole { INITIATOR, RESPONDER };



///////////////////////////////////////////////////////////////////////////////
// TunnelContentDescription
///////////////////////////////////////////////////////////////////////////////

struct TunnelContentDescription : public ContentDescription {
  std::string description;

  TunnelContentDescription(const std::string& desc) : description(desc) { }
  virtual ContentDescription* Copy() const {
    return new TunnelContentDescription(*this);
  }
};

bool FindTunnelContent(const cricket::SessionDescription* sdesc,
                       std::string* name,
                       const TunnelContentDescription** content);

///////////////////////////////////////////////////////////////////////////////
// TunnelSessionClient
///////////////////////////////////////////////////////////////////////////////

// Base class is still abstract
class TunnelSessionClientBase: public talk_base::MessageHandler {
public:
  TunnelSessionClientBase(const std::string &ns);
  virtual ~TunnelSessionClientBase();

  talk_base::Thread* signaling_thread() { return signaling_thread_; }
  talk_base::Thread* worker_thread() { return worker_thread_; }

  void OnSessionCreate(BaseSession* session, bool received);
  void OnSessionDestroy(BaseSession* session);

  // This can be called on any thread.  The stream interface is
  // thread-safe, but notifications must be registered on the creating
  // thread.
  talk_base::StreamInterface* CreateTunnel(const std::string& description);

  talk_base::StreamInterface* AcceptTunnel(BaseSession* session);
  void DeclineTunnel(BaseSession* session);

  // Invoked on an incoming tunnel
  virtual void OnIncomingTunnel( BaseSession *session) = 0;

  // Invoked on an outgoing session request
  virtual SessionDescription* CreateOffer(const std::string &description) = 0;
  // Invoked on a session request accept to create
  // the local-side session description
  virtual SessionDescription* CreateAnswer(
      const SessionDescription* offer) = 0;

protected:
  virtual  TunnelSession* CreateTunnelInternel(SessionDescription* offer) = 0;

  void OnMessage(talk_base::Message* pmsg);

  // helper method to instantiate TunnelSession. By overriding this,
  // subclasses of TunnelSessionClient are able to instantiate
  // subclasses of TunnelSession instead.
  virtual TunnelSession* MakeTunnelSession(BaseSession* session,
                                           talk_base::Thread* stream_thread,
                                           TunnelSessionRole role);

  std::vector<TunnelSession*> sessions_;
  std::string namespace_;
  bool shutdown_;
  talk_base::Thread* signaling_thread_;
  talk_base::Thread* worker_thread_;
};

class TunnelSessionClient
  : public TunnelSessionClientBase, public sigslot::has_slots<>  {
public:
  TunnelSessionClient();
  TunnelSessionClient(const std::string &ns);
  virtual ~TunnelSessionClient();

  TransportDescriptionFactory * transport_desc_factory(){return &desc_factory;}
  // Signal arguments are this, initiator, description, session
  sigslot::signal3<TunnelSessionClient*, std::string, BaseSession*>
    SignalIncomingTunnel;

  virtual void OnIncomingTunnel(BaseSession *session);
  virtual SessionDescription* CreateOffer(const std::string &description);
  virtual SessionDescription* CreateAnswer(
      const SessionDescription* offer);
protected:
  TransportDescriptionFactory  desc_factory;

};

///////////////////////////////////////////////////////////////////////////////
// TunnelSession
// Note: The lifetime of TunnelSession is complicated.  It needs to survive
// until the following three conditions are true:
// 1) TunnelStream has called Close (tracked via non-null stream_)
// 2) PseudoTcp has completed (tracked via non-null tcp_)
// 3) Session has been destroyed (tracked via non-null session_)
// This is accomplished by calling CheckDestroy after these indicators change.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// TunnelStream
// Note: Because TunnelStream provides a stream interface, its lifetime is
// controlled by the owner of the stream pointer.  As a result, we must support
// both the TunnelSession disappearing before TunnelStream, and vice versa.
///////////////////////////////////////////////////////////////////////////////

class PseudoTcpChannel;

class TunnelSession : public sigslot::has_slots<> {
 public:
  // Signalling thread methods
  TunnelSession(TunnelSessionClientBase* client, BaseSession* session,
                talk_base::Thread* stream_thread);

  virtual talk_base::StreamInterface* GetStream();
  bool HasSession(BaseSession* session);
  BaseSession* ReleaseSession(bool channel_exists);

 protected:
  virtual ~TunnelSession();

  virtual void OnSessionState(BaseSession* session, BaseSession::State state);
  virtual void OnInitiate();
  virtual void OnAccept();
  virtual void OnTerminate();
  virtual void OnChannelClosed(PseudoTcpChannel* channel);

  TunnelSessionClientBase* client_;
  BaseSession* session_;
  PseudoTcpChannel* channel_;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace cricket

#endif // __TUNNELSESSIONCLIENT_H__
