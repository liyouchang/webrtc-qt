#ifndef KE_RECORDER_H
#define KE_RECORDER_H

#include <QFile>


struct RecordFileInfo{
    std::string peer_id;
    int size;
    std::string file_date;
    std::string remote_name;
};


class KeRecorder : public QObject
{
    Q_OBJECT
public:
    explicit KeRecorder(QObject *parent = 0);
    bool OpenFile(QString path,QString peer_id);
    bool OpenFile(RecordFileInfo info, QString record_path);
    int GetFileSize();
    void SetPreFileSize(int pre_size);
signals:
    void SigAbleToPlay(QString peer_id,QString file_name,int file_size);

public slots:
    void OnRecvRecordData(QString peer_id,QByteArray data);
    void OnRecordDownloadEnd(QString peer_id);

private:
    QString save_file_path_;
    QFile * file_;
    int pre_file_size_;
    QString save_peer_id_;
    bool sig_played;
};

#endif // KE_RECORDER_H
