#ifndef AVSERVICE_H
#define AVSERVICE_H

#include <Windows.h>

class AVService
{
public:
    explicit AVService(int streamID);
    ~AVService();
    void SetPlayWnd(HWND hWnd);
    int OpenStream(HWND playWnd);
    int OpenSound();
    int CloseSound();
    int InputStream(const char *data, int dataLen);
    int CloseStream();
    bool IsPlaying();
    int CapPic(const char *fileName);

    //play file methods
    int PlayFile(const char *fileName,int fileSize,HWND playWnd);
    int SetFileSize(long fileSize);
    int SetPlayPos(int pos);
    int GetPlayPos(int &pos);
    int CloseFile();
    enum PlayStatus
    {
        PLAYSTATUS_Free = 0,
        PLAYSTATUS_PlayStream,
        PLAYSTATUS_PlayFile
    };
public://静态方法
    int mCameraID;
    static int StartTalk();
    static int StopTalk();
    static int GetAudioData(char * pBuf);
    static int Initialize();
    static int Cleanup();
private:
    long m_lPlayHandle;
    long m_fileHandle;
    long  m_lPause;
    HWND m_hPlayWnd;
    PlayStatus  playStatus; //0,无播放；1，播放视频
    int m_iPlaySpeed; //播放速度 0、正常播放；-5、暂停
public:
    static bool isInitailed ;


};

#endif // AVSERVICE_H
