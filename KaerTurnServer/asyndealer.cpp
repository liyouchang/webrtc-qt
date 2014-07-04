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

AsynDealer::AsynDealer(talk_base::Thread * thread):
    context_(NULL),socket_(NULL),ownThread(false)
{
    if(thread){
        zmq_thread_ = thread;
    }else{
        zmq_thread_ = new talk_base::Thread();
        zmq_thread_->Start();
        ownThread = true;
    }
}

AsynDealer::~AsynDealer()
{
    zmq_thread_->Clear(this);
    zmq_thread_->Invoke<void>(
                talk_base::Bind(&AsynDealer::terminate_z, this));

    if(ownThread){
        delete zmq_thread_;
    }
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
    zmq::zmsg * pzmsg = new zmq::zmsg() ;
    pzmsg->wrap(addr,"");
    pzmsg->append(data);
    PostZmqMessage * msgData = new  PostZmqMessage(pzmsg);
    zmq_thread_->Post(this,MSG_TOSEND,msgData);
}

bool AsynDealer::SendRecv(const std::string &sendAddr, const std::string &sendData,
                          std::string *recvData, int timeout)
{
    return zmq_thread_->Invoke<bool>(
                talk_base::Bind(&AsynDealer::SendRecv_z, this,sendAddr,sendData,
                                recvData,timeout));
}

bool AsynDealer::initialize_z(const std::string &id, const std::string &router)
{
    ASSERT(zmq_thread_->IsCurrent());
    try{
        context_ = new zmq::context_t();
        socket_ = new zmq::socket_t(*context_,ZMQ_DEALER);
        if(id.empty()){
            id_ = s_set_id(*socket_);
        }
        else{
            socket_->setsockopt(ZMQ_IDENTITY,id.c_str(),id.length());
            id_ = id;
        }
        LOG(INFO) << "dealer id is "<<id_ <<" connect to "<<router;
        socket_->connect(router.c_str());
        this->router_ = router;
        zmq_thread_->PostDelayed(10,this,MSG_TOREAD);
    }catch(zmq::error_t e){
        LOG(WARNING) << "AsynDealer::initialize_z---" <<
                        "failed , enum:"<<e.num()<<" edes:" <<e.what();
        return false;
    }
    return true;
}

void AsynDealer::terminate_z()
{
    ASSERT(zmq_thread_->IsCurrent());
    if(socket_){
        socket_->disconnect(router_.c_str());
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
    //std::cout<<"send :"<<msg.GetBody()<<" data:"<<data<<std::endl;
    msg.send(*socket_);
    return true;
}


void AsynDealer::recv_z()
{
    ASSERT(zmq_thread_->IsCurrent());
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

bool AsynDealer::SendRecv_z(const std::string &sendAddr, const std::string &sendData,
                            std::string *recvData, int timeout)
{

    ASSERT(zmq_thread_->IsCurrent());
    send_z(sendAddr,sendData);
    zmq_pollitem_t  items [] = {{*socket_,0,ZMQ_POLLIN,0}};
    zmq_poll(items,1,timeout);
    if(items[0].revents & ZMQ_POLLIN){
        zmq::zmsg msg;
        msg.recv(*socket_);
        std::string addr = msg.GetAddress();
        if(addr != sendAddr){
            return false;
        }
        std::string data = msg.GetBody();
        *recvData = data;
        return true;
    }

    return false;

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
        talk_base::scoped_ptr<PostZmqMessage>  pData(
                    static_cast <PostZmqMessage*>(msg->pdata));
        zmq::zmsg * pzmsg = pData->data().get();
        pzmsg->send(*socket_);
        SignalSent();
    }
        break;
    default:
        break;
    }
}
