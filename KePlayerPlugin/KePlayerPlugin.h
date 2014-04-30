#ifndef KEPLAYERPLUGIN_H
#define KEPLAYERPLUGIN_H

#include <QWidget>
#include <QQueue>
#include <QString>
#include "zmqclient/peerconnectionclientdealer.h"
#include "KeQtTunnelClient.h"
//


class VideoWall;
struct RecordFileInfo;
class KeRecorder;

enum KE_ERROR{
    KE_SUCCESS = 0,
    KE_PARAM_ERROR = 10001,
    KE_TUNNEL_NOT_OPEN = 10002,
    KE_INIT_CUT_VIDEO_FAILED = 10003,
    KE_OTHER_ERROR
};

class KePlayerPlugin : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString savePath READ savePath WRITE setSavePath )

    Q_CLASSINFO("ClassID","{806873E8-893C-402D-8129-34951563272A}")
    Q_CLASSINFO("InterfaceID", "{63314D1F-5592-4166-87F0-655F2C8DBFF8}")
    Q_CLASSINFO("EventsID", "{45133EE9-DFB6-49FE-9D91-434ECCF96A49}")

    Q_CLASSINFO("MIME", "application/keplayer-plugin:kpp:Kaer player plugin")
    Q_CLASSINFO("ToSuperClass", "KePlayerPlugin")

public:
    KePlayerPlugin(QWidget *parent = 0);
    ~KePlayerPlugin();
    QString savePath() const;

signals:
    void TunnelOpened(const QString &);
    void TunnelClosed(const QString &);
    void RecordStatus(const QString &,int );
    void RecvPeerMsg(const QString &,const QString &);
    void RemoteFileDownloadEnd(QString peer_id);

public slots:
    void about();
    void SetDivision(int num);
    int PlayLocalFile();
    int Initialize(QString routerUrl);
    int OpenTunnel(QString peer_id);
    int CloseTunnel(QString peer_id);
    int StartVideo(QString peer_id);
    int StopVideo(QString peerId);
    int StartCut(QString peerId);
    int StopCut(QString peerId);

    int SendCommand(QString peer_id,QString msg);

    int PlayRecordFiles(QString peer_id,QString record_info_list);
    int StopPlayFile(QString peer_id);
    void setSavePath(const QString &path );


    void OnRecordStatus(QString peer_id,int status);
private:
    QQueue<RecordFileInfo> need_play_records_;
    VideoWall * video_wall_;
    talk_base::scoped_ptr<KeQtTunnelClient> tunnel_;
    talk_base::scoped_ptr<PeerConnectionClientDealer> connection_;
    QString m_savePath;
    bool is_inited;
};

#endif // KEPLAYERPLUGIN_H
