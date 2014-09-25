/*
 *
 * Copyright 2014 Kaer Electric Co.,Ltd
 * Listens see <http://www.gnu.org/licenses/>.
 * AUTHORS lht
 *
 * AsynDealer类实现了一个异步zmq dealer的通讯方法
 * 通过talk_base::Thread的PostMessage方法实现读取和发送的时序交替，
 * 通过Signal返回读取的数据
 */

#include "asyndealer.h"
#include "talk/base/bind.h"
#include "talk/base/logging.h"



typedef talk_base::ScopedMessageData<zmq::zmsg> PostZmqMessage;

AsynDealer::AsynDealer():context_(NULL),socket_(NULL)
{
    zmq_thread_ = new talk_base::Thread();
    zmq_thread_->Start();
}

AsynDealer::~AsynDealer()
{
    this->terminate();
    delete zmq_thread_;
}

bool AsynDealer::initialize(const std::string &id, const std::string &router)
{
    return zmq_thread_->Invoke<bool>(
                talk_base::Bind(&AsynDealer::initialize_z,this,id,router));
}

void AsynDealer::terminate()
{
    zmq_thread_->Invoke<void>(
                talk_base::Bind(&AsynDealer::terminate_z, this));
}

bool AsynDealer::send(const std::string & addr, const std::string & data)
{
    //TODO:use SendMessage
    return zmq_thread_->Invoke<bool>(
                talk_base::Bind(&AsynDealer::send_z, this,addr,data));
}

void AsynDealer::AsynSend(const std::string &addr, const std::string &data)
{
    zmq::zmsg * pzmsg = new zmq::zmsg();
    pzmsg->wrap(addr,"");
    pzmsg->append(data);
    PostZmqMessage * msgData = new  PostZmqMessage(pzmsg);
    zmq_thread_->Post(this,MSG_TOSEND,msgData);
}

bool AsynDealer::connect(const std::string &id, const std::string &router)
{
    return zmq_thread_->Invoke<bool>(
                talk_base::Bind(&AsynDealer::connect_z,this,id,router));
}

void AsynDealer::disconnect()
{
    zmq_thread_->Invoke<void>(
                talk_base::Bind(&AsynDealer::disconnect_z, this));
}

bool AsynDealer::initialize_z(const std::string &id, const std::string &router)
{
    ASSERT(zmq_thread_->IsCurrent());
    try{
//        context_ = new zmq::context_t(1,5);
        context_ = new zmq::context_t(1,2);
    }catch(zmq::error_t e){
        LOG_F(WARNING) <<" failed , enum:"<<e.num()<<" edes:" <<e.what();
        return false;
    }
    return connect_z(id,router);
}

void AsynDealer::terminate_z()
{
    disconnect_z();
    if(socket_){
        delete socket_;
        socket_ = NULL;
    }
    if(context_){
        delete context_;
        context_ = NULL;
    }

}

bool AsynDealer::send_z(const std::string & addr,const std::string & data)
{
    ASSERT(zmq_thread_->IsCurrent());
    zmq::zmsg msg;
    msg.wrap(addr,"");
    msg.append(data);
    msg.send(*socket_);
    return true;
}


void AsynDealer::recv_z()
{
    ASSERT(zmq_thread_->IsCurrent());
    if(!socket_){
        LOG_T_F(WARNING) << "socket null";
        return;
    }
    zmq_pollitem_t  items [] = {{*socket_,0,ZMQ_POLLIN,0}};
    zmq_poll(items,1,10);
    if(items[0].revents & ZMQ_POLLIN){
        zmq::zmsg msg;
        msg.recv(*socket_);
        std::string addr = msg.GetAddress();
        std::string data = msg.GetBody();
        SignalReadData(addr,data);
    }
}

bool AsynDealer::connect_z(const std::string & id,const std::string & router)
{
    ASSERT(zmq_thread_->IsCurrent());
    try{
        socket_ = new zmq::socket_t(*context_,ZMQ_DEALER);
        int reconnectInterval = 10000;
        socket_->setsockopt(ZMQ_RECONNECT_IVL,&reconnectInterval,sizeof(reconnectInterval));
        int highwater = 3;
        socket_->setsockopt(ZMQ_SNDHWM,&highwater,sizeof(highwater));
        socket_->setsockopt(ZMQ_RCVHWM,&highwater,sizeof(highwater));
        int ligger = 100;
        socket_->setsockopt(ZMQ_LINGER,&ligger,sizeof(ligger));

        if(id.empty()){
            id_ = s_set_id(*socket_);
        }
        else{
            socket_->setsockopt(ZMQ_IDENTITY,id.c_str(),id.length());
            id_ = id;
        }
        LOG_T_F(INFO) << "dealer id is "<<id_;
        socket_->connect(router.c_str());
        this->router_ = router;
        zmq_thread_->PostDelayed(10,this,MSG_TOREAD);
    }catch(zmq::error_t e){
        LOG_F(WARNING) <<"failed , enum:"<< e.num()<<" edes:" <<e.what();
        return false;
    }
    return true;
}

void AsynDealer::disconnect_z()
{
    zmq_thread_->Clear(this,MSG_TOREAD);
    zmq_thread_->Clear(this,MSG_TOSEND);
    if ( socket_ ) {
        socket_->disconnect(router_.c_str());
        delete socket_;
        socket_ = NULL;
    }
}

void AsynDealer::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_TOREAD:
        recv_z();
        //to receive more
        zmq_thread_->PostDelayed(10,this,MSG_TOREAD);
        break;
    case MSG_TOSEND:
    {
        PostZmqMessage *pData = static_cast <PostZmqMessage*>(msg->pdata);
        zmq::zmsg * pzmsg = pData->data().get();
        pzmsg->send(*socket_,true);
        SignalSent();
        delete pData;
    }
        break;
    default:
        break;
    }
}
