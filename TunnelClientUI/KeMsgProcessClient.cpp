#include "KeMsgProcessClient.h"
#include <QBuffer>
#include <QDebug>
KeMsgProcessClient::KeMsgProcessClient(QObject *parent) :
    QObject(parent)
{
    terminal_ = NULL;
}

void KeMsgProcessClient::SetTerminal(std::string peer_id, PeerTerminalInterface *t)
{
    this->peer_id_ = peer_id;
    msgRecv.clear();
    bufPos = 0;
    toRead = 0;
    terminal_  = t;
    terminal_->SignalTunnelMessage.connect(this,&KeMsgProcessClient::OnTunnelMessage);
}

int KeMsgProcessClient::AskVideo()
{
    QByteArray msgSend;
    int msgLen = sizeof(KEVideoServerReq);
    msgSend.resize(msgLen);
    KEVideoServerReq * pReqMsg;
    pReqMsg = (KEVideoServerReq *)msgSend.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = KEMSG_TYPE_VIDEOSERVER;
    pReqMsg->msgLength = msgLen;
    pReqMsg->clientID = 0;
    pReqMsg->channelNo = 1;
    pReqMsg->videoID = 0;
    pReqMsg->video = 0;
    pReqMsg->listen = 0;
    pReqMsg->talk = 0;
    pReqMsg->protocalType = 0;
    pReqMsg->transSvrIp = 0;

    return terminal_->SendByTunnel(this->peer_id_,msgSend.data(),msgSend.length());
}

void KeMsgProcessClient::OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg)
{
    ASSERT(this->peer_id_ == peer_id);
    ASSERT(msg.length() > 0);

    QByteArray allBytes(msg.data(),msg.length());
    this->ExtractMessage(allBytes);
}

void KeMsgProcessClient::ExtractMessage(QByteArray &allBytes)
{
    int nRead= 0;
    QBuffer buffer(&allBytes);
    buffer.open(QIODevice::ReadOnly);
    const int headLen = 10;
    while(!buffer.atEnd())
    {
        if (msgRecv.isEmpty())//上一个消息已经读取完成
        {
            nRead = buffer.read(headBuf+bufPos,headLen-bufPos);
            if(nRead < headLen-bufPos)//消息头在最后几个字节，记录读取的字节，下次继续读取。
            {
                qDebug()<<"Continue Read head in new package\r\n ";
                bufPos = nRead;
                break;
            }
            unsigned char  protocal = headBuf[0];
            int msgLen;
            memcpy(&msgLen,&headBuf[2],4);
            if (protocal != PROTOCOL_HEAD||  msgLen>msgMaxLen)
            {
                qWarning()<<"The message Protocal Head error"<<msgLen <<" protocal:"<<protocal;
                msgRecv.clear();
                break;
            }
            msgRecv.resize(msgLen);
            bufPos = 0;
            memcpy(msgRecv.data(),headBuf,headLen);
            bufPos += headLen;
            toRead =  msgLen-headLen;
            if (toRead != 0)//防止 headLen 越界
            {
                nRead = buffer.read(msgRecv.data()+bufPos, toRead);
                bufPos += nRead;
                toRead -= nRead;
            }
        }
        else//上一个消息未完成读取
        {
            nRead = buffer.read(msgRecv.data()+bufPos,toRead);
            if (nRead < toRead){
                qDebug()<<"to read more and more!";
            }
            bufPos += nRead;
            toRead -= nRead;
        }
        if(toRead == 0 && bufPos == msgRecv.size())//全部读取
        {
            this->OnMessageRespond(msgRecv);
            msgRecv.clear();
            bufPos = 0;
        }
    }
}

void KeMsgProcessClient::OnMessageRespond(QByteArray &msgData)
{
    unsigned char  msgType = msgData[1];
    switch(msgType)
    {
    case KEMSG_TYPE_VIDEOSERVER:
        break;
    case KEMSG_TYPE_MEDIATRANS:
        break;
    case KEMSG_TYPE_VIDEOSTREAM:
    case KEMSG_TYPE_AUDIOSTREAM:{
        KERTStreamHead * pMsg = (KERTStreamHead *)msgData.data();
        int cameraID = CreateCameraID(pMsg->videoID, pMsg->channelNo);
        const int sendStartPos = 11;
        QByteArray data = msgData.right(pMsg->msgLength-sendStartPos);
        emit this->SigRecvMediaData(cameraID,msgType,data);
        break;
    }
    default:
        qDebug("Receive unkown message: %d ",msgType);
        break;
    }
}
