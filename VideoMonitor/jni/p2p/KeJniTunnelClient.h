/*
 * KeJniTunnelClient.h
 * An interface to control tunnel
 *  Created on: 2014-4-1
 *      Author: lht
 */

#ifndef KEJNITUNNELCLIENT_H_
#define KEJNITUNNELCLIENT_H_
#include "libjingle_app/ketunnelclient.h"
#include "libjingle_app/kelocalclient.h"

class KeJniTunnelClient : public kaerp2p::KeTunnelClient{
public:
	KeJniTunnelClient();
	virtual ~KeJniTunnelClient();
    virtual void OnTunnelOpened(kaerp2p::PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnTunnelClosed(kaerp2p::PeerTerminalInterface * t,
                                const std::string & peer_id);
protected:
    virtual void OnRecvAudioData(const std::string & peer_id,
                                 const char * data,int len);
    virtual void OnRecvVideoData(const std::string & peer_id,
                                 const char * data,int len);
    virtual void OnRecordStatus(const std::string & peer_id,int status);
};


class KeJniLocalClient : public kaerp2p::KeLocalClient
{
public:
    KeJniLocalClient();
    virtual ~KeJniLocalClient();
    virtual void OnTunnelOpened(kaerp2p::PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnTunnelClosed(kaerp2p::PeerTerminalInterface * t,
                                const std::string & peer_id);
protected:
    virtual void OnSearchedDeviceInfo(const std::string &devInfo);
    virtual void OnRecvAudioData(const std::string & peer_id,
                                 const char * data,int len);
    virtual void OnRecvVideoData(const std::string & peer_id,
                                 const char * data,int len);
};

#endif /* KEJNITUNNELCLIENT_H_ */
