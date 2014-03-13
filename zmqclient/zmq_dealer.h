#ifndef ZMQ_DEALER_H
#define ZMQ_DEALER_H
#include <QThread>

//typedef void (WINAPI * FCallReturnFileList)(int iRes,  int iFileCount, void* pContext);

#define MAX_RecvSize 1024*1024*2
struct sSendData
{
    char iUse;
    //char *bufSendTo;
    //char *bufSendData;
    QByteArray bufSendTo;
    QByteArray bufSendData;
};

class ZMQ_Dealer : public QThread
{
    Q_OBJECT
public:

    void ZMQ_init(int itype, char* name); //1type 1 client 0 worker

    void ZMQ_SendData(QByteArray sSendAd, QByteArray sSendBuf);

public:
    bool isthreadrun;
    ZMQ_Dealer();
    ~ZMQ_Dealer();
    void sendSignalToB(QByteArray sRevFrom, QByteArray sRevData);

signals:
    void toB(QByteArray sRevFrom, QByteArray sRevData);//1-name 2-data

protected:
    void run();
};

#endif // ZMQ_DEALER_H
