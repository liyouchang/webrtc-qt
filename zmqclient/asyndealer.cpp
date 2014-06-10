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

AsynDealer::AsynDealer()
{
    zmq_thread_ = new talk_base::Thread();
    zmq_thread_->Start();
    owns_ptrs_ = true;
    beInit = false;
}

AsynDealer::~AsynDealer()
{
    zmq_thread_->Clear(this);
    zmq_thread_->Invoke<void>(
                talk_base::Bind(&AsynDealer::terminate_z, this));
    if(owns_ptrs_){
        delete zmq_thread_;
    }
}

int AsynDealer::initialize(const std::string &id, const std::string &router)
{
    if(!zmq_thread_->started()){
        std::cout<<"zmq thread not start"<<std::endl;
        return -1;
    }
    return zmq_thread_->Invoke<int>(
                talk_base::Bind(&AsynDealer::initialize_z,this,id,router));
}

void AsynDealer::terminate()
{
    zmq_thread_->Invoke<void>(
                talk_base::Bind(&AsynDealer::terminate_z, this));
}

int AsynDealer::send(const std::string & addr, const std::string & data)
{
    //TODO:use SendMessage
    return zmq_thread_->Invoke<int>(
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

int AsynDealer::initialize_z(const std::string &id, const std::string &router)
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
        LOG(INFO) << "dealer id is "<<id_;
        socket_->connect(router.c_str());
        this->router_ = router;
        zmq_thread_->PostDelayed(10,this,MSG_TOREAD);

    }catch(zmq::error_t e){
        LOG(WARNING) << __FUNCTION__ << " error :" <<e.what();
        return e.num();
    }
    beInit = true;
    return 0;
}

void AsynDealer::terminate_z()
{
    ASSERT(zmq_thread_->IsCurrent());
    ASSERT(beInit);
    socket_->disconnect(router_.c_str());
    delete socket_;
    beInit = false;
    delete context_;
}

int AsynDealer::send_z(const std::string & addr,const std::string & data)
{
    ASSERT(zmq_thread_->IsCurrent());
    zmq::zmsg msg;
    msg.wrap(addr,"");
    msg.append(data);
    std::cout<<"send :"<<msg.GetBody()<<" data:"<<data<<std::endl;
    msg.send(*socket_);
    return 0;
}


int AsynDealer::recv_z()
{
    ASSERT(zmq_thread_->IsCurrent());
    zmq_pollitem_t  items [] = {{*socket_,0,ZMQ_POLLIN,0}};
    zmq_poll(items,1,10);
    if(items[0].revents & ZMQ_POLLIN){
        zmq::zmsg msg;
        msg.recv(*socket_);
        std::string addr = msg.GetAddress();
        std::string data = msg.GetBody();
        //LOG(INFO) <<"Client "<<this->id_ << " :Receive "<<addr<<data;

        SignalReadData(addr,data);
    }
    return 0;
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
