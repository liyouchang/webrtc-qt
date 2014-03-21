#ifndef AVSERVICE_H
#define AVSERVICE_H

#include <QObject>
#include<Windows.h>

class AVService : public QObject
{
    Q_OBJECT
public:
    explicit AVService(QObject *parent = 0);

    void SetPlayWnd(HWND hWnd);
    int OpenStream();
    int OpenSound();
    int CloseSound();
    int InputStream(const char * data, int dataLen);
    int CloseStream();
    bool IsPlaying();
    int CapPic(const char * fileName);
    int PlayFile(const char * fileName,int fileSize = 0);

    enum PlayStatus
    {
        PLAYSTATUS_Free = 0,
        PLAYSTATUS_PlayStream,
        PLAYSTATUS_PlayFile
    };

signals:

public slots:
    //int InputStream(QByteArray data);

public://静态方法
    int mCameraID;
    //static int StartTalk();
    //static int StopTalk();
    //static int GetAudioData(char * pBuf);
    static int Initialize();
    static int Cleanup();
private:
    long m_lPlayHandle;
    long  m_lPause;
    HWND m_hPlayWnd;
    PlayStatus  playStatus; //0,无播放；1，播放视频
    int m_iPlaySpeed;                       //播放速度 0、正常播放；-5、暂停
public:
    static int StreamID;
    static bool isInitailed ;


};

#endif // AVSERVICE_H
