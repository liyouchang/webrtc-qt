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
#include "talk/base/messagehandler.h"
class JniPeerConnection : public kaerp2p::PeerConnectionClientInterface ,public talk_base::MessageHandler{
public:
	enum{
		MSG_SENDTOPEER
	};
	JniPeerConnection();
	virtual ~JniPeerConnection();

    virtual bool SendToPeer(const std::string &peer_id, const std::string& message);
protected:
    bool SendToPeer_jni(const std::string &peer_id, const std::string& message);
    //talk_base::Thread * jni_thread;
    virtual void OnMessage(talk_base::Message * msg);
};

#endif /* JNIPEERCONNECTION_H_ */
