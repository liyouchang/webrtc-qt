#include "avservice.h"
#include"AVPlay.h"
#include <QDebug>

AVService::AVService(int streamID)
{
    m_lPlayHandle = streamID;
    m_lPause      = 1;
    m_hPlayWnd = NULL;
    playStatus = PLAYSTATUS_Free;
    m_iPlaySpeed = 0;
    m_fileHandle = 65+streamID;
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

int AVService::OpenStream(HWND playWnd)
{
    if(playWnd != 0){
        m_hPlayWnd = playWnd;
    }
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

int AVService::OpenSound()
{
    int iRet = AV_SoundPlay(m_lPlayHandle);
    if (iRet != 0 )
    {
        qCritical("AV_SoundPlay error iRet= %d",iRet);
        return iRet;
    }
    return iRet;


}

int AVService::CloseSound()
{
    return AV_StopSound(m_lPlayHandle);
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
    QString name = fileName;
    name.replace(QRegExp("/"),"\\\\");
    int ret = -100;
    if(playStatus == 1){
        qDebug()<<"AVService::CapPic"<<"playhandle"<<fileName;
        ret = AV_CapPic_Ex(m_lPlayHandle,fileName);
    }else if(playStatus == 2){
        ret = AV_CapPic_Ex(m_fileHandle,fileName);
    }
    return ret;
}
/**
 * @brief AVService::PlayFile
 * @param fileName
 * @param fileSize : set to 0 by default
 * @param playWnd
 * @return
 */
int AVService::PlayFile(const char *fileName, int fileSize, HWND playWnd)
{
      //回放句柄范围  65~96
    if(playWnd != 0){
        m_hPlayWnd = reinterpret_cast<HWND>(playWnd);
    }
    long lRet = AV_OpenFile_Ex(m_fileHandle,(char *)fileName,fileSize);
    if (lRet == 0)
    {
        lRet = AV_Play(m_fileHandle,m_hPlayWnd);
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

int AVService::SetFileSize(long fileSize)
{
    return AV_SetFileSize(m_fileHandle,fileSize);
}

int AVService::SetPlayPos(int pos)
{
    return AV_SetPlayPos(m_fileHandle, pos);
}

int AVService::CloseFile()
{
    playStatus = PLAYSTATUS_Free;
    return AV_CloseFile_Ex(m_fileHandle);
}

int AVService::StartTalk()
{
    AV_InitAudioTalk();
    return AV_TalkOpen();
}

int AVService::StopTalk()
{
    return AV_TalkClose();
}

int AVService::GetAudioData(char *pBuf)
{
    return AV_GetAudioData(pBuf);
}


bool AVService::isInitailed = 0;

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

