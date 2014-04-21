#include "KeQtTunnelClient.h"
#include <QBuffer>
#include <QDebug>
#include <QFile>
KeQtTunnelClient::KeQtTunnelClient(QObject *parent) :
    QObject(parent)
{
}

void KeQtTunnelClient::OnRecvAudioData(const std::string &peer_id, const char *data, int len)
{
    QByteArray mediaData(data,len);
    emit SigRecvAudioData(peer_id.c_str(),mediaData);
}

void KeQtTunnelClient::OnRecvVideoData(const std::string &peer_id, const char *data, int len)
{
    QByteArray mediaData(data,len);
    emit SigRecvVideoData(peer_id.c_str(),mediaData);
}

void KeQtTunnelClient::OnRecordFileData(const std::string &peer_id, const char *data, int len)
{
    QByteArray mediaData(data,len);
    emit SigRecordFileData(peer_id.c_str(),mediaData);

}

void KeQtTunnelClient::OnRecordStatus(const std::string &peer_id, int status)
{
    emit SigRecordStatus(peer_id.c_str(),status);
}

void KeQtTunnelClient::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    qDebug()<<"KeQtTunnelClient::OnTunnelOpened";
    KeTunnelClient::OnTunnelOpened(t,peer_id);
    emit SigTunnelOpened(peer_id.c_str());
}

void KeQtTunnelClient::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    qDebug()<<"KeQtTunnelClient::OnTunnelClosed";
    KeTunnelClient::OnTunnelClosed(t,peer_id);
    emit SigTunnelClosed(peer_id.c_str());
}

void KeQtTunnelClient::OnRouterMessage(const std::string &peer_id, const std::string &msg)
{
    emit SigRecvPeerMsg(peer_id.c_str(),msg.c_str());
}






//void KeQtTunnelClient::ExtractMessage(QByteArray &allBytes)
//{
//    int nRead= 0;
//    QBuffer buffer(&allBytes);
//    buffer.open(QIODevice::ReadOnly);
//    const int headLen = 10;
//    while(!buffer.atEnd())
//    {
//        if (msgRecv.isEmpty())//上一个消息已经读取完成
//        {
//            nRead = buffer.read(headBuf+bufPos,headLen-bufPos);
//            if(nRead < headLen-bufPos)//消息头在最后几个字节，记录读取的字节，下次继续读取。
//            {
//                qDebug()<<"Continue Read head in new package\r\n ";
//                bufPos = nRead;
//                break;
//            }
//            unsigned char  protocal = headBuf[0];
//            int msgLen;
//            memcpy(&msgLen,&headBuf[2],4);
//            if (protocal != PROTOCOL_HEAD||  msgLen>msgMaxLen)
//            {
//                qWarning()<<"The message Protocal Head error"<<msgLen <<" protocal:"<<protocal;
//                msgRecv.clear();
//                break;
//            }
//            msgRecv.resize(msgLen);
//            bufPos = 0;
//            memcpy(msgRecv.data(),headBuf,headLen);
//            bufPos += headLen;
//            toRead =  msgLen-headLen;
//            if (toRead != 0)//防止 headLen 越界
//            {
//                nRead = buffer.read(msgRecv.data()+bufPos, toRead);
//                bufPos += nRead;
//                toRead -= nRead;
//            }
//        }
//        else//上一个消息未完成读取
//        {
//            nRead = buffer.read(msgRecv.data()+bufPos,toRead);
//            if (nRead < toRead){
//                qDebug()<<"to read more and more!";
//            }
//            bufPos += nRead;
//            toRead -= nRead;
//        }
//        if(toRead == 0 && bufPos == msgRecv.size())//全部读取
//        {
//            this->OnMessageRespond(msgRecv);
//            msgRecv.clear();
//            bufPos = 0;
//        }
//    }
//}

//void KeQtTunnelClient::OnMessageRespond(QByteArray &msgData)
//{
//    unsigned char  msgType = msgData[1];
//    switch(msgType)
//    {
//    case KEMSG_TYPE_VIDEOSERVER:
//        break;
//    case KEMSG_TYPE_MEDIATRANS:
//        break;
//    case KEMSG_TYPE_VIDEOSTREAM:
//    case KEMSG_TYPE_AUDIOSTREAM:{
//        KERTStreamHead * pMsg = (KERTStreamHead *)msgData.data();
//        int cameraID = CreateCameraID(pMsg->videoID, pMsg->channelNo);
//        const int sendStartPos = 11;
//        QByteArray data = msgData.right(pMsg->msgLength-sendStartPos);
//        emit this->SigRecvMediaData(cameraID,msgType,data);
//        break;
//    }
//    default:
//        qDebug("Receive unkown message: %d ",msgType);
//        break;
//    }
//}
