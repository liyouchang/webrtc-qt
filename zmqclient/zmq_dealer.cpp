#include "zmq_dealer.h"
#include "zmq.h"
#include "zmq.hpp"
#include <QDebug>

void *ctx;
void *client;
QList <sSendData*>sendlist;
ZMQ_Dealer::ZMQ_Dealer()
{
}

ZMQ_Dealer::~ZMQ_Dealer()
{
    isthreadrun = false;
     zmq_ctx_destroy(ctx);
}

void ZMQ_Dealer::ZMQ_init(int itype, char* name)
{
    ctx = zmq_init(2);

    client = zmq_socket(ctx, ZMQ_DEALER);
    zmq_setsockopt(client, ZMQ_IDENTITY, name, strlen(name));
   // if(itype)
    {
        zmq_connect(client,  "tcp://192.168.40.191:5555");
    }
 //   else
    {
 //       zmq_connect(client,  "tcp://192.168.40.191:5556");
    }


}


void ZMQ_Dealer::ZMQ_SendData(QByteArray sSendAd, QByteArray sSendBuf)
{

    sSendData *sdata = new sSendData;
    sdata->bufSendTo = sSendAd;//qstrdup(sSendAd);
   // sdata->bufSendData = sSendBuf.toBase64();//qstrdup(sSendBuf);
    sdata->bufSendData = sSendBuf;
    sendlist.append(sdata);
    return;
}

void ZMQ_Dealer::sendSignalToB(QByteArray sRevFrom, QByteArray sRevData)
{
    emit toB(sRevFrom,sRevData);
}

void ZMQ_Dealer::run()
{
    zmq_pollitem_t items [] = { { client, 0, ZMQ_POLLIN, 0 } };


    isthreadrun = true;

    char bufFrom[1024];
    char *buffer = new char[MAX_RecvSize];
    while (isthreadrun) {

        zmq_poll (items, 1, 1);
        if (items [0].revents & ZMQ_POLLIN) {


            int size = zmq_recv(client, bufFrom, 1024, 0);
            if (size >= 1024) break;
            bufFrom[size] = 0;
            //printf("收到数据来自 %s\n", bufFrom);

            size = zmq_recv(client, buffer, MAX_RecvSize, 0);
            if (size >= MAX_RecvSize) break;
            buffer[size] = 0;
            //printf("收到数据内容 %s\n", buffer);
            QByteArray qbyteadd(bufFrom);
            QByteArray qbytebuf(buffer);     
            //sendSignalToB(qbyteadd,qbytebuf.fromBase64(qbytebuf));
            sendSignalToB(qbyteadd,qbytebuf);


        }
        if(!sendlist.isEmpty())
        {
            sSendData  *sData = sendlist.takeFirst();
            zmq_send(client, sData->bufSendTo, strlen(sData->bufSendTo), ZMQ_SNDMORE);
            zmq_send(client, sData->bufSendData, strlen(sData->bufSendData), 0);           
            delete sData;

             //delete list.takeFirst();
        }

    }
    delete buffer;
    qDebug()<<"break";
    zmq_close(client);    
}
