#include "recorderavi.h"

#include <stdlib.h>
#include <cstring>
#include <stdint.h>

#include "talk/base/stream.h"


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


typedef struct tagBITMAPINFOHEADER{
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
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
        uint8_t    rgbBlue;
        uint8_t    rgbGreen;
        uint8_t    rgbRed;
        uint8_t    rgbReserved;
} RGBQUAD;

//strf video info
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;

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

/**********************************************************************/
//启动与退出
/**********************************************************************/
void aviWriteAudio(char *audio,int audio_len,char *aviBuf,int *aviPtr,int *aviTotal,char *index,int *index_ptr)
{
    memcpy(aviBuf+(*aviPtr),"01wb",4);
    memcpy(aviBuf+(*aviPtr)+4,&audio_len,4);
    memcpy(aviBuf+(*aviPtr)+8,audio,audio_len);
    (*aviPtr) = (*aviPtr)+8+audio_len;
    if(audio_len&0x01)
    {
        memcpy(aviBuf+(*aviPtr),"0",1);
        (*aviPtr)++;
    }

    if ((*index_ptr) != 0)
        (*index_ptr) -= 4;
    memcpy(index+(*index_ptr),"01wb",4);
    memset(index+(*index_ptr)+4,0,4);
    memcpy(index+(*index_ptr)+8,aviTotal,4);
    memcpy(index+(*index_ptr)+12,&audio_len,4);
    if(audio_len&0x01)
        (*aviTotal) = (*aviTotal)+audio_len+8+1;
    else
        (*aviTotal) = (*aviTotal)+audio_len+8;
    int total = (*aviTotal);
    memcpy(index+(*index_ptr)+16,&total,4);
    (*index_ptr) += 20;
}

void aviWriteVideo(char *media,int media_len,char *aviBuf,int *aviPtr,int * aviTotal,char *index,int *index_ptr)
{
    memcpy(aviBuf+(*aviPtr),"00dc",4);
    memcpy(aviBuf+(*aviPtr)+4,&media_len,4);
    memcpy(aviBuf+(*aviPtr)+8,media,media_len);
    (*aviPtr) = (*aviPtr)+8+media_len;
    if(media_len&0x01)
    {
        memcpy(aviBuf+(*aviPtr),"0",1);
        (*aviPtr)++;
    }

    if ((*index_ptr) != 0)
        (*index_ptr) -= 4;
    memcpy(index+(*index_ptr),"00dc",4);
    memset(index+(*index_ptr)+4,0,4);
    if ((media[4]&0x1f)==0x07)
        index[(*index_ptr)+4] = 0x10;
    memcpy(index+(*index_ptr)+8,aviTotal,4);
    memcpy(index+(*index_ptr)+12,&media_len,4);
    if(media_len&0x01)
        (*aviTotal) = (*aviTotal)+media_len+8+1;
    else
        (*aviTotal) = (*aviTotal)+media_len+8;
    int total = (*aviTotal);
    memcpy(index+(*index_ptr)+16,&total,4);
    (*index_ptr) += 20;
}

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

void aviCreateHead(char *head_buf,int *ptr,char *index_buf,int *index_len,int reso,int frame)
{
    int len;
    unsigned short tmp;
    (*ptr) = 0;
    memcpy(head_buf+(*ptr),"RIFF",4);(*ptr)+=4;
    memset(head_buf+(*ptr),0,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"AVI ",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"LIST",4);(*ptr)+=4;
    len = 4 + 4*16 + 12 + 4*16 + 4*12 +12 + 4*16 + 8+4*4+2;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"hdrl",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"avih",4);(*ptr)+=4;
    len = 4*16 - 8;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 1000000/frame;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;			// ************my test 视频帧间隔时间 以毫秒为单位
    memset(head_buf+(*ptr),0xff,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    len = AVIF_HASINDEX|AVIF_ISINTERLEAVED|AVIF_TRUSTCKTYPE;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 3000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;			//  *************my test 文件的总帧数 此处有误,不过不影响。
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    len = 2;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 1000000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    int width;
    int height;
    Reso2WidthHeigh(reso,&width,&height);
    memcpy(head_buf+(*ptr),&width,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),&height,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"LIST",4);(*ptr)+=4;
    len = 4 + 4*16 + 4*12;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"strl",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"strh",4);(*ptr)+=4;
    len = 4*16 - 8;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"vids",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"h264",4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    len = 1000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;	 // dwScale 这个流使用的时间尺度
    len = frame*1000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;	//  ********* my test dwRate  dwRate / dwScale = 帧率（fram rate ）。
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    len = 3000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;	// 	**********my test dwLength 表示流的长度。这其实就是总帧数，用dwLength除以帧率，即得到流的总时长。
    len = (1<<20);
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = -1;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = width*height;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    tmp = width;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    tmp = height;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    memcpy(head_buf+(*ptr),"strf",4);(*ptr)+=4;
    len = 4*12 - 8;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 4*12 - 8;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;			// 0xb0
    memcpy(head_buf+(*ptr),&width,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),&height,4);(*ptr)+=4;
    tmp = 1;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    tmp = 24;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    memcpy(head_buf+(*ptr),"h264",4);(*ptr)+=4;
    len = width*height;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;

    memcpy(head_buf+(*ptr),"LIST",4);(*ptr)+=4;
    len = 4 + 4*16 + 4*6 + 2;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"strl",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"strh",4);(*ptr)+=4;
    len = 4*16 - 8;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"auds",4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"G711",4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    len = 1;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 8000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    len = 325;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 32768;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 0;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 8000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    tmp = 0;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    tmp = 0;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;

    memcpy(head_buf+(*ptr),"strf",4);(*ptr)+=4;
    len = 4*4+2;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    tmp = 0x07;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    tmp = 0x01;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    len = 8000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    len = 8000;
    memcpy(head_buf+(*ptr),&len,4);(*ptr)+=4;
    tmp = 1;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    tmp = 8;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    tmp = 0;
    memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;

    memcpy(head_buf+(*ptr),"LIST",4);(*ptr)+=4;
    memset(head_buf+(*ptr),0x00,4);(*ptr)+=4;
    memcpy(head_buf+(*ptr),"movi",4);(*ptr)+=4;

    (*index_len) = 0;
    memcpy(index_buf+(*index_len),"idx1",4);(*index_len)+=4;
    memset(index_buf+(*index_len),0x00,4);(*index_len)+=4;
    len = (*ptr);
    memcpy(index_buf+(*index_len),&len,4);(*index_len)+=4;
}


RecorderAvi::RecorderAvi():frameRate_(25),frameType_(2)
{
    aviIndex_ = new talk_base::Buffer();
    aviFile_ = new talk_base::FileStream();
}

RecorderAvi::~RecorderAvi()
{
    delete aviIndex_;
    delete aviFile_;
}


bool RecorderAvi::StartRecord(const std::string & filename)
{
    bool ret = aviFile_->Open(filename,"w",NULL);
    if(!ret){
        return ret;
    }
    unsigned int len = 0;
    talk_base::Buffer head;
    head.AppendData("RIFF~~~~AVI",12);
    head.AppendData("LIST",4);
    len = 4 + 4*16 + 12 + 4*16 + 4*12 +12 + 4*16 + 8+4*4+2;
    head.AppendData(&len,4);
    head.AppendData("hdrl",4);
    head.AppendData("avih",4);
    len = sizeof(MainAVIHeader);
    head.AppendData(&len,4);

    MainAVIHeader avimainheader;
    avimainheader.dwMicroSecPerFrame = 1000000/frameRate_;
    avimainheader.dwMaxBytesPerSec = 0xffffffff;
    avimainheader.dwPaddingGranularity = 0;
    avimainheader.dwFlages = AVIF_HASINDEX|AVIF_ISINTERLEAVED|AVIF_TRUSTCKTYPE;
    avimainheader.dwTotalFrame = 3000;
    avimainheader.dwInitialFrames = 0;
    avimainheader.dwStreams = 2;
    avimainheader.dwSuggestedBufferSize = 1000000;

    int width;
    int height;
    Reso2WidthHeigh(this->frameType_,&width,&height);

    avimainheader.dwWidth = width;
    avimainheader.dwHeight = height;

    head.AppendData(&avimainheader,sizeof(MainAVIHeader));

    head.AppendData("LIST",4);
    len = 4 + 4*16 + 4*12;
    head.AppendData(&len,4);
    head.AppendData("strl",4);
    head.AppendData("strh",4);
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
    avistreamheader.dwLength = 3000;
    avistreamheader.dwSuggestedBufferSize = (1<<20);
    avistreamheader.dwQuality = -1;
    avistreamheader.dwSampleSize = width*height;
    avistreamheader.rcFrame.left = 0;
    avistreamheader.rcFrame.top = 0;
    avistreamheader.rcFrame.right = width;
    avistreamheader.rcFrame.bottom = height;
    head.AppendData(&avistreamheader,sizeof(AVIStreamHeader));

    head.AppendData("strf",4);
    len = 4*12 - 8;
    head.AppendData(&len,4);
    BITMAPINFOHEADER bitmapinfoheader;
    bitmapinfoheader.biSize = 4*12 -8;
    bitmapinfoheader.biWidth = width;
    bitmapinfoheader.biHeight = height;
    bitmapinfoheader.biPlanes = 1;
    bitmapinfoheader.biBitCount = 24;
    memcpy(&bitmapinfoheader.biCompression,"h264",4);
    bitmapinfoheader.biSizeImage = width*height;
    head.AppendData(&bitmapinfoheader,sizeof(BITMAPINFOHEADER));

    head.AppendData("LIST",4);
    len = 4 + 4*16 + 4*6 + 2;
    head.AppendData(&len,4);
    head.AppendData("strl",4);
    head.AppendData("strh",4);
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

    head.AppendData("strf",4);
    len = sizeof(WAVEFORMAT);
    head.AppendData(&len,4);
    WAVEFORMAT waveformat;
    waveformat.wFormatTag = 0x07;
    waveformat.nChannels = 0x01;
    waveformat.nSamplesPerSec = 8000;
    waveformat.nAvgBytesPerSec = 8000;
    waveformat.nBlockAlign = 1;
    waveformat.biSize = 8;
    //tmp = 0;
    //memcpy(head_buf+(*ptr),&tmp,2);(*ptr)+=2;
    head.AppendData(&waveformat,sizeof(WAVEFORMAT));


    head.AppendData("LIST~~~~movi",12);


//    (*index_len) = 0;
//    memcpy(index_buf+(*index_len),"idx1",4);(*index_len)+=4;
//    memset(index_buf+(*index_len),0x00,4);(*index_len)+=4;
//    len = (*ptr);
//    memcpy(index_buf+(*index_len),&len,4);(*index_len)+=4;

}

void RecorderAvi::OnVideoData(const std::string &peerId, const char *data, int len)
{
    if(this->peerId_ != peerId){
        return;
    }

}
