#include "kevideocutter.h"
#include <QFile>

KeVideoCutter::KeVideoCutter(QObject *parent) :
    QObject(parent),saveFile(NULL)
{
}


bool KeVideoCutter::Init(QString fileName)
{
    saveFile = new QFile(fileName,this);
    bool ret = saveFile->open(QIODevice::WriteOnly);
    return ret;

}

bool KeVideoCutter::IsOpen()
{
    return (saveFile && saveFile->isOpen());
}

void KeVideoCutter::OnVideoData(const char *data, int len)
{
    if(IsOpen()){
        saveFile->write(data,len);
    }
}

void KeVideoCutter::OnAudioData(const char *data, int len)
{
    if(IsOpen()){
        const int kFrameHeadLen = 16;
        saveFile->write(data+kFrameHeadLen,len-kFrameHeadLen);
    }
}

