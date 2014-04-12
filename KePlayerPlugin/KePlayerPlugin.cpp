#include "KePlayerPlugin.h"

KePlayerPlugin::KePlayerPlugin(QWidget *parent)
    : QWidget(parent),
      connection_(new   PeerConnectionClientDealer()),
      tunnel_(new KeQtTunnelClient())
{
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin(0);
    videoWall = new VideoWall(this);
    vbox->addWidget( videoWall );

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
    qDebug("to emit closed");
    emit TunnelClosed("ddd");
    this->videoWall->SetDivision(num);
}

QString KePlayerPlugin::PlayLocalFile()
{
    //QMessageBox::aboutQt(this);
    this->videoWall->PlayLocalFile();
    return "play success";
}

int KePlayerPlugin::Initialize(QString routerUrl)
{
    int ret = connection_->Connect("tcp://192.168.40.191:5555","");
    if(ret != 0){
        return ret;
    }
    tunnel_->Initialize(connection_.get());
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvVideoData,
                     this->videoWall,&VideoWall::OnRecvMediaData);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvAudioData,
                     this->videoWall,&VideoWall::OnRecvMediaData);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigTunnelOpened,
                     this,&KePlayerPlugin::TunnelOpened);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigTunnelClosed,
                     this,&KePlayerPlugin::TunnelClosed);



    return 0;
}

int KePlayerPlugin::StartVideo(QString peer_id)
{
    std::string str_id = peer_id.toStdString();
    tunnel_->AskPeerVideo(str_id);
    return 0;
}

void KePlayerPlugin::paintEvent(QPaintEvent *)
{
    qDebug()<<"KePlayerPlugin::paintEvent";
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

