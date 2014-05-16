/*
 *
 * Copyright 2014 Kaer Electric Co.,Ltd
 * Listens see <http://www.gnu.org/licenses/>.
 * AUTHORS lht
 *
 * AsynDealer类实现了一个异步zmq dealer的通讯方法
 *
 */

#ifndef ASYNDEALER_H
#define ASYNDEALER_H


#include "zmq.h"
#include "zmq.hpp"
#include "zhelpers.hpp"
#include "zmsg.hpp"


#include "talk/base/thread.h"
#include "talk/base/sigslot.h"
#include "talk/base/buffer.h"
#include <queue>

class AsynDealer: public talk_base::MessageHandler
{
public:
    enum{
        MSG_TOREAD,
        MSG_TOSEND
    };
    AsynDealer();
    ~AsynDealer();
    void OnMessage(talk_base::Message *msg);
    int initialize(const std::string & id,const std::string & router);
    void terminate();
    int send(const std::string & addr,const std::string & data);
    void AsynSend(const std::string & addr,const std::string & data);
    std::string id(){return id_;}
    sigslot::signal2<const std::string &,const std::string &> SignalReadData;
    sigslot::signal0<> SignalSent;
protected:
    int initialize_z(const std::string & id,const std::string & router);
    void terminate_z();
    int send_z(const std::string & addr,const std::string & data);
    int recv_z();
private:
    talk_base::Thread * zmq_thread_;
    bool owns_ptrs_;
    bool beInit;
    zmq::context_t *context_;
    zmq::socket_t *socket_;
    std::string id_;
    std::string router_;
};

#endif // ASYNDEALER_H
