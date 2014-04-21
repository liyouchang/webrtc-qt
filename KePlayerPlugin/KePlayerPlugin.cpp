#include "KePlayerPlugin.h"
#include <QtWidgets>
#include "VideoWall.h"

KePlayerPlugin::KePlayerPlugin(QWidget *parent)
    : QWidget(parent),
      connection_(new   PeerConnectionClientDealer()),
      tunnel_(new KeQtTunnelClient()),
      is_inited(false)
{
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin(0);
    videoWall = new VideoWall(this);
    vbox->addWidget( videoWall );
    m_savePath = QDir::currentPath();

    QObject::connect(videoWall,&VideoWall::SigNeedStopPeerPlay,
                     this,&KePlayerPlugin::StopVideo);
    QObject::connect(this,&KePlayerPlugin::TunnelClosed,
                     videoWall,&VideoWall::StopPeerPlay);

}

KePlayerPlugin::~KePlayerPlugin()
{

}

void KePlayerPlugin::about()
{
    QMessageBox::aboutQt(this);
}

void KePlayerPlugin::SetDivision(int num)
{
    this->videoWall->SetDivision(num);
}

QString KePlayerPlugin::PlayLocalFile()
{
    this->videoWall->PlayLocalFile();
    return "play success";
}

int KePlayerPlugin::Initialize(QString routerUrl)
{
    if(is_inited){
        return 10002;
    }
    //int ret = connection_->Connect("tcp://192.168.40.191:5555","");
    int ret = connection_->Connect(routerUrl.toStdString(),"");
    if(ret != 0){
        return ret;
    }
    tunnel_->Init(connection_.get());
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvVideoData,
                     this->videoWall,&VideoWall::OnRecvMediaData);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvAudioData,
                     this->videoWall,&VideoWall::OnRecvMediaData);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigTunnelOpened,
                     this,&KePlayerPlugin::TunnelOpened);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigTunnelClosed,
                     this,&KePlayerPlugin::TunnelClosed);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecordStatus,
                     this,&KePlayerPlugin::RecordStatus);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvPeerMsg,
                     this,&KePlayerPlugin::RecvPeerMsg);




    this->is_inited = true;
    return 0;
}

int KePlayerPlugin::StartVideo(QString peer_id)
{
    std::string str_id = peer_id.toStdString();
    int ret =  tunnel_->StartPeerMedia(str_id,true);
    if(ret != 0){
        return ret;
    }
    int index = videoWall->SetPeerPlay(peer_id);
    qDebug()<<"KePlayerPlugin::StartVideo play index is "<< index;


}

int KePlayerPlugin::StopVideo(QString peer_id)
{
    if(peer_id.isEmpty()){
        return 10001;
    }
    videoWall->StopPeerPlay(peer_id);
    std::string str_id = peer_id.toStdString();
    return tunnel_->StartPeerMedia(str_id,false);
}

int KePlayerPlugin::SendCommand(QString peer_id, QString msg)
{
    std::string str_id = peer_id.toStdString();
    std::string str_msg = msg.toStdString();
    return tunnel_->SendCommand(str_id,str_msg);
}

void KePlayerPlugin::setSavePath(const QString &path)
{
    this->m_savePath = path;
}

QString KePlayerPlugin::savePath() const
{
    return m_savePath;
}


int KePlayerPlugin::OpenTunnel(QString peer_id)
{
    std::string str_id = peer_id.toStdString();
    int ret = tunnel_->OpenTunnel(str_id);
    return ret;
}

int KePlayerPlugin::CloseTunnel(QString peer_id)
{
    std::string str_id = peer_id.toStdString();
    int ret = tunnel_->CloseTunnel(str_id);
    return ret;
}

