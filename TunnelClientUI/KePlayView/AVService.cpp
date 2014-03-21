#include "avservice.h"


#include"AVPlay.h"

AVService::AVService(QObject *parent) :
    QObject(parent)
{
    StreamID++;
    if (StreamID>35)
    {
        StreamID=1;
    }
    m_lPlayHandle = StreamID;
    m_lPause      = 1;
    m_hPlayWnd = NULL;
    playStatus = PLAYSTATUS_Free;
    m_iPlaySpeed = 0;
}

void AVService::SetPlayWnd(HWND hWnd)
{
    m_hPlayWnd = hWnd;
    if (IsPlaying())
    {
        int ret = AV_ReInitViewHandle(m_lPlayHandle,m_hPlayWnd);
        if (ret != 0)
        {
            qWarning("AV_ReInitViewHandle error!");
        }
    }
}

int AVService::OpenStream()
{
    int iRet = AV_OpenStream_Ex(m_lPlayHandle);
    if (iRet != 0)
    {
        qWarning("播放器打开失败 iRet=%d\n",iRet);
        playStatus = PLAYSTATUS_Free;
        return iRet;
    }
    iRet = AV_Play(m_lPlayHandle,m_hPlayWnd);
    if (iRet != 0 )
    {
        qCritical("播放器播放失败 iRet= %d",iRet);
        playStatus = PLAYSTATUS_Free;
        return iRet;
    }
    playStatus = PLAYSTATUS_PlayStream;
    //AV_SetPlayVideoInfo(m_lPlayHandle,m_videoID,m_channelNo);
    return iRet;
}

int AVService::InputStream(const char *data, int dataLen)
{
    AV_InputData(m_lPlayHandle,(long *)(data) , dataLen);
    return 0;
}

int AVService::CloseStream()
{
    int ret = AV_CloseStream_Ex(m_lPlayHandle);
    playStatus = PLAYSTATUS_Free;
    return ret;
}

bool AVService::IsPlaying()
{
    return (this->playStatus != PLAYSTATUS_Free);
}

int AVService::CapPic(const char * fileName)
{
    return AV_CapPic_Ex(m_lPlayHandle,fileName);
}

int AVService::PlayFile(const char *fileName, int fileSize)
{
    int playHandle = 65;  //回放句柄范围  65~96

    long lRet = AV_OpenFile_Ex(playHandle,(char *)fileName,fileSize);
    if (lRet == 0)
    {
        lRet = AV_Play(playHandle,m_hPlayWnd);
        if (lRet != 0)
        {
            qWarning("播放失败");
            return lRet;
        }
        //AV_SetSpeed(m_lPlayHandle,m_iPlaySpeed);
        //this->NormalPlay();
        playStatus = PLAYSTATUS_PlayFile;

    }else{
        qWarning("打开文件失败");
    }
    return lRet;
}


bool AVService::isInitailed = 0;

int AVService::StreamID = 0;

int AVService::Initialize()
{
    if (!isInitailed)
        isInitailed = true;
    else
        return 0;
    int ret = AV_Initial(0);
    if (ret != 0)
    {
        qWarning("AV_Initial Error!");
    }
    return ret;
}

int AVService::Cleanup()
{
    if (isInitailed == 0)
    {
        return 0;
    }
    isInitailed = 0;
    return AV_Free();
}

