#include "KePlayerPlugin.h"

#include <QtWidgets>
#include <QDir>
#include <vector>

#include "talk/base/logging.h"

#include "libjingle_app/defaults.h"
#include "libjingle_app/p2pconductor.h"
#include "libjingle_app/peerterminal.h"

#include "zmqclient/peerconnectionclientdealer.h"

#include "VideoWall.h"
#include "ke_recorder.h"

#ifdef WIN32
#include <windows.h>
#include <Shlobj.h>

#ifdef UNICODE

#define QStringToTCHAR(x)     (wchar_t*) x.utf16()

#define PQStringToTCHAR(x)    (wchar_t*) x->utf16()

#define TCHARToQString(x)     QString::fromUtf16((const ushort * )(x))

#define TCHARToQStringN(x,y)  QString::fromUtf16((const ushort * )(x),(y))

#else

#define QStringToTCHAR(x)     x.local8Bit().constData()

#define PQStringToTCHAR(x)    x->local8Bit().constData()

#define TCHARToQString(x)     QString::fromLocal8Bit((x))

#define TCHARToQStringN(x,y)  QString::fromLocal8Bit((x),(y))

#endif

#endif
KePlayerPlugin::KePlayerPlugin(QWidget *parent)
    : QWidget(parent),
      connection_(NULL),tunnel_(NULL),localClient_(NULL),
      is_inited(false)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    video_wall_ = new VideoWall(this);
    vbox->addWidget(video_wall_);

    QObject::connect(video_wall_,&VideoWall::SigNeedStopPeerPlay,
                     this,&KePlayerPlugin::StopVideo);
    QObject::connect(this,&KePlayerPlugin::TunnelClosed,
                     video_wall_,&VideoWall::StopPeerPlay);

    QObject::connect(video_wall_,&VideoWall::SigToNoramlScreen,
                     this,&KePlayerPlugin::ResizeToNormal);

    QString currentPath = QCoreApplication::applicationDirPath();
    QDir configDir(currentPath);
    myconfig = new QSettings(configDir.absoluteFilePath("pluginconfig.ini"),
                             QSettings::IniFormat,this);
    //set log info
    QString logFileName = myconfig->value("log/filename").toString();
    QString logConfig = myconfig->value("log/config").toString();
    QString logFilePath = configDir.absoluteFilePath(logFileName);
    qDebug()<<"log info "<<logConfig<<" logFilePath "<<logFilePath;
    talk_base::LogMessage::ConfigureLogging(logConfig.toLatin1().constData(),
                                            logFilePath.toLocal8Bit().constData());

    //set save path
    m_savePath = myconfig->value("plugin/save_path").toString();
    if ( savePath().isEmpty() ) {
        QDir saveDir = QDir::home();
        saveDir.mkdir("ShijietongData");
        saveDir.cd("ShijietongData");
        this->setSavePath( saveDir.absolutePath() );
    }

}

KePlayerPlugin::~KePlayerPlugin()
{
    qDebug()<<"KePlayerPlugin::~KePlayerPlugin";
    DestroyAll();
}

void KePlayerPlugin::DestroyAll()
{
    qDebug()<<"KePlayerPlugin::DestroyAll";
    if(tunnel_){
        delete tunnel_;
        tunnel_ = NULL;
    }
    if(connection_){
        delete connection_;
        connection_ = NULL;
    }
    if(localClient_){
        delete localClient_;
        localClient_ = NULL;
    }
    if(video_wall_){
        delete video_wall_;
        video_wall_ = NULL;
    }
    if(myconfig){
        delete myconfig;
        myconfig = NULL;
    }
}

void KePlayerPlugin::about()
{
    QMessageBox::aboutQt(this);
}

void KePlayerPlugin::SetDivision(int num)
{
    this->video_wall_->SetDivision(num);
}

int KePlayerPlugin::PlayLocalFile()
{
    QString filename = QFileDialog::getOpenFileName(
                this,"Open Video File",savePath(),
                "Video files (*.h264 *.264);;All files(*.*)");
    if (!filename.isNull()) {
        qDebug()<<QDir::currentPath();
        this->video_wall_->PlayLocalFile("",filename,0);
    }
    return 0;
}

QString KePlayerPlugin::GetLocalPath()
{
    QString ret;
#ifdef WIN32
    BROWSEINFO bInfo;
    ZeroMemory(&bInfo, sizeof(bInfo));
    bInfo.hwndOwner = (HWND)this->effectiveWinId();
    bInfo.lpszTitle = TEXT("choose dir:");
    bInfo.ulFlags = BIF_RETURNONLYFSDIRS;
    LPITEMIDLIST lpDlist; //用来保存返回信息的IDList
    lpDlist = SHBrowseForFolder(&bInfo); //显示选择对话框
    if(lpDlist != NULL)  //用户按了确定按钮
    {
        TCHAR chPath[255]; //用来存储路径的字符串
        SHGetPathFromIDList(lpDlist, chPath);//把项目标识列表转化成字符串
        ret = TCHARToQString(chPath);
    }
#else
    ret = QFileDialog::getExistingDirectory(
                this,"选择路径",savePath(),
                QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks|QFileDialog::ReadOnly);
#endif
    qDebug() << " get local path " << ret;
    return ret;
}

void KePlayerPlugin::FullScreen()
{
    this->video_wall_->showfullScreenWall();
//    qDebug()<<"resize to normal";
//    emit this->ResizeToNormal();
}

int KePlayerPlugin::GetVersion()
{
    const int kVersion = 57;
    return kVersion;
}
/**
 * @brief KePlayerPlugin::Initialize
 * @param routerUrl : a url to connect to message server(eg. tcp://222.174.213.185:5555)
 * @param iceServers : the servers used by ice protocal to accomplish NAT traversal,
 *  this param is a json string contains a array of json object of server connection info
 *  (eg. "[{"uri":"stun:222.174.213.185:5389"},{"uri":"turn:222.174.213.185:5766"}]").
 * @return : 0 for success.
 */
int KePlayerPlugin::Initialize(QString routerUrl, QString jstrIceServers)
{
    qDebug()<<"KePlayerPlugin::Initialize---routerUrl:"<<routerUrl<<
              " iceServers"<<jstrIceServers;
    LOG(INFO)<<"KePlayerPlugin version is "<<kaerp2p::ToStringVersion(this->GetVersion());

    //init ice servers
    std::string servers = jstrIceServers.toStdString();
    kaerp2p::P2PConductor::AddIceServers(servers);

    if(is_inited){
        qDebug()<<"KePlayerPlugin::Initialize---alreay init";
        return KE_SUCCESS;
    }
    connection_ = new PeerConnectionClientDealer();

    if(!connection_->Connect(routerUrl.toStdString(),"")){
        qWarning()<<"KePlayerPlugin::Initialize---connect error";
        return KE_FAILED;
    }

    kaerp2p::PeerTerminal * peerTerminal = new kaerp2p::PeerTerminal(connection_);

    tunnel_ = new KeQtTunnelClient(this);
    tunnel_->Init(peerTerminal);
    QObject::connect(tunnel_,&KeQtTunnelClient::SigRecvVideoData,
                     this->video_wall_,&VideoWall::OnRecvMediaData);
    QObject::connect(tunnel_,&KeQtTunnelClient::SigRecvAudioData,
                     this->video_wall_,&VideoWall::OnRecvMediaData);
    QObject::connect(tunnel_,&KeQtTunnelClient::SigTunnelOpened,
                     this,&KePlayerPlugin::TunnelOpened);
    QObject::connect(tunnel_,&KeQtTunnelClient::SigTunnelClosed,
                     this,&KePlayerPlugin::TunnelClosed);
    QObject::connect(tunnel_,&KeQtTunnelClient::SigRecordStatus,
                     this,&KePlayerPlugin::RecordStatus);
    QObject::connect(tunnel_,&KeQtTunnelClient::SigRecvPeerMsg,
                     this,&KePlayerPlugin::RecvPeerMsg);
    QObject::connect(this->video_wall_,&VideoWall::SigTalkData,
                     this->tunnel_,&KeQtTunnelClient::OnTalkData);
    this->is_inited = true;
    localClient_ = new KeQtLocalClient(this);
    kaerp2p::LocalTerminal * local = new kaerp2p::LocalTerminal();
    local->Initialize();
    localClient_->Init(local);
    QObject::connect(localClient_,&KeQtLocalClient::SigRecvVideoData,
                     this->video_wall_,&VideoWall::OnRecvMediaData);
    QObject::connect(localClient_,&KeQtLocalClient::SigRecvAudioData,
                     this->video_wall_,&VideoWall::OnRecvMediaData);
    QObject::connect(localClient_,&KeQtLocalClient::SigTunnelOpened,
                     this,&KePlayerPlugin::TunnelOpened);
    QObject::connect(localClient_,&KeQtLocalClient::SigTunnelClosed,
                     this,&KePlayerPlugin::TunnelClosed);
    QObject::connect(localClient_,&KeQtLocalClient::SigSearchedDeviceInfo,
                     this,&KePlayerPlugin::LocalDeviceInfo);
    return KE_SUCCESS;
}


//video:1 main stream 2 sub stream
int KePlayerPlugin::StartVideo(QString peer_id, int video)
{
    std::string str_id = peer_id.toStdString();
    if(!tunnel_->StartPeerMedia(str_id,video)){
        qWarning()<<"KePlayerPlugin::StartVideo---"<<"start video failed";
        return KE_VIDEO_START_FAILED;
    }
    int index = video_wall_->SetPeerPlay(peer_id);
    qDebug()<<"KePlayerPlugin::StartVideo play index is "<<index;
    return KE_SUCCESS;
}

int KePlayerPlugin::StopVideo(QString peerId)
{
    if(peerId.isEmpty()){
        return KE_PARAM_ERROR;
    }
    video_wall_->StopPeerPlay(peerId);
    std::string strId = peerId.toStdString();
    if(!tunnel_->StopPeerMedia(strId)){
        return KE_FAILED;
    }
    return KE_SUCCESS;
}

QString KePlayerPlugin::StartCut(QString peerId)
{
    bool result = true;
    QString info;
    do{
        if(peerId.isEmpty()){
            result = false;
            break;
        }
        std::string strId = peerId.toStdString();
        if(!tunnel_->IsTunnelOpened(strId)){
            result = false;
            break;
        }
        QDir fileDir(savePath());
        fileDir.mkdir("RecordFiles");
        fileDir.cd("RecordFiles");
        QString fileName = KePlayerPlugin::GetTimeFileName(peerId,"avi",fileDir.path());
        info = fileName;
        if(!tunnel_->StartPeerVideoCut(strId,fileName.toLocal8Bit().constData())){
            result = false;
            break;
        }
    }while(false);

    QJsonObject jreturn;
    jreturn.insert("result",result);
    jreturn.insert("info",info);
    QJsonDocument jsondoc(jreturn);
    return jsondoc.toJson();
}

int KePlayerPlugin::StopCut(QString peerId)
{
    if (peerId.isEmpty()) {
        return KE_PARAM_ERROR;
    }
    std::string strId = peerId.toStdString();
    if (!tunnel_->StopPeerVideoCut(strId)) {
        return KE_VIDEO_CUT_FAILED;
    }
    return KE_SUCCESS;
}


QString KePlayerPlugin::Capture(QString peerId)
{
    bool result = true;
    QString info;
    if(peerId.isEmpty()){
        result = false;
    }else{
        QDir fileDir(savePath());
        fileDir.mkdir("CaptureFiles");
        fileDir.cd("CaptureFiles");
        QString fileName = KePlayerPlugin::GetTimeFileName(peerId,"bmp",fileDir.path());
        info = fileName;
        bool ret = video_wall_->Capture(peerId,fileName);
        if(!ret){
            result = false;
        }
    }
    QJsonObject jreturn;
    jreturn.insert("result",result);
    jreturn.insert("info",info);
    QJsonDocument jsondoc(jreturn);
    return jsondoc.toJson();
}

bool KePlayerPlugin::OpenSound(QString peerId)
{
    if(peerId.isEmpty()){
        return false;
    }
    return video_wall_->OpenSound(peerId);
}

bool KePlayerPlugin::CloseSound(QString peerId)
{
    if(peerId.isEmpty()){
        return false;
    }
    return video_wall_->CloseSound(peerId);
}

bool KePlayerPlugin::StartTalk()
{
    return video_wall_->StartTalk();
}

bool KePlayerPlugin::StopTalk()
{
    return video_wall_->StopTalk();
}

int KePlayerPlugin::SendCommand(QString peer_id, QString msg)
{
    std::string str_id = peer_id.toStdString();
    std::string str_msg = msg.toStdString();
    return tunnel_->SendCommand(str_id,str_msg);
}

bool KePlayerPlugin::PlayRecordFile(QString peerId, QString remoteFile)
{
    std::string strId = peerId.toStdString();
    std::string remoteFileName = remoteFile.toStdString();
    qDebug()<<"fileName-"<<remoteFileName.c_str();
    bool result = tunnel_->PlayRemoteFile(strId,remoteFileName);
    if(!result){
        qWarning() << "tunnel DownloadRemoteFile error ";
        return false;
    }
    int index = video_wall_->SetPeerPlay(peerId);
    qDebug()<<"KePlayerPlugin::PlayRecordFile play index is " << index <<" peer id is "<<peerId;
    return true;
}

bool KePlayerPlugin::StopPlayFile(QString peer_id)
{

    if(peer_id.isEmpty()){
        return false;
    }
    video_wall_->StopPeerPlay(peer_id);

    std::string strId = peer_id.toStdString();

    if(!tunnel_->SetPlayFileStatus(strId,3,-1,-1)){
        qWarning() << "tunnel StopPlayFile error ";
        return false;
    }
    this->video_wall_->StopPeerPlay(peer_id);
    return 0;
}

bool KePlayerPlugin::SetPlayPosition(QString peer_id,int pos)
{
    std::string strId = peer_id.toStdString();
    qDebug()<<"KePlayerPlugin::SetPlayPosition  "<< pos;
    return tunnel_->SetPlayFileStatus(strId,2,pos,-1);
}

void KePlayerPlugin::setSavePath(const QString &path)
{
    qDebug()<<"KePlayerPlugin::setSavePath---path:"<<path;
    if(path.isEmpty()){
        return;
    }
    this->m_savePath = path;
    myconfig->setValue("plugin/save_path",savePath());
}

void KePlayerPlugin::OnRecordStatus(QString peer_id, int status)
{
    //    std::string str_id = peer_id.toStdString();
    //    if(status == 6){//download end , download another file
    //        if(!need_play_records_.empty()){
    //            RecordFileInfo file = need_play_records_.dequeue();
    //            ASSERT(str_id == file.peer_id);
    //            tunnel_->DownloadRemoteFile(str_id,need_play_records_.at(0).remote_name);

    //            KeRecorder * recorder = this->findChild<KeRecorder *>(peer_id);
    //            if(recorder){
    //                this->video_wall_->SetLocalPlayFileSize(peer_id,recorder->GetFileSize());
    //            }
    //        }else{ //download end
    //            emit RemoteFileDownloadEnd(peer_id);
    //        }
    //    }
}

QString KePlayerPlugin::GetSaveDirList(QString saveType)
{
    bool result = true;
    QJsonArray jdateDirArray;

    QDir fileDir(savePath());
    if(!fileDir.cd(saveType)){
        result = false;
    }else{
        QStringList dirList = fileDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
        jdateDirArray = QJsonArray::fromStringList(dirList);
    }
    QJsonObject jreturn;
    jreturn.insert("result",result);
    jreturn.insert("dirArray",jdateDirArray);
    QJsonDocument jsondoc(jreturn);
    return jsondoc.toJson();
}

QString KePlayerPlugin::GetSaveFileList(QString dateDir)
{
    bool result = true;
    QJsonArray jfilesArray;
    QDir fileDir(savePath());
    if(!fileDir.cd(dateDir)){
        result = false;
    }else{
        QStringList list = fileDir.entryList(QDir::Files);
        jfilesArray = QJsonArray::fromStringList(list);
    }
    QJsonObject jreturn;
    jreturn.insert("result",result);
    jreturn.insert("files",jfilesArray);
    QJsonDocument jsondoc(jreturn);
    return jsondoc.toJson();
}

QString KePlayerPlugin::GetSaveFileData(QString fileName,int scaleWidth,int scaleHeight)
{
    bool result = true;
    QString imageDataStr = "data:image/jpg;base64,";
    QDir fileDir(savePath());
    QString  filePath = fileDir.absoluteFilePath(fileName);
    QImage imageFile(filePath);
    if(imageFile.isNull()){
        result = false;
    }else{
        QImage scaledImage = imageFile.scaled(scaleWidth,scaleHeight);
        if(scaledImage.isNull()){
            scaledImage = imageFile;
        }
        QByteArray fileRawData;
        QBuffer buffer(&fileRawData);
        buffer.open(QIODevice::WriteOnly);
        scaledImage.save(&buffer, "JPG"); // writes image into fileRawData in PNG format
        imageDataStr += fileRawData.toBase64(
                    QByteArray::Base64Encoding|QByteArray::KeepTrailingEquals);
    }
    QJsonObject jreturn;
    jreturn.insert("result",result);
    jreturn.insert("dataImage",imageDataStr);
    QJsonDocument jsondoc(jreturn);
    return jsondoc.toJson();
}

int KePlayerPlugin::SearchLocalDevice()
{
    localClient_->SearchLocalDevice();
    return 0;
}

int KePlayerPlugin::OpenLocalDevice(QString peerAddr)
{
    std::string strId = peerAddr.toStdString();
    if(! localClient_->OpenTunnel(strId)){
        return KE_FAILED;
    }
    return KE_SUCCESS;
}

int KePlayerPlugin::CloseLocalDevice(QString peerAddr)
{
    std::string strId = peerAddr.toStdString();
    if(!localClient_->CloseTunnel(strId)){
        return KE_FAILED;
    }
    return KE_SUCCESS;
}

int KePlayerPlugin::StartLocalVideo(QString peerAddr)
{
    if(peerAddr.isEmpty()){
        return KE_PARAM_ERROR;
    }
    std::string strAddr = peerAddr.toStdString();
    if(!localClient_->StartPeerMedia(strAddr)){
        qWarning()<<"KePlayerPlugin::StartVideo---"<<"start video failed";
        return KE_VIDEO_START_FAILED;
    }
    int index = video_wall_->SetPeerPlay(peerAddr);
    qDebug()<<"KePlayerPlugin::StartVideo play index is " << index;
    return KE_SUCCESS;
}

int KePlayerPlugin::StopLocalVideo(QString peerAddr)
{
    if(peerAddr.isEmpty()){
        return KE_PARAM_ERROR;
    }
    video_wall_->StopPeerPlay(peerAddr);
    std::string strId = peerAddr.toStdString();
    if(!localClient_->StopPeerMedia(strId)){
        return KE_FAILED;
    }
    return KE_SUCCESS;
}

QString KePlayerPlugin::GetTimeFileName(QString peerId, QString extName, QString path)
{
    QString dateStr = QDate::currentDate().toString("yyyy-MM-dd");
    QString timeStr = QTime::currentTime().toString("hh-mm-ss-zzz");
    QDir fileDir(path);
    if(!fileDir.cd(dateStr)) {
        fileDir.mkdir(dateStr);
        fileDir.cd(dateStr);
    }
    QString fileName = QString("%1_%2_%3.%4").
            arg(peerId.left(12)).arg(dateStr).arg(timeStr).arg(extName);
    return fileDir.filePath(fileName);
}

//void KePlayerPlugin::paintEvent(QPaintEvent *e)
//{
////    e->ignore();
//    qDebug()<<"KePlayerPlugin::paintEvent "<<e->rect();
//}

//void KePlayerPlugin::resizeEvent(QResizeEvent *e)
//{
//    qDebug()<<"KePlayerPlugin::resizeEvent --- old size "<<e->oldSize()<<
//              "  new size "<<e->size();
//}

QString KePlayerPlugin::savePath() const
{
    return m_savePath;
}

int KePlayerPlugin::OpenTunnel(QString peerId)
{
    std::string strId = peerId.toStdString();
    if(! tunnel_->OpenTunnel(strId)){
        return KE_FAILED;
    }
    return KE_SUCCESS;
}

int KePlayerPlugin::CloseTunnel(QString peer_id)
{
    std::string strId = peer_id.toStdString();
    if(!tunnel_->CloseTunnel(strId)){
        return KE_FAILED;
    }
    return KE_SUCCESS;
}

bool KePlayerPlugin::IsTunnelOpened(QString peer_id)
{
    if(peer_id.isEmpty()){
        return false;
    }
    std::string strId = peer_id.toStdString();
    return tunnel_->IsTunnelOpened(strId);
}

