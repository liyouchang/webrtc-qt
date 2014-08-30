#ifndef KEPLAYERPLUGIN_H
#define KEPLAYERPLUGIN_H

#include <QWidget>
#include <QQueue>
#include <QString>
#include "zmqclient/peerconnectionclientdealer.h"
#include "KeQtTunnelClient.h"
#include "libjingle_app/kelocalclient.h"


#ifdef QAXSERVER
#include <QAxBindable>
#include "ObjectSafetyImpl.h"
#endif

#ifdef NPAPI
#include "qtbrowserplugin.h"
#endif

QT_BEGIN_NAMESPACE
class QSettings;

QT_END_NAMESPACE


class VideoWall;
struct RecordFileInfo;
class KeRecorder;

enum KE_ERROR{
    KE_SUCCESS = 0,
    KE_FAILED = 10000,
    KE_PARAM_ERROR = 10001,
    KE_TUNNEL_NOT_OPEN = 10002,
    KE_VIDEO_CUT_FAILED = 10003,
    KE_VIDEO_START_FAILED = 10004,
    KE_CAPTURE_FAILED = 10005,
    KE_OTHER_ERROR
};

class KePlayerPlugin : public QWidget
        #ifdef QAXSERVER
        , public QAxBindable
        #endif
        #ifdef NPAPI
        , public QtNPBindable
        #endif

{
    Q_OBJECT
    Q_PROPERTY( QString m_savePath READ savePath WRITE setSavePath )

    Q_CLASSINFO("ClassID","{806873E8-893C-402D-8129-34951563272A}")
    Q_CLASSINFO("InterfaceID", "{63314D1F-5592-4166-87F0-655F2C8DBFF8}")
    Q_CLASSINFO("EventsID", "{45133EE9-DFB6-49FE-9D91-434ECCF96A49}")

    Q_CLASSINFO("MIME", "application/keplayer-plugin:kpp:Kaer player plugin")
    Q_CLASSINFO("ToSuperClass", "KePlayerPlugin")

public:
    KePlayerPlugin(QWidget *parent = 0);
    ~KePlayerPlugin();

#ifdef QAXSERVER
    // QAxBindable interface
    QAxAggregated *createAggregate(){
        return new ObjectSafetyImpl;
    }
#endif

signals:
    void TunnelOpened(const QString &);
    void TunnelClosed(const QString &);
    void RecordStatus(const QString &,int,int,int );
    void RecvPeerMsg(const QString &,const QString &);
    void RemoteFileDownloadEnd(QString peer_id);
    void LocalDeviceInfo(const QString & devInfo);

public slots:
    void DestroyAll();
    //change the version when you need to auto update
    int GetVersion();
    void about();
    void SetDivision(int num);
    int PlayLocalFile();
    QString GetLocalPath(void);
    void FullScreen();

    //int GetLocalPlayPos();
    //void SetLocalPlayPos();

    int Initialize(QString routerUrl, QString jstrIceServers);
    int OpenTunnel(QString peer_id);
    int CloseTunnel(QString peer_id);
    bool IsTunnelOpened(QString peer_id);
    int StartVideo(QString peer_id,int video);
    int StopVideo(QString peerId);
    QString StartCut(QString peerId);
    int StopCut(QString peerId);

    QString Capture(QString peerId);
    bool OpenSound(QString peerId);
    bool CloseSound(QString peerId);

    bool StartTalk();
    bool StopTalk();

    int SendCommand(QString peer_id,QString msg);

    bool PlayRecordFile(QString peer_id, QString remoteFile);
    bool StopPlayFile(QString peer_id);
    bool SetPlayPosition(QString peer_id,int pos);
    void setSavePath(const QString &path);
    QString savePath() const;

    void OnRecordStatus(QString peer_id,int status);
    //saveType: 在保存路径(savePath)中的保存类型如 "CaptureFiles","RecordFiles"
    QString GetSaveDirList(QString saveType);
    //dateDir:需要"saveType/dateDir",如: "CaptureFiles/2014-05-20"
    QString GetSaveFileList(QString dateDir);
    //fileName:需要文件相对于savePath的路径,
    //如"CaptureFiles/2014-05-20/0090B01AF67F_2014-05-20_13-36-37-012.bmp"
    //width,height : 文件扩展大小,0时返回原始图片
    QString GetSaveFileData(QString fileName,int scaleWidth,int scaleHeight);

    int SearchLocalDevice();
    int OpenLocalDevice(QString peerAddr);
    int CloseLocalDevice(QString peerAddr);
    int StartLocalVideo(QString peerAddr);
    int StopLocalVideo(QString peerId);

private:
    static QString GetTimeFileName(QString peerId,QString extName,QString path);
    QQueue<RecordFileInfo> need_play_records_;
    VideoWall * video_wall_;
    KeQtTunnelClient * tunnel_;
    PeerConnectionClientDealer * connection_;
    KeQtLocalClient * localClient_;
    QString m_savePath;
    bool is_inited;
    QSettings *myconfig;


    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
};

#endif // KEPLAYERPLUGIN_H
