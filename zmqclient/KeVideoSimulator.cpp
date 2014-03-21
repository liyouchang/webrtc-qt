#include "KeVideoSimulator.h"
#include "talk/base/pathutils.h"
KeVideoSimulator::KeVideoSimulator()
{
    file_thread_ = new talk_base::Thread();
    file_thread_->Start();

startSend = false;
}

KeVideoSimulator::~KeVideoSimulator()
{
    delete file_thread_;
}

bool KeVideoSimulator::ReadVideoData(std::string file_name)
{
    talk_base::Pathname path;
    bool result = talk_base::Filesystem::GetAppPathname(&path);
    if(!result){
        return false;
    }
    path.AppendPathname(file_name);

    LOG(INFO)<<"file name is "<<path.pathname();

    talk_base::FileStream * stream = talk_base::Filesystem::OpenFile(path,"r");
    if(stream == NULL){
        LOG(WARNING) << "open file error";
        return false;
    }
    size_t fileSize;
    result = stream->GetSize(&fileSize);
    if(!result){
        return false;
    }

    video_data_.SetLength(fileSize);
    size_t readData;
    int error;
    talk_base::StreamResult sr = stream->Read(video_data_.data(),fileSize,&readData,&error);
    if(sr != talk_base::SR_SUCCESS){
        return false;
    }
    return true;
}

void KeVideoSimulator::SetTerminal(std::string peer_id, PeerTerminalInterface *t)
{
    this->peer_id_ = peer_id;
    terminal_ = t;
    msgRecv.SetLength(0);
    bufPos = 0;
    toRead = 0;
    terminal_->SignalTunnelMessage.connect(this,&KeVideoSimulator::OnTunnelMessage);

    this->SignalRecvAskVideoMsg.connect(this,&KeVideoSimulator::OnSendVideo);
}

void KeVideoSimulator::OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg)
{
    ASSERT(this->peer_id_ == peer_id);
    ASSERT(msg.length() > 0);
    this->ExtractMessage(msg);
}

void KeVideoSimulator::OnTunnelOpend(PeerTerminalInterface *t, const std::string &peer_id)
{
    this->SetTerminal(peer_id,t);
}

void KeVideoSimulator::OnSendVideo()
{
    if(!startSend){
        file_thread_->Post(this,MSG_SENDFILEVIDEO);
        startSend = true;
    }
}

void KeVideoSimulator::ExtractMessage(talk_base::Buffer &allBytes)
{
    talk_base::MemoryStream buffer(allBytes.data(),allBytes.length());
    const int headLen = 10;
    talk_base::StreamResult result;
    size_t read_bytes;
    int error;
    while(true)
    {
        if (msgRecv.length() == 0)//上一个消息已经读取完成
        {
            result = buffer.Read(headBuf+bufPos,headLen-bufPos,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if(read_bytes < headLen-bufPos)//消息头在最后几个字节，记录读取的字节，下次继续读取。
            {
                LOG(INFO)<<"Continue Read head in new package\r\n ";
                bufPos = read_bytes;
                break;
            }
            unsigned char  protocal = headBuf[0];
            int msgLen = *((int*)&headBuf[2]);
            if (protocal != PROTOCOL_HEAD||  msgLen>msgMaxLen)
            {
                LOG(WARNING)<<"The message Protocal Head error, Clear the recv buffer!\r\n";
                msgRecv.SetLength(0);
                break;
            }
            msgRecv.SetLength(msgLen);
            bufPos = 0;
            memcpy(msgRecv.data(),headBuf,headLen);
            bufPos += headLen;
            toRead =  msgLen-headLen;
            if (toRead != 0)//防止 headLen 越界
            {
                result = buffer.Read(msgRecv.data()+bufPos,toRead,&read_bytes,&error);
                if(result == talk_base::SR_EOS){
                    break;
                }
                bufPos += read_bytes;
                toRead -= read_bytes;
            }
        }
        else//上一个消息未完成读取
        {
            result = buffer.Read(msgRecv.data()+bufPos,toRead,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if (read_bytes < toRead){
                LOG(INFO)<<"to read more and more!";
            }
            bufPos += read_bytes;
            toRead -= read_bytes;
        }
        if(toRead == 0 && bufPos == msgRecv.length())//全部读取
        {
            this->OnMessageRespond(msgRecv);
            msgRecv.SetLength(0);
            bufPos = 0;
        }
    }
}

void KeVideoSimulator::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType){
    case KEMSG_TYPE_VIDEOSERVER:
        //send stream
        LOG(INFO)<< __FUNCTION__<<"receive message video server msg";
        SignalRecvAskVideoMsg();
        break;
    }
}

void KeVideoSimulator::OnMessage(talk_base::Message *msg)
{
    if(msg->message_id = MSG_SENDFILEVIDEO){
        static int fileBufPos = 0;
        static int lastFrameNo = 0;
        if(fileBufPos > video_data_.length()){
            fileBufPos = 0;
        }
        KEFrameHead * pHead = (KEFrameHead *)(video_data_.data() + fileBufPos);
        int frameLen = pHead->frameLen + sizeof(KEFrameHead);

        terminal_->SendByTunnel(video_data_.data() + bufPos,frameLen);
        fileBufPos += frameLen;

        if(lastFrameNo == 0){
            lastFrameNo = pHead->frameNo;
        }

        if(startSend){
            if(lastFrameNo != pHead->frameNo){
                file_thread_->PostDelayed(40,this ,MSG_SENDFILEVIDEO);
            }
            else{
                file_thread_->Post(this,MSG_SENDFILEVIDEO);
            }
        }
    }
}
