#include "KePlayerPlugin.h"

#include <QtWidgets>
#include <QDir>
#include <vector>

#include "libjingle_app/defaults.h"
#include "talk/base/logging.h"
#include "libjingle_app/p2pconductor.h"
#include "VideoWall.h"
#include "ke_recorder.h"
#include "libjingle_app/p2pconductor.h"

KePlayerPlugin::KePlayerPlugin(QWidget *parent)
    : QWidget(parent),
      connection_(new PeerConnectionClientDealer()),
      tunnel_(new KeQtTunnelClient()),
      is_inited(false)
{
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin(0);
    video_wall_ = new VideoWall(this);
    vbox->addWidget( video_wall_ );

    QObject::connect(video_wall_,&VideoWall::SigNeedStopPeerPlay,
                     this,&KePlayerPlugin::StopVideo);
    QObject::connect(this,&KePlayerPlugin::TunnelClosed,
                     video_wall_,&VideoWall::StopPeerPlay);

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
    if(savePath().isEmpty()){
        QDir saveDir = QDir::home();
        saveDir.mkdir("ShijietongData");
        saveDir.cd("ShijietongData");
        m_savePath = saveDir.absolutePath();
        myconfig->setValue("plugin/save_path",savePath());
    }

    localClient_ = new KeQtLocalClient(this);
}

KePlayerPlugin::~KePlayerPlugin()
{
    qDebug()<<"KePlayerPlugin::~KePlayerPlugin";
    myconfig->setValue("plugin/save_path",savePath());
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
    return QFileDialog::getExistingDirectory(
                this,"选择路径",savePath(),
                QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
}

void KePlayerPlugin::FullScreen()
{
    this->video_wall_->showfullScreenWall();
}

int KePlayerPlugin::GetVersion()
{
    const int kVersion = 32;
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

    if(is_inited){
        return KE_SUCCESS;
    }
    if(!connection_->Connect(routerUrl.toStdString(),"")){
        qWarning()<<"KePlayerPlugin::Initialize---connect error";
        return KE_FAILED;
    }


    tunnel_->Init(connection_.get());
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvVideoData,
                     this->video_wall_,&VideoWall::OnRecvMediaData);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvAudioData,
                     this->video_wall_,&VideoWall::OnRecvMediaData);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigTunnelOpened,
                     this,&KePlayerPlugin::TunnelOpened);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigTunnelClosed,
                     this,&KePlayerPlugin::TunnelClosed);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecordStatus,
                     this,&KePlayerPlugin::RecordStatus);
    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvPeerMsg,
                     this,&KePlayerPlugin::RecvPeerMsg);
    this->is_inited = true;


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

    //init ice servers
    std::string servers = jstrIceServers.toStdString();
    kaerp2p::P2PConductor::AddIceServers(servers);

    return 0;
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
    qDebug()<<"KePlayerPlugin::StartVideo play index is " << index;
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

int KePlayerPlugin::SendCommand(QString peer_id, QString msg)
{
    std::string str_id = peer_id.toStdString();
    std::string str_msg = msg.toStdString();
    return tunnel_->SendCommand(str_id,str_msg);
}

int KePlayerPlugin::PlayRecordFiles(QString peerId, QString jstrRecordArray)
{
    std::string strId = peerId.toStdString();

    QJsonParseError jparseerror;
    QJsonDocument jsondoc(
                QJsonDocument::fromJson(jstrRecordArray.toUtf8(),&jparseerror));
    if(jparseerror.error != QJsonParseError::NoError){
        return 10001;
    }
    QJsonArray jrecordArray = jsondoc.array();
    QJsonObject jrecordObj = jrecordArray.first().toObject();
    QString fileName = jrecordObj.value("fileName").toString();
    QString fileDate = jrecordObj.value("fileDate").toString();
    int fileSize = jrecordObj.value("fileSize").toInt();

    std::string remoteFileName = fileName.toStdString();
    qDebug()<<"fileName-"<<remoteFileName.c_str();
    QDir fileDir(savePath());
    fileDir.mkdir("RemoteFiles");
    fileDir.cd("RemoteFiles");
    QString saveFilePath = fileDir.filePath(QString("%1_%2.%3").
                                            arg(peerId.left(12)).arg(fileDate).arg("h264"));

    bool result = tunnel_->DownloadRemoteFile(
                strId,remoteFileName,saveFilePath.toLocal8Bit().constData(),
                fileSize/10);
    if(!result){
        qWarning() << "tunnel DownloadRemoteFile error ";
        return 10005;
    }
    /*
    need_play_records_.clear();
    std::vector<Json::Value>::iterator it = record_vector.begin();
    for(;it != record_vector.end();++it){
        RecordFileInfo file_info;
        file_info.peer_id = str_id;
        if(!GetIntFromJsonObject(*it,"file_size",&file_info.size)){
            qWarning() << "get file size error. " << record_info_list;
            return 10003;
        }
        if(!GetStringFromJsonObject(*it,"file_name",&file_info.remote_name)){
            qWarning() << "get file name error. " <<record_info_list;
            return 10003;
        }
        if(!GetStringFromJsonObject(*it,"file_date",&file_info.file_date)){
            qWarning() << "get file date error. " << record_info_list;
            return 10003;
        }
        need_play_records_.enqueue(file_info);
    }
    if(need_play_records_.empty()){

        return 10002;
    }
    RecordFileInfo first_record = need_play_records_.dequeue();
    bool result = tunnel_->DownloadRemoteFile(str_id,first_record.remote_name);
    if(!result){
        qWarning() << "tunnel DownloadRemoteFile error ";
        return 10005;
    }

    KeRecorder * recorder = new KeRecorder(this);
    recorder->setObjectName(peer_id);

    QObject::connect(recorder,&KeRecorder::SigAbleToPlay,
                     this->video_wall_,&VideoWall::PlayLocalFile);

    QObject::connect(this,&KePlayerPlugin::RemoteFileDownloadEnd,
                     recorder,&KeRecorder::OnRecordDownloadEnd);


    recorder->OpenFile(first_record,m_savePath);
*/
    return 0;
}

int KePlayerPlugin::StopPlayFile(QString peer_id)
{
    KeRecorder * recorder = this->findChild<KeRecorder*>(peer_id);
    if(recorder){
        delete recorder;
    }
    this->video_wall_->StopFilePlay(peer_id);
    return 0;
}

void KePlayerPlugin::setSavePath(const QString &path)
{
    qDebug()<<"KePlayerPlugin::setSavePath---path:"<<path;
    if(path.isEmpty()){
        return;
    }
    this->m_savePath = path;
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

