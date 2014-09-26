#include "recordermp4.h"
#include "talk/base/logging.h"

kaerp2p::RecorderMp4::RecorderMp4(const std::string &peerId,
                                  int frameRate, int width, int height):
    frameRate(frameRate),frameHeight(height),frameWidth(width),peerId(peerId),
    hMp4File(NULL)
{
    mp4 = new MP4Encoder();
}

kaerp2p::RecorderMp4::~RecorderMp4()
{
    delete mp4;
}

bool kaerp2p::RecorderMp4::StartSave(const std::string &fileName)
{
    LOG_F(INFO)<< fileName;
    hMp4File = mp4->CreateMP4File(
                fileName.c_str(),frameWidth,frameHeight,90000,frameRate);
    return !(hMp4File == MP4_INVALID_FILE_HANDLE);
}

bool kaerp2p::RecorderMp4::StopSave()
{
    LOG_F(INFO) << hMp4File;
    mp4->CloseMP4File(hMp4File);
    return true;
}

void kaerp2p::RecorderMp4::OnVideoData(const std::string &peerId, const char *data, int len)
{
    int writelen  = mp4->WriteH264Data(hMp4File,(const unsigned char *)data+12,len-12);
//    LOG_F(INFO)<<"receive len "<<len<<"; write len "<<writelen;

}

void kaerp2p::RecorderMp4::OnAudioData(const std::string &peerId, const char *data, int len)
{
   int writelen =  mp4->WriteULawData(hMp4File,(const unsigned char *)data+16,len-16);
//    LOG_F(INFO)<<"receive len "<<len<<"; write len "<<writelen;
}
