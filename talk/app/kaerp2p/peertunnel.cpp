#include "peertunnel.h"

#include "talk/app/webrtc/portallocatorfactory.h"
#include "talk/app/webrtc/jsepicecandidate.h"

namespace kaerp2p{



// The min number of tokens must present in Turn host uri.
// e.g. user@turn.example.org
static const size_t kTurnHostTokensNum = 2;
// Number of tokens must be preset when TURN uri has transport param.
static const size_t kTurnTransportTokensNum = 2;
// The default stun port.
static const int kDefaultStunPort = 3478;
static const int kDefaultStunTlsPort = 5349;
static const char kTransport[] = "transport";
static const char kUdpTransportType[] = "udp";
static const char kTcpTransportType[] = "tcp";

// NOTE: Must be in the same order as the ServiceType enum.
static const char* kValidIceServiceTypes[] = {
    "stun", "stuns", "turn", "turns", "invalid" };

enum ServiceType {
    STUN,     // Indicates a STUN server.
    STUNS,    // Indicates a STUN server used with a TLS session.
    TURN,     // Indicates a TURN server
    TURNS,    // Indicates a TURN server used with a TLS session.
    INVALID,  // Unknown.
};

enum {
    MSG_SET_SESSIONDESCRIPTION_SUCCESS = 0,
    MSG_SET_SESSIONDESCRIPTION_FAILED,
    MSG_GETSTATS,
    MSG_ICECONNECTIONCHANGE,
    MSG_ICEGATHERINGCHANGE,
    MSG_ICECANDIDATE,
    MSG_ICECOMPLETE,
    MSG_CREATE_SESSION
};

struct CandidateMsg : public talk_base::MessageData {
    explicit CandidateMsg(const webrtc::JsepIceCandidate* candidate)
        : candidate(candidate) {
    }
    talk_base::scoped_ptr<const webrtc::JsepIceCandidate> candidate;
};

struct SetSessionDescriptionMsg : public talk_base::MessageData {
    explicit SetSessionDescriptionMsg(
            webrtc::SetSessionDescriptionObserver* observer)
        : observer(observer) {
    }

    talk_base::scoped_refptr<webrtc::SetSessionDescriptionObserver> observer;
    std::string error;
};
struct CreateSessionParams : public talk_base::MessageData {
  CreateSessionParams(
      cricket::PortAllocator * port_allocator)
      :
        port_allocator(port_allocator),
        result(true)
        {}
  cricket::PortAllocator * port_allocator;
  bool result;
};

PeerTunnel::PeerTunnel()
{
    signaling_thread_ = new talk_base::Thread();
    bool result = signaling_thread_->Start();
    ASSERT(result);
    worker_thread_ = new talk_base::Thread();
    result = worker_thread_->Start();
    ASSERT(result);
}

PeerTunnel::PeerTunnel(talk_base::Thread *signaling_thread, talk_base::Thread *worker_thread):
    signaling_thread_(signaling_thread),worker_thread_(worker_thread)
{

}

talk_base::StreamInterface *PeerTunnel::GetStream()
{
    return session_->GetStream();
}


// |in_str| should be of format
// stunURI       = scheme ":" stun-host [ ":" stun-port ]
// scheme        = "stun" / "stuns"
// stun-host     = IP-literal / IPv4address / reg-name
// stun-port     = *DIGIT

// draft-petithuguenin-behave-turn-uris-01
// turnURI       = scheme ":" turn-host [ ":" turn-port ]
// turn-host     = username@IP-literal / IPv4address / reg-name
bool GetServiceTypeAndHostnameFromUri(const std::string& in_str,
                                      ServiceType* service_type,
                                      std::string* hostname) {
  std::string::size_type colonpos = in_str.find(':');
  if (colonpos == std::string::npos) {
    return false;
  }
  std::string type = in_str.substr(0, colonpos);
  for (size_t i = 0; i < ARRAY_SIZE(kValidIceServiceTypes); ++i) {
    if (type.compare(kValidIceServiceTypes[i]) == 0) {
      *service_type = static_cast<ServiceType>(i);
      break;
    }
  }
  if (*service_type == INVALID) {
    return false;
  }
  *hostname = in_str.substr(colonpos + 1, std::string::npos);
  return true;
}

// This method parses IPv6 and IPv4 literal strings, along with hostnames in
// standard hostname:port format.
// Consider following formats as correct.
// |hostname:port|, |[IPV6 address]:port|, |IPv4 address|:port,
// |hostname|, |[IPv6 address]|, |IPv4 address|
bool ParseHostnameAndPortFromString(const std::string& in_str,
                                    std::string* host,
                                    int* port) {
  if (in_str.at(0) == '[') {
    std::string::size_type closebracket = in_str.rfind(']');
    if (closebracket != std::string::npos) {
      *host = in_str.substr(1, closebracket - 1);
      std::string::size_type colonpos = in_str.find(':', closebracket);
      if (std::string::npos != colonpos) {
        if (!talk_base::FromString(
            in_str.substr(closebracket + 2, std::string::npos), port)) {
          return false;
        }
      }
    } else {
      return false;
    }
  } else {
    std::string::size_type colonpos = in_str.find(':');
    if (std::string::npos != colonpos) {
      *host = in_str.substr(0, colonpos);
      if (!talk_base::FromString(
          in_str.substr(colonpos + 1, std::string::npos), port)) {
        return false;
      }
    } else {
      *host = in_str;
    }
  }
  return true;
}

typedef webrtc::PortAllocatorFactoryInterface::StunConfiguration
    StunConfiguration;
typedef webrtc::PortAllocatorFactoryInterface::TurnConfiguration
    TurnConfiguration;

bool ParseIceServers(const PeerTunnelInterface::IceServers& configuration,
                     std::vector<StunConfiguration>* stun_config,
                     std::vector<TurnConfiguration>* turn_config) {
  // draft-nandakumar-rtcweb-stun-uri-01
  // stunURI       = scheme ":" stun-host [ ":" stun-port ]
  // scheme        = "stun" / "stuns"
  // stun-host     = IP-literal / IPv4address / reg-name
  // stun-port     = *DIGIT

  // draft-petithuguenin-behave-turn-uris-01
  // turnURI       = scheme ":" turn-host [ ":" turn-port ]
  //                 [ "?transport=" transport ]
  // scheme        = "turn" / "turns"
  // transport     = "udp" / "tcp" / transport-ext
  // transport-ext = 1*unreserved
  // turn-host     = IP-literal / IPv4address / reg-name
  // turn-port     = *DIGIT
  for (size_t i = 0; i < configuration.size(); ++i) {
    PeerTunnelInterface::IceServer server = configuration[i];
    if (server.uri.empty()) {
      LOG(WARNING) << "Empty uri.";
      continue;
    }
    std::vector<std::string> tokens;
    std::string turn_transport_type = kUdpTransportType;
    talk_base::tokenize(server.uri, '?', &tokens);
    std::string uri_without_transport = tokens[0];
    // Let's look into transport= param, if it exists.
    if (tokens.size() == kTurnTransportTokensNum) {  // ?transport= is present.
      std::string uri_transport_param = tokens[1];
      talk_base::tokenize(uri_transport_param, '=', &tokens);
      if (tokens[0] == kTransport) {
        // As per above grammar transport param will be consist of lower case
        // letters.
        if (tokens[1] != kUdpTransportType && tokens[1] != kTcpTransportType) {
          LOG(LS_WARNING) << "Transport param should always be udp or tcp.";
          continue;
        }
        turn_transport_type = tokens[1];
      }
    }

    std::string hoststring;
    ServiceType service_type = INVALID;
    if (!GetServiceTypeAndHostnameFromUri(uri_without_transport,
                                         &service_type,
                                         &hoststring)) {
      LOG(LS_WARNING) << "Invalid transport parameter in ICE URI: "
                      << uri_without_transport;
      continue;
    }

    // Let's break hostname.
    tokens.clear();
    talk_base::tokenize(hoststring, '@', &tokens);
    hoststring = tokens[0];
    if (tokens.size() == kTurnHostTokensNum) {
      server.username = talk_base::s_url_decode(tokens[0]);
      hoststring = tokens[1];
    }

    int port = kDefaultStunPort;
    if (service_type == TURNS) {
      port = kDefaultStunTlsPort;
      turn_transport_type = kTcpTransportType;
    }

    std::string address;
    if (!ParseHostnameAndPortFromString(hoststring, &address, &port)) {
      LOG(WARNING) << "Invalid Hostname format: " << uri_without_transport;
      continue;
    }


    if (port <= 0 || port > 0xffff) {
      LOG(WARNING) << "Invalid port: " << port;
      continue;
    }

    switch (service_type) {
      case STUN:
      case STUNS:
        stun_config->push_back(StunConfiguration(address, port));
        break;
      case TURN:
      case TURNS: {
        if (server.username.empty()) {
          // Turn url example from the spec |url:"turn:user@turn.example.org"|.
          std::vector<std::string> turn_tokens;
          talk_base::tokenize(address, '@', &turn_tokens);
          if (turn_tokens.size() == kTurnHostTokensNum) {
            server.username = talk_base::s_url_decode(turn_tokens[0]);
            address = turn_tokens[1];
          }
        }

        bool secure = (service_type == TURNS);

        turn_config->push_back(TurnConfiguration(
                                   address, port,
                                                 server.username,
                                                 server.password,
                                                 turn_transport_type,
                                                 secure));
        // STUN functionality is part of TURN.
        // Note: If there is only TURNS is supplied as part of configuration,
        // we will have problem in fetching server reflexive candidate, as
        // currently we don't have support of TCP/TLS in stunport.cc.
        // In that case we should fetch server reflexive addess from
        // TURN allocate response.
        stun_config->push_back(StunConfiguration(address, port));
        break;
      }
      case INVALID:
      default:
        LOG(WARNING) << "Configuration not supported: " << server.uri;
        return false;
    }
  }
  return true;
}

bool PeerTunnel::Initialize(const IceServers &configuration,
                            PeerTunnelObserver *observer)
{
    std::vector<PortAllocatorFactoryInterface::StunConfiguration> stun_config;
    std::vector<PortAllocatorFactoryInterface::TurnConfiguration> turn_config;
    if (!ParseIceServers(configuration, &stun_config, &turn_config)) {
        return false;
    }
    if(!allocator_factory_){
        allocator_factory_ = PortAllocatorFactory::Create(worker_thread_);
        if (!allocator_factory_)
            return false;
    }

    ASSERT(observer != NULL);
    if (!observer)
        return false;
    observer_ = observer;
    port_allocator_.reset(
                allocator_factory_->CreatePortAllocator(stun_config, turn_config));

    // To handle both internal and externally created port allocator, we will
    // enable BUNDLE here.
    int portallocator_flags = cricket::PORTALLOCATOR_ENABLE_BUNDLE |
            cricket::PORTALLOCATOR_ENABLE_SHARED_UFRAG |
            cricket::PORTALLOCATOR_ENABLE_SHARED_SOCKET;

    port_allocator_->set_flags(portallocator_flags);
    // No step delay is used while allocating ports.
    port_allocator_->set_step_delay(cricket::kMinimumStepDelay);


    CreateSessionParams params( port_allocator_.get());
    signaling_thread_->Send(this, MSG_CREATE_SESSION, &params);
    return params.result;
}


const SessionDescriptionInterface *PeerTunnel::local_description() const
{
    return session_->local_desc_.get();

}

const SessionDescriptionInterface *PeerTunnel::remote_description() const
{
    return session_->remote_desc_.get();
}

void PeerTunnel::CreateOffer(CreateSessionDescriptionObserver *observer)
{
    if (!VERIFY(observer != NULL)) {
        LOG(LS_ERROR) << "CreateOffer - observer is NULL.";
        return;
    }
    session_->CreateOffer(observer);

}

void PeerTunnel::CreateAnswer(CreateSessionDescriptionObserver *observer)
{
    if (!VERIFY(observer != NULL)) {
        LOG(LS_ERROR) << "CreateAnswer - observer is NULL.";
        return;
    }
    session_->CreateAnswer(observer);
}

void PeerTunnel::SetLocalDescription(SetSessionDescriptionObserver *observer, SessionDescriptionInterface *desc)
{
    if (!VERIFY(observer != NULL)) {
        LOG(LS_ERROR) << "SetLocalDescription - observer is NULL.";
        return;
    }
    if (!desc) {
        PostSetSessionDescriptionFailure(observer, "SessionDescription is NULL.");
        return;
    }
    std::string error;
    if (!session_->SetLocalDescription(desc, &error)) {
        PostSetSessionDescriptionFailure(observer, error);
        return;
    }
    SetSessionDescriptionMsg* msg =  new SetSessionDescriptionMsg(observer);
    signaling_thread()->Post(this, MSG_SET_SESSIONDESCRIPTION_SUCCESS, msg);
}

void PeerTunnel::SetRemoteDescription(SetSessionDescriptionObserver *observer, SessionDescriptionInterface *desc)
{
    if (!VERIFY(observer != NULL)) {
        LOG(LS_ERROR) << "SetRemoteDescription - observer is NULL.";
        return;
    }
    if (!desc) {
        PostSetSessionDescriptionFailure(observer, "SessionDescription is NULL.");
        return;
    }
    std::string error;
    if (!session_->SetRemoteDescription(desc, &error)) {
        PostSetSessionDescriptionFailure(observer, error);
        return;
    }
    SetSessionDescriptionMsg* msg  = new SetSessionDescriptionMsg(observer);
    signaling_thread()->Post(this, MSG_SET_SESSIONDESCRIPTION_SUCCESS, msg);
}

bool PeerTunnel::AddIceCandidate(const IceCandidateInterface *ice_candidate)
{
    return session_->ProcessIceMessage(ice_candidate);
}

void PeerTunnel::Close()
{
    session_->Terminate();
}

PeerTunnelInterface::SignalingState PeerTunnel::signaling_state()
{
    return signaling_state_;
}

IceObserver::IceConnectionState PeerTunnel::ice_connection_state()
{
    return  ice_connection_state_;
}

IceObserver::IceGatheringState PeerTunnel::ice_gathering_state()
{
    return ice_gathering_state_;
}

void PeerTunnel::PostSetSessionDescriptionFailure(SetSessionDescriptionObserver *observer, const std::string &error)
{
    SetSessionDescriptionMsg* msg  = new SetSessionDescriptionMsg(observer);
    msg->error = error;
    signaling_thread()->Post(this, MSG_SET_SESSIONDESCRIPTION_FAILED, msg);

}

void PeerTunnel::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_SET_SESSIONDESCRIPTION_SUCCESS: {
        SetSessionDescriptionMsg* param =
                static_cast<SetSessionDescriptionMsg*>(msg->pdata);
        param->observer->OnSuccess();
        delete param;
        break;
    }
    case MSG_SET_SESSIONDESCRIPTION_FAILED: {
        SetSessionDescriptionMsg* param =
                static_cast<SetSessionDescriptionMsg*>(msg->pdata);
        param->observer->OnFailure(param->error);
        delete param;
        break;
    }
    case MSG_ICECONNECTIONCHANGE: {
        observer_->OnIceConnectionChange(ice_connection_state_);
        break;
    }
    case MSG_ICEGATHERINGCHANGE: {
        observer_->OnIceGatheringChange(ice_gathering_state_);
        break;
    }
    case MSG_ICECANDIDATE: {
        CandidateMsg* data = static_cast<CandidateMsg*>(msg->pdata);
        observer_->OnIceCandidate(data->candidate.get());
        delete data;
        break;
    }
    case MSG_CREATE_SESSION:{
        CreateSessionParams * params = static_cast<CreateSessionParams *>(msg->pdata);

        session_.reset(new KaerSession(this->signaling_thread_,
                                        this->worker_thread_,
                                         params->port_allocator));

        // Initialize the WebRtcSession. It creates transport channels etc.
        if (!session_->Initialize()){
            params->result = false;
            break;
        }

        // Register PeerConnection as receiver of local ice candidates.
        // All the callbacks will be posted to the application from PeerConnection.
        session_->RegisterIceObserver(this);
        session_->SignalState.connect(this, &PeerTunnel::OnSessionStateChange);
        break;
    }
    default:
        ASSERT(false && "Not implemented");
        break;
    }
}

void PeerTunnel::OnIceConnectionChange(IceObserver::IceConnectionState new_state)
{
    ice_connection_state_ = new_state;
    signaling_thread()->Post(this, MSG_ICECONNECTIONCHANGE);

}

void PeerTunnel::OnIceGatheringChange(IceObserver::IceGatheringState new_state)
{
    if (IsClosed()) {
        return;
    }
    ice_gathering_state_ = new_state;
    signaling_thread()->Post(this, MSG_ICEGATHERINGCHANGE);
}

void PeerTunnel::OnIceCandidate(const IceCandidateInterface *candidate)
{
    JsepIceCandidate* candidate_copy = NULL;
    if (candidate) {
        // TODO(ronghuawu): Make IceCandidateInterface reference counted instead
        // of making a copy.
        candidate_copy = new JsepIceCandidate(candidate->sdp_mid(),
                                              candidate->sdp_mline_index(),
                                              candidate->candidate());
    }
    // The Post takes the ownership of the |candidate_copy|.
    signaling_thread()->Post(this, MSG_ICECANDIDATE,
                             new CandidateMsg(candidate_copy));
}

void PeerTunnel::OnSessionStateChange(cricket::BaseSession *session, cricket::BaseSession::State state)
{
    switch (state) {
      case cricket::BaseSession::STATE_INIT:
        ChangeSignalingState(kStable);
        break;
      case cricket::BaseSession::STATE_SENTINITIATE:
        ChangeSignalingState(kHaveLocalOffer);
        break;
      case cricket::BaseSession::STATE_SENTPRACCEPT:
        ChangeSignalingState(kHaveLocalPrAnswer);
        break;
      case cricket::BaseSession::STATE_RECEIVEDINITIATE:
        ChangeSignalingState(kHaveRemoteOffer);
        break;
      case cricket::BaseSession::STATE_RECEIVEDPRACCEPT:
        ChangeSignalingState(kHaveRemotePrAnswer);
        break;
      case cricket::BaseSession::STATE_SENTACCEPT:
      case cricket::BaseSession::STATE_RECEIVEDACCEPT:
        ChangeSignalingState(kStable);
        break;
      case cricket::BaseSession::STATE_RECEIVEDTERMINATE:
        ChangeSignalingState(kClosed);
        break;
      default:
        break;
    }
}

void PeerTunnel::ChangeSignalingState(SignalingState signaling_state)
{
    signaling_state_ = signaling_state;
    if (signaling_state == kClosed) {
      ice_connection_state_ = kIceConnectionClosed;
      observer_->OnIceConnectionChange(ice_connection_state_);
      if (ice_gathering_state_ != kIceGatheringComplete) {
        ice_gathering_state_ = kIceGatheringComplete;
        observer_->OnIceGatheringChange(ice_gathering_state_);
      }
    }
    observer_->OnSignalingChange(signaling_state_);

}




}
