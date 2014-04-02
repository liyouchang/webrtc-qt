/*
 * JniPeerConnection.h
 *
 *  Created on: 2014-4-1
 *      Author: lht
 */

#ifndef JNIPEERCONNECTION_H_
#define JNIPEERCONNECTION_H_

#include "libjingle_app/PeerConnectionClinetInterface.h"
#include "talk/base/thread.h"

class JniPeerConnection : public kaerp2p::PeerConnectionClientInterface {
public:
	JniPeerConnection();
	virtual ~JniPeerConnection();

    virtual bool SendToPeer(const std::string &peer_id, const std::string& message);
protected:
    bool SendToPeer_jni(const std::string &peer_id, const std::string& message);
    talk_base::Thread * jni_thread;
};

#endif /* JNIPEERCONNECTION_H_ */
