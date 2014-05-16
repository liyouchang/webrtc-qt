/*
 *
 * Copyright 2014 Kaer Electric Co.,Ltd
 * Listens see <http://www.gnu.org/licenses/>.
 * AUTHORS lht
 *
 * RecorderAvi 实现了视频数据存储成avi文件的功能
 *
 */


#include "recorderavi.h"

#include <stdlib.h>
#include <cstring>
#include <stdint.h>

#include "talk/base/stream.h"
#include "talk/base/stringencode.h"

#define AVIF_HASINDEX       0x00000010  // Index at end of file?
#define AVIF_ISINTERLEAVED  0x00000100
#define AVIF_TRUSTCKTYPE    0x00000800  // Use CKType to find key frames?


//avih
struct MainAVIHeader{
    uint32_t dwMicroSecPerFrame;//显示每桢所需的时间ns，定义avi的显示速率
    uint32_t dwMaxBytesPerSec; // 最大的数据传输率
    uint32_t dwPaddingGranularity; //记录块的长度需为此值的倍数，通常是2048
    uint32_t dwFlages; //AVI文件的特殊属性，如是否包含索引块，音视频数据是否交叉存储
    uint32_t dwTotalFrame; //文件中的总桢数
    uint32_t dwInitialFrames; //说明在开始播放前需要多少桢
    uint32_t dwStreams; //文件中包含的数据流种类
    uint32_t dwSuggestedBufferSize; //建议使用的缓冲区的大小，
    //通常为存储一桢图像以及同步声音所需要的数据之和
    uint32_t dwWidth; //图像宽
    uint32_t dwHeight; //图像高
    uint32_t dwReserved[4]; //保留值
};

struct AVIRECT
{
    uint16_t    left;
    uint16_t    top;
    uint16_t    right;
    uint16_t    bottom;
} ;

//strh
struct AVIStreamHeader{
    char fccType[4]; //4字节，表示数据流的种类 vids 表示视频数据流 //auds 音频数据流
    char fccHandler[4];//4字节 ，表示数据流解压缩的驱动程序代号
    uint32_t dwFlags; //数据流属性
    uint16_t wPriority; //此数据流的播放优先级
    uint16_t wLanguage; //音频的语言代号
    uint32_t dwInitalFrames;//说明在开始播放前需要多少桢
    uint32_t dwScale; //数据量，视频每桢的大小或者音频的采样大小
    uint32_t dwRate; //dwScale /dwRate = 每秒的采样数
    uint32_t dwStart; //数据流开始播放的位置，以dwScale为单位
    uint32_t dwLength; //数据流的数据量，以dwScale为单位
    uint32_t dwSuggestedBufferSize; //建议缓冲区的大小
    uint32_t dwQuality; //解压缩质量参数，值越大，质量越好
    uint32_t dwSampleSize; //音频的采样大小
    AVIRECT rcFrame; //视频图像所占的矩形
};


struct keBITMAPINFOHEADER{
    uint32_t      biSize;
    long       biWidth;
    long       biHeight;
    uint16_t       biPlanes;
    uint16_t       biBitCount;
    uint32_t      biCompression;
    uint32_t      biSizeImage;
    long       biXPelsPerMeter;
    long       biYPelsPerMeter;
    uint32_t      biClrUsed;
    uint32_t      biClrImportant;
};

struct keRGBQUAD {
    uint8_t    rgbBlue;
    uint8_t    rgbGreen;
    uint8_t    rgbRed;
    uint8_t    rgbReserved;
};

//strf video info
struct keBITMAPINFO {
    keBITMAPINFOHEADER    bmiHeader;
    keRGBQUAD             bmiColors[1];
};

//strf wave info
typedef struct {
    uint16_t wFormatTag;
    uint16_t nChannels; //声道数
    uint32_t nSamplesPerSec; //采样率
    uint32_t nAvgBytesPerSec; //WAVE声音中每秒的数据量
    uint16_t nBlockAlign; //数据块的对齐标志
    uint16_t biSize; //此结构的大小
}WAVEFORMAT;


typedef struct {
    uint32_t ckid; //记录数据块中子块的标记
    uint32_t dwFlags; //表示chid所指子块的属性
    uint32_t dwChunkOffset; //子块的相对位置
    uint32_t dwChunkLength; //子块长度
}AVIINDEXENTRY;


void Reso2WidthHeigh(int reso,int *width,int *high)
{
    switch(reso)
    {
    case 1:                     //QCIF
        *width = 176;
        *high = 144;
        break;
    case 2:                     //CIF
        *width = 352;
        *high = 288;
        break;
    case 3:                     //HD
        *width = 704;
        *high = 288;
        break;
    case 4:                     //QVGA
        *width = 320;
        *high = 240;
        break;
    case 7:						//VGA
        *width = 640;
        *high = 480;
        break;
    case 10:                    //720P
        *width = 1280;
        *high = 720;
        break;
    case 12:                    //960P
        *width = 1280;
        *high = 960;
        break;
    case 6:                     //1080P
        *width = 1920;
        *high = 1080;
        break;
    default:                    //D1
        *width = 704;
        *high = 576;
        break;
    }
}


RecorderAvi::RecorderAvi(const std::string &peerId,int frameRate,int frameType):
    frameRate_(frameRate),frameType_(frameType),peerId_(peerId)
{
    aviIndex_ = new talk_base::Buffer();
    aviFile_ = new talk_base::FileStream();
    this->moviListLen = 4;
    this->recvIFrame = false;
    this->frameCount = 0;

}

RecorderAvi::~RecorderAvi()
{
    StopRecord();
    delete aviIndex_;
    delete aviFile_;
}


bool RecorderAvi::StartRecord(const std::string & filename)
{
    bool ret = aviFile_->Open(filename,"wb",NULL);
    if(!ret){
        LOG(WARNING)<<"RecorderAvi::StartRecord---"<<
                      "open file error "<<filename;
        return ret;
    }
    unsigned int len = 0;
    talk_base::Buffer head;
    head.AppendData("RIFF~~~~AVI ",12);//20
    head.AppendData("LIST",4);
    //len = 4 + 4*16 + 12 + 4*16 + 4*12 +12 + 4*16 + 4*6+2;
    int aviListLen = 4 + 4*16 + 12 + 4*16 + 4*12 +12 + 4*16 + 4*6;
    head.AppendData(&aviListLen,4);

    head.AppendData("hdrl",4);//4

    head.AppendData("avih",4); //4*16
    len = sizeof(MainAVIHeader);
    head.AppendData(&len,4);
    MainAVIHeader avimainheader;
    memset(&avimainheader,sizeof(MainAVIHeader),0);
    avimainheader.dwMicroSecPerFrame = 1000000/frameRate_;
    avimainheader.dwMaxBytesPerSec = 0xffffffff;
    avimainheader.dwPaddingGranularity = 0;
    avimainheader.dwFlages = AVIF_HASINDEX|AVIF_ISINTERLEAVED|AVIF_TRUSTCKTYPE;
    avimainheader.dwTotalFrame = 0;
    avimainheader.dwInitialFrames = 0;
    avimainheader.dwStreams = 2;
    avimainheader.dwSuggestedBufferSize = 1000000;
    int width;
    int height;
    Reso2WidthHeigh(this->frameType_,&width,&height);
    avimainheader.dwWidth = width;
    avimainheader.dwHeight = height;
    avimainheader.dwReserved[0] = 0;
    avimainheader.dwReserved[1] = 0;
    avimainheader.dwReserved[2] = 0;
    avimainheader.dwReserved[3] = 0;
    head.AppendData(&avimainheader,sizeof(MainAVIHeader));

    head.AppendData("LIST",4);//12
    len = 4 + 4*16 + 4*12;
    head.AppendData(&len,4);
    head.AppendData("strl",4);

    head.AppendData("strh",4);//4*16
    len =sizeof(AVIStreamHeader);
    head.AppendData(&len,4);
    AVIStreamHeader avistreamheader;
    memcpy(avistreamheader.fccType,"vids",4);
    memcpy(avistreamheader.fccHandler,"h264",4);
    avistreamheader.dwFlags = 0;
    avistreamheader.wPriority = 0;
    avistreamheader.wLanguage = 0;
    avistreamheader.dwInitalFrames = 0;
    avistreamheader.dwScale = 1000;
    avistreamheader.dwRate = this->frameRate_*1000;
    avistreamheader.dwStart = 0;
    avistreamheader.dwLength = 0;
    avistreamheader.dwSuggestedBufferSize = (1<<20);
    avistreamheader.dwQuality = -1;
    avistreamheader.dwSampleSize = width*height;
    avistreamheader.rcFrame.left = 0;
    avistreamheader.rcFrame.top = 0;
    avistreamheader.rcFrame.right = width;
    avistreamheader.rcFrame.bottom = height;
    head.AppendData(&avistreamheader,sizeof(AVIStreamHeader));

    head.AppendData("strf",4);//4*12
    len = 4*12 - 8;
    head.AppendData(&len,4);
    keBITMAPINFOHEADER bitmapinfoheader;
    memset(&bitmapinfoheader,sizeof(keBITMAPINFOHEADER),0);
    bitmapinfoheader.biSize = 4*12 -8;
    bitmapinfoheader.biWidth = width;
    bitmapinfoheader.biHeight = height;
    bitmapinfoheader.biPlanes = 1;
    bitmapinfoheader.biBitCount = 24;
    memcpy(&bitmapinfoheader.biCompression,"h264",4);
    bitmapinfoheader.biSizeImage = width*height;
    bitmapinfoheader.biXPelsPerMeter = 0;
    bitmapinfoheader.biYPelsPerMeter = 0;
    bitmapinfoheader.biClrUsed = 0;
    bitmapinfoheader.biClrImportant = 0;
    head.AppendData(&bitmapinfoheader,sizeof(keBITMAPINFOHEADER));

    head.AppendData("LIST",4);//12
    //len = 4 + 4*16 + 4*6 + 2;
    len = 4 + 4*16 + 4*6 ;
    head.AppendData(&len,4);
    head.AppendData("strl",4);

    head.AppendData("strh",4);//4*16
    len =sizeof(AVIStreamHeader);
    head.AppendData(&len,4);
    //    AVIStreamHeader avistreamheader; //reuse
    memcpy(avistreamheader.fccType,"auds",4);
    memcpy(avistreamheader.fccHandler,"G711",4);
    avistreamheader.dwFlags = 0;
    avistreamheader.wPriority = 0;
    avistreamheader.wLanguage = 0;
    avistreamheader.dwInitalFrames = 0;
    avistreamheader.dwScale = 1;
    avistreamheader.dwRate = 8000;
    avistreamheader.dwStart = 0;
    avistreamheader.dwLength = 325;
    avistreamheader.dwSuggestedBufferSize = 32768;
    avistreamheader.dwQuality = 0;
    avistreamheader.dwSampleSize = 8000;
    avistreamheader.rcFrame.left = 0;
    avistreamheader.rcFrame.top = 0;
    avistreamheader.rcFrame.right = 0;
    avistreamheader.rcFrame.bottom = 0;
    head.AppendData(&avistreamheader,sizeof(AVIStreamHeader));

    head.AppendData("strf",4);//4*6
    len = sizeof(WAVEFORMAT);
    head.AppendData(&len,4);
    WAVEFORMAT waveformat;
    waveformat.wFormatTag = 0x07;
    waveformat.nChannels = 0x01;
    waveformat.nSamplesPerSec = 8000;
    waveformat.nAvgBytesPerSec = 8000;
    waveformat.nBlockAlign = 1;
    waveformat.biSize = 8;
    head.AppendData(&waveformat,sizeof(WAVEFORMAT));
    //这里原来有这个代码，不知有什么影响
    //    int tmp = 0;
    //    head.AppendData(&tmp,2);
    //+2
    head.AppendData("LIST~~~~movi",12);
    //init variable
    this->moviListLen = 4;
    this->recvIFrame = false;
    this->frameCount = 0;

    //index
    aviIndex_->AppendData("idx1~~~~",8);

    talk_base::StreamResult result = aviFile_->WriteAll(
                head.data(),head.length(),NULL,NULL);
    if(result != talk_base::SR_SUCCESS){
        LOG(WARNING)<<"RecorderAvi::StartRecord---"<<"write file error ";
        return false;
    }
    return true;
}

bool RecorderAvi::StopRecord()
{
    if(!recvIFrame){
        return false;
    }
    recvIFrame = false;
    size_t indexDataLen = aviIndex_->length() - 8;

    memcpy(aviIndex_->data()+4,&indexDataLen,4);
    talk_base::StreamResult result = aviFile_->WriteAll(
                aviIndex_->data(),aviIndex_->length(),NULL,NULL);
    if(result != talk_base::SR_SUCCESS){
        LOG(WARNING)<<"RecorderAvi::StopRecord---"<<"write file error ";
        return false;
    }
    size_t totalLen ;
    aviFile_->Flush();
    bool ret = aviFile_->GetSize(&totalLen);
    if(!ret){
        LOG(WARNING)<<"RecorderAvi::StopRecord---"<<"GetSize error ";
        return false;
    }
    totalLen -= 8;
    aviFile_->SetPosition(4);
    aviFile_->Write(&totalLen,4,NULL,NULL);
    aviFile_->SetPosition(316);
    aviFile_->Write(&moviListLen,4,NULL,NULL);

    aviFile_->Close();
    return true;
}

void RecorderAvi::OnVideoData(const std::string &peerId, const char *data, int len)
{
    if(this->peerId_ != peerId){
        return;
    }
    AviFileWriteVideo(data+12,len-12);
}

void RecorderAvi::OnAudioData(const std::string &peerId, const char *data, int len)
{
    if(this->peerId_ != peerId){
        return;
    }
    AviFileWriteAudio(data+16,len-16);
}

bool RecorderAvi::AviFileWriteVideo(const char * mediaData,int mediaLen)
{

    if(!recvIFrame){
        if((mediaData[4]&0x1f)==0x07){
            LOG(INFO)<<"RecorderAvi::AviFileWriteVideo---"<<
                       "receive iframe, start record";
            recvIFrame = true;
        }else{
            return false;
        }
    }

    AVIINDEXENTRY aviindex;
    memcpy(&aviindex.ckid,"00dc",4);
    aviindex.dwFlags = ((mediaData[4]&0x1f)==0x07) ?0x10 :0 ;//判断idr帧
    aviindex.dwChunkOffset = 320 + moviListLen;
    aviindex.dwChunkLength = mediaLen;
    aviIndex_->AppendData(&aviindex,sizeof(AVIINDEXENTRY));

    talk_base::Buffer data;
    data.AppendData("00dc",4);
    data.AppendData(&mediaLen,4);
    data.AppendData(mediaData,mediaLen);
    moviListLen += mediaLen+8;
    if(mediaLen&0x01)//长度需为偶数?
    {
        ++moviListLen;
        data.AppendData("0",1);
    }
    size_t written;
    talk_base::StreamResult result = aviFile_->WriteAll(data.data(),data.length(),&written,NULL);
    if(result != talk_base::SR_SUCCESS){
        LOG(WARNING)<<"RecorderAvi::aviFileWriteVideo---"<<"write file error ";
        return false;
    }
    ++frameCount;
    return true;
}

bool RecorderAvi::AviFileWriteAudio(const char *mediaData, int mediaLen)
{
    if(!recvIFrame){
        return false;
    }
    AVIINDEXENTRY aviindex;
    memcpy(&aviindex.ckid,"01wb",4);
    aviindex.dwFlags = 0;
    aviindex.dwChunkOffset = 320 + moviListLen;
    aviindex.dwChunkLength = mediaLen;
    aviIndex_->AppendData(&aviindex,sizeof(AVIINDEXENTRY));

    talk_base::Buffer data;
    data.AppendData("01wb",4);
    data.AppendData(&mediaLen,4);
    data.AppendData(mediaData,mediaLen);
    moviListLen += mediaLen+8;
    if(mediaLen&0x01)//长度需为偶数?
    {
        data.AppendData("0",1);
        ++moviListLen;
    }
    size_t written;
    talk_base::StreamResult result = aviFile_->WriteAll(data.data(),data.length(),&written,NULL);
    if(result != talk_base::SR_SUCCESS){
        LOG(WARNING)<<"RecorderAvi::aviFileWriteVideo---"<<"write file error ";
        return false;
    }
    return true;
}
