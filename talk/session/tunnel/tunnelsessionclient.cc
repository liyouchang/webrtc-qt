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

#include "talk/base/basicdefs.h"
#include "talk/base/basictypes.h"
#include "talk/base/common.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/base/stringutils.h"
#include "talk/p2p/base/constants.h"
#include "talk/p2p/base/transportchannel.h"
#include "pseudotcpchannel.h"
#include "tunnelsessionclient.h"

namespace cricket {

const char NS_TUNNEL[] = "http://www.google.com/talk/tunnel";
const char CN_TUNNEL[] = "tunnel";

enum {
  MSG_CLOCK = 1,
  MSG_DESTROY,
  MSG_TERMINATE,
  MSG_EVENT,
  MSG_CREATE_TUNNEL,
};

struct EventData : public talk_base::MessageData {
  int event, error;
  EventData(int ev, int err = 0) : event(ev), error(err) { }
};

struct CreateTunnelData : public talk_base::MessageData {
  std::string description;
  talk_base::Thread* thread;
  talk_base::StreamInterface* stream;
};

extern const talk_base::ConstantLabel SESSION_STATES[];

const talk_base::ConstantLabel SESSION_STATES[] = {
  KLABEL(BaseSession::STATE_INIT),
  KLABEL(BaseSession::STATE_SENTINITIATE),
  KLABEL(BaseSession::STATE_RECEIVEDINITIATE),
  KLABEL(BaseSession::STATE_SENTACCEPT),
  KLABEL(BaseSession::STATE_RECEIVEDACCEPT),
  KLABEL(BaseSession::STATE_SENTMODIFY),
  KLABEL(BaseSession::STATE_RECEIVEDMODIFY),
  KLABEL(BaseSession::STATE_SENTREJECT),
  KLABEL(BaseSession::STATE_RECEIVEDREJECT),
  KLABEL(BaseSession::STATE_SENTREDIRECT),
  KLABEL(BaseSession::STATE_SENTTERMINATE),
  KLABEL(BaseSession::STATE_RECEIVEDTERMINATE),
  KLABEL(BaseSession::STATE_INPROGRESS),
  KLABEL(BaseSession::STATE_DEINIT),
  LASTLABEL
};



///////////////////////////////////////////////////////////////////////////////
// TunnelSessionClientBase
///////////////////////////////////////////////////////////////////////////////

TunnelSessionClientBase::TunnelSessionClientBase(const std::string &ns)
  : namespace_(ns), shutdown_(false) {

}

TunnelSessionClientBase::~TunnelSessionClientBase() {
  shutdown_ = true;
  for (std::vector<TunnelSession*>::iterator it = sessions_.begin();
       it != sessions_.end();
       ++it) {
     BaseSession* session = (*it)->ReleaseSession(true);
  }
}

void TunnelSessionClientBase::OnSessionCreate(BaseSession* session, bool received) {
  LOG(LS_INFO) << "TunnelSessionClientBase::OnSessionCreate: received=" 
               << received;
  //ASSERT(session_manager_->signaling_thread()->IsCurrent());
  if (received)
    sessions_.push_back(
        MakeTunnelSession(session, talk_base::Thread::Current(), RESPONDER));
}

void TunnelSessionClientBase::OnSessionDestroy(BaseSession* session) {
  LOG(LS_INFO) << "TunnelSessionClientBase::OnSessionDestroy";
  //ASSERT(session_manager_->signaling_thread()->IsCurrent());
  if (shutdown_)
    return;
  for (std::vector<TunnelSession*>::iterator it = sessions_.begin();
       it != sessions_.end();
       ++it) {
    if ((*it)->HasSession(session)) {
      VERIFY((*it)->ReleaseSession(false) == session);
      sessions_.erase(it);
      return;
    }
  }
}

talk_base::StreamInterface* TunnelSessionClientBase::CreateTunnel(const std::string& description) {
  // Valid from any thread
  CreateTunnelData data;
  data.description = description;
  data.thread = talk_base::Thread::Current();
  data.stream = NULL;
  signaling_thread()->Send(this, MSG_CREATE_TUNNEL, &data);
  return data.stream;

}

talk_base::StreamInterface* TunnelSessionClientBase::AcceptTunnel(
    BaseSession* session) {
  //ASSERT(session_manager_->signaling_thread()->IsCurrent());
  TunnelSession* tunnel = NULL;
  for (std::vector<TunnelSession*>::iterator it = sessions_.begin();
       it != sessions_.end();
       ++it) {
    if ((*it)->HasSession(session)) {
      tunnel = *it;
      break;
    }
  }
  ASSERT(tunnel != NULL);

  SessionDescription* answer = CreateAnswer(session->remote_description());
  if (answer == NULL)
    return NULL;

  //session->Accept(answer);
  return tunnel->GetStream();
}

void TunnelSessionClientBase::DeclineTunnel(BaseSession* session) {
 // ASSERT(session_manager_->signaling_thread()->IsCurrent());
  //session->Reject(STR_TERMINATE_DECLINE);
}

void TunnelSessionClientBase::OnMessage(talk_base::Message *pmsg)
{
    if (pmsg->message_id == MSG_CREATE_TUNNEL) {
      ASSERT(signaling_thread()->IsCurrent());
      CreateTunnelData* data = static_cast<CreateTunnelData*>(pmsg->pdata);
      SessionDescription* offer = CreateOffer(data->description);
      if (offer == NULL) {
        return;
      }
//      BaseSession* session = this->CreateSession(offer);
//      TunnelSession* tunnel = MakeTunnelSession(session, data->thread,
//                                                INITIATOR);
//      sessions_.push_back(tunnel);
      //session->Initiate(offer);
      TunnelSession* tunnel = CreateTunnelInternel(offer);
      data->stream = tunnel->GetStream();
    }
}


TunnelSession* TunnelSessionClientBase::MakeTunnelSession(BaseSession *session, talk_base::Thread* stream_thread,
    TunnelSessionRole /*role*/) {
  return new TunnelSession(this, session, stream_thread);
}

///////////////////////////////////////////////////////////////////////////////
// TunnelSessionClient
///////////////////////////////////////////////////////////////////////////////

TunnelSessionClient::TunnelSessionClient(const std::string &ns)
    : TunnelSessionClientBase(ns) {
}

TunnelSessionClient::TunnelSessionClient()
    : TunnelSessionClientBase( NS_TUNNEL) {
}

TunnelSessionClient::~TunnelSessionClient() {
}



SessionDescription* NewTunnelSessionDescription(
    const std::string& content_name, ContentDescription* content) {
  SessionDescription* sdesc = new SessionDescription();
  sdesc->AddContent(content_name, NS_TUNNEL, content);
  return sdesc;
}

bool FindTunnelContent(const cricket::SessionDescription* sdesc,
                       std::string* name,
                       const TunnelContentDescription** content) {
  const ContentInfo* cinfo = sdesc->FirstContentByType(NS_TUNNEL);
  if (cinfo == NULL)
    return false;

  *name = cinfo->name;
  *content = static_cast<const TunnelContentDescription*>(
      cinfo->description);
  return true;
}

void TunnelSessionClient::OnIncomingTunnel(BaseSession *session) {
  std::string content_name;
  const TunnelContentDescription* content = NULL;
  if (!FindTunnelContent(session->remote_description(),
                         &content_name, &content)) {
    //session->Reject(STR_TERMINATE_INCOMPATIBLE_PARAMETERS);
    return;
  }

  SignalIncomingTunnel(this,content->description, session);
}

SessionDescription* TunnelSessionClient::CreateOffer(const std::string &description) {
  SessionDescription* offer = NewTunnelSessionDescription(
      CN_TUNNEL, new TunnelContentDescription(description));
  talk_base::scoped_ptr<TransportDescription> tdesc(
      this->transport_desc_factory()->CreateOffer(
          TransportOptions(), NULL));
  if (tdesc.get()) {
    offer->AddTransportInfo(TransportInfo(CN_TUNNEL, *tdesc));
  } else {
    delete offer;
    offer = NULL;
  }
  return offer;
}

SessionDescription* TunnelSessionClient::CreateAnswer(
    const SessionDescription* offer) {
  std::string content_name = CN_TUNNEL;
  const TunnelContentDescription* offer_tunnel = NULL;
  if (!FindTunnelContent(offer, &content_name, &offer_tunnel))
    return NULL;

  SessionDescription* answer = NewTunnelSessionDescription(
      content_name, new TunnelContentDescription(offer_tunnel->description));
//   SessionDescription* answer = NewTunnelSessionDescription(
//        content_name, new TunnelContentDescription("test"));
  const TransportInfo* tinfo = offer->GetTransportInfoByName(content_name);
  if (tinfo) {
    const TransportDescription* offer_tdesc = &tinfo->description;
    ASSERT(offer_tdesc != NULL);
    talk_base::scoped_ptr<TransportDescription> tdesc(
      transport_desc_factory()->CreateAnswer(
          offer_tdesc, TransportOptions(),  NULL));
    if (tdesc.get()) {
      answer->AddTransportInfo(TransportInfo(content_name, *tdesc));
    } else {
      delete answer;
      answer = NULL;
    }
  }
  return answer;
}
///////////////////////////////////////////////////////////////////////////////
// TunnelSession
///////////////////////////////////////////////////////////////////////////////

//
// Signalling thread methods
//

TunnelSession::TunnelSession(TunnelSessionClientBase* client, BaseSession* session,
                             talk_base::Thread* stream_thread)
    : client_(client), session_(session), channel_(NULL) {
  ASSERT(client_ != NULL);
  ASSERT(session_ != NULL);
  session_->SignalState.connect(this, &TunnelSession::OnSessionState);
  channel_ = new PseudoTcpChannel(stream_thread, session_);
  channel_->SignalChannelClosed.connect(this, &TunnelSession::OnChannelClosed);
}

TunnelSession::~TunnelSession() {
  ASSERT(client_ != NULL);
  ASSERT(session_ == NULL);
  ASSERT(channel_ == NULL);
}

talk_base::StreamInterface* TunnelSession::GetStream() {
  ASSERT(channel_ != NULL);
  return channel_->GetStream();
}

bool TunnelSession::HasSession(BaseSession* session) {
  ASSERT(NULL != session_);
  return (session_ == session);
}

BaseSession* TunnelSession::ReleaseSession(bool channel_exists) {
  ASSERT(NULL != session_);
  ASSERT(NULL != channel_);
  BaseSession* session = session_;
  session_->SignalState.disconnect(this);
  session_ = NULL;
  if (channel_exists)
    channel_->SignalChannelClosed.disconnect(this);
  channel_ = NULL;
  delete this;
  return session;
}

void TunnelSession::OnSessionState(BaseSession* session,
                                   BaseSession::State state) {
  LOG(LS_INFO) << "TunnelSession::OnSessionState("
               << talk_base::nonnull(
                    talk_base::FindLabel(state, SESSION_STATES), "Unknown")
               << ")";
  ASSERT(session == session_);

  switch (state) {
  case BaseSession::STATE_RECEIVEDINITIATE:
    OnInitiate();
    break;
  case BaseSession::STATE_SENTACCEPT:
  case BaseSession::STATE_RECEIVEDACCEPT:
    OnAccept();
    break;
  case BaseSession::STATE_SENTTERMINATE:
  case BaseSession::STATE_RECEIVEDTERMINATE:
    OnTerminate();
    break;
  case BaseSession::STATE_DEINIT:
    // ReleaseSession should have been called before this.
    ASSERT(false);
    break;
  default:
    break;
  }
}

void TunnelSession::OnInitiate() {
  ASSERT(client_ != NULL);
  ASSERT(session_ != NULL);
  client_->OnIncomingTunnel(session_);
}

void TunnelSession::OnAccept() {
  ASSERT(channel_ != NULL);
  const ContentInfo* content =
      session_->remote_description()->FirstContentByType(NS_TUNNEL);
  ASSERT(content != NULL);
  VERIFY(channel_->Connect(
      content->name, "tcp", ICE_CANDIDATE_COMPONENT_DEFAULT
             ));
}

void TunnelSession::OnTerminate() {
  ASSERT(channel_ != NULL);
  channel_->OnSessionTerminate(session_);
}

void TunnelSession::OnChannelClosed(PseudoTcpChannel* channel) {
  ASSERT(channel_ == channel);
  ASSERT(session_ != NULL);
  //session_->Terminate();
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cricket
