#ifndef KEMESSAGE_H
#define KEMESSAGE_H

enum KEMsgType
{
    DevMsg_HeartBeat = 0x03,
    KEDevMsg_AlarmVideo= 0x0E,
    KEDevMsg_AlarmSenser = 0x0F,
    KEDevMsg_SearchOnlineDevice = 0x49,
    KEMSG_RecordFileList = 0x53,
    KEMSG_REQUEST_PLAY_FILE = 0x54,
    KEMSG_RecordPlayData = 0x55,
    DevMsg_GetPTZParam = 0x73,
    DevMsg_SerialData = 0x46,
    DevMsg_WifiCheck=0xD1,
    DevMsg_WifiStart = 0xD2,
    Client_GetKeyDevice= 0xE0,
    Client_LoginDevice=0xE1,
    KEMSG_TYPE_VideoSvrOnline = 0x8B,//13.	请求视频服务器的状态
    KEMSG_TYPE_MEDIATRANS = 0x8F,
    KEMSG_TYPE_VIDEOSERVER = 0x0C,
    KEMSG_TYPE_MEDIASTATUS = 0x0D,
    KEMSG_TYPE_AUDIOSTREAM = 0x38,
    KEMSG_TYPE_VIDEOSTREAM = 0x39,
    DevMsg_SETVIDEOPARAM= 0x3B,
    DevMsg_GETVIDEOPARAM = 0x3E,
    Device_LoginServer = 0x44,
    Device_GetKeyServer = 0xD0,
    PROTOCOL_HEAD = 0xff
};

#define RESP_ACK 0x0d
#define RESP_NAK 0x05
#define RESP_END 0x06
#define RESP_CTRL 0x07
#define RESP_TALK_INUSE 0x08

#define KE_Terminal_Port 22616

#define MEDIA_NOCHANGE 100

inline int CreateCameraID(int vid,int chNo){
    return (vid<<8) + chNo;
}

//inline void CopyIP(char * dstIP,int srcIP){
//    int tmpIp = qToBigEndian<quint32>(srcIP);
//    QHostAddress addr(tmpIp);
//    strcpy(dstIP,addr.toString().toLatin1().data());
//}


#pragma pack(1)


struct KEDevMsgHead
{
    unsigned char protocal;
    unsigned char msgType;
    unsigned int msgLength;
    int videoID;
};

struct KERTStreamHead
{
    unsigned char protocal;
    unsigned  char msgType;//0x39 0x38
    int msgLength;
    int videoID;
    char channelNo;
};

struct KEFrameHead {
    short frameNo;
    short piecesNo;
    int second;//时间戳
    char millisecond;//millisecond devide 10
    char frameType;// 0-D1 1-QCIF 3-HD1 7-VGA 10-720P 4-QVGA 50-audio_frame
    short frameLen;
};

struct KEDevMsgViaClient
{
    unsigned char protocal;
    unsigned char msgType;
    unsigned int msgLength;
    int videoID;
    int clientID;
};

struct KEDevClientGetKeyResp{
    unsigned char protocal;
    unsigned char msgType;//0xE0
    unsigned int msgLength;
    int videoID;
    int clientID;
    char key[8];
};

struct KEDevClientLoginReq{
    unsigned char protocal;
    unsigned char msgType;//0xE1
    unsigned int msgLength;
    int videoID;
    int clientID;
    char md5[16];
};

struct KeDeviceGetKeyReq{
    unsigned char protocal;
    unsigned char msgType;//0xD0
    unsigned int msgLength;
    int videoID;
    char deviceMac[16];
};

struct KeDeviceGetKeyResp{
    unsigned char protocal;
    unsigned char msgType;//0xD0
    unsigned int msgLength;
    int videoID;
    char key[8];
};

struct KeDeviceLoginResp{
    unsigned char protocal;
    unsigned char msgType;//0x44
    unsigned int msgLength;
    int videoID;
    char resp;
};


struct SChlName
{
    unsigned char byStart;
    char szName[40];
};

struct KEDevClientLoginResp{
    unsigned char protocal;
    unsigned char msgType;//0xE1
    unsigned int msgLength;
    int videoID;
    int clientID;
    char resp;
    char grade;						//用户级别0：超级(具有设置、查看权限) 1：普通(只有查看权限)
    int ip;							//终端IP
    int mask;						//终端子网掩码
    int gateway;					//终端网关
    int hardVer;					//终端硬件版本号
    int softVer;					//终端软件版本号
    int encodeVer;					//终端编码版本号
    char videoModel[16];          //终端类型
    char deviceId[20];            //终端设备标识码
    char terName[40];				//终端名称
    //SChlName chlName[32];			//通道名称
    //char terTypeNo[32];           //设备对应的型号
};

struct KEDevMsgCommonReq{
    unsigned char protocal;
    unsigned  char msgType;
    unsigned int msgLength;
    int videoID;
    int clientID;
    char resp;
};

struct KEChannelCommonResp{
    unsigned char protocal;
    unsigned  char msgType;
    unsigned int msgLength;
    int videoID;
    int clientID;
    char chanelNo;
    char resp;
};

struct KEDeviceCommonResp
{
    unsigned char protocal;
    unsigned  char msgType;
    unsigned int msgLength;
    int videoID;
    int clientID;
    char resp;
};

struct KEDevAPListItem
{
    char essid[32];
    char encryptType; // 0/1/2/3 关闭/wep/WPA-PSK/WPA2-PSK
    char wepPlace;      //WEP:1-4的数字 PSK:1- TKIP;2-AES
    char pwdFormat;  //0/1 WEP 16禁制/ASCII
    char wifiStart;        //0/1=关/开
    char password[32];
};

struct KEDevWifiStartReq{
    unsigned char protocal;
    unsigned char msgType; //0xD2
    int msgLength;
    int videoID;
    int clientID;
    KEDevAPListItem APItem;
    char  pppoeUse;  //0/1关/开
    char pppoeAccount[30];
    char pppoePWD[30];
};


struct KEVedioParam{
    char videoFormat;//视频图像大小（P制）：0-D1，1-qcif，2-CIF，3-hd1，4-QVGA，5-SVGA(800*600)，6-FULL HD，7-VGA(640*480)，8-QQVGA(160*112)，9-XGA(1024*768)，10-D4/720p(1280*720)，11-WXGA(1280*800)，12-XVGA(1280*960)，13-UXGA(1600*1200)，14-D2/480p(720*480)（N制）：0-VGA, 1-QQVGA, 2-QVGA
    char videoFramerate;//视频帧率
    char mainIFrameInterval;// 关键桢间隔
    short maxRate;// 最大码率：16K~3000
    short targetRate;//目标码率：16K~3000
    char rateControlMethod;//码率控制方式：0/1=VBR/CBR
    char preferredRate;// 码率控制优先选择 0/1=祯率/图像质量 默认1图像质量
    char dummy[5];
    char subVideoFormat;//子视频图像格式
    char subVideoFramerate;
    char subIFrameInterval;
    short subMaxRate;
    short subTargetRate;
    char subDummy[5];
    char subChannelEnable;//通道使能1表示关闭使能，其他值表示使能
    char subStream;//0传输/1存储
    char centerStorage;//中心存储0是不启用，1是启用
};

struct KEVideoServerReq
{
    unsigned char protocal;
    unsigned char msgType;//0x0c
    int msgLength;
    int videoID;
    int clientID;
    char channelNo;
    char video;//视频=1 request main,2 request sub   =0 停止 100 状态不变
    char listen;//监听=1请求 =0 停止 100 状态不变
    char talk;// 对讲=1请求 =0 停止 100 状态不变
    char protocalType;//0/tcp ,1/udp
    int transSvrIp;//服务器IP
};

struct KEVideoServerResp
{
    unsigned char protocal;
    unsigned char msgType;//0x0c
    int msgLength;
    int videoID;
    int clientID;
    char channelNo;
    char respType;// ACK/NAK
    char frameRate; //lht add frame rate and frametype
    char frameType;
};

struct KEMediaStatus{
    unsigned char protocal;
    unsigned char msgType;//0x0D KEMSG_TYPE_MEDIASTATUS
    unsigned int msgLength;
    int clientID;
    int videoID;
    char channelNo;
    int video;//0:无视频,1:主通道视频(高清),2:子通道视频(均衡),3:扩展通道视频(流畅),100:无效
    int listen;//0:无监听,1:有监听,100:无效
    int talk;//1:有对讲,0:无对讲,100:无效
    char respType;//error num 预留
};

typedef struct _KEMsgMediaTransReq
{
    unsigned char protocal;
    unsigned char msgType;//0x8f
    unsigned int msgLength;
    int clientID;
    int videoID;
    char channelNo;
    char video;//视频=0请求   =1 停止
    char listen;//监听=0请求   =1 停止
    char talk;// 对讲=0请求   =1 停止
    char devType;// 2：客户端   3：平台   5：录像服务器
}KEMsgMediaTransReq,*PKEMsgMediaTransReq;

typedef struct _KEMsgMediaTransResp
{
    unsigned  char protocal;
    unsigned char msgType;//0x8f
    unsigned int msgLength;
    int clientID;
    int videoID;
    char channelNo;
    char video;//视频=0请求   =1 停止
    char listen;//监听=0请求   =1 停止
    char talk;//, , 对讲=0请求   =1 停止
    char respType;// 响应类型:0请求成功;1 失败;2视频服务器不在线;	3通道被禁用;4超出最大转发数;8每通道最多可发给8个客户端、一个录像和一个平台;6 最多可建立600个TCP连接
}KEMsgMediaTransResp,*PKEMsgMediaTransResp;


struct KERecordFileListReq
{
    unsigned  char protocal;
    unsigned  char msgType;//0x53
    int msgLength;//29
    int videoID;
    int clientID;
    char channelNo;
    char startTime[6];
    char endTime[6];
    char fileType;//1计划录像；2表示传感器报警；	3移动侦测报警；4抓拍照片；
    char alarmNo;//1
};

struct KERecordFileListResp
{
    unsigned  char protocal;
    unsigned  char msgType;//0x53
    int msgLength;
    int videoID;
    int clientID;
    char channelNo;
    char resp;
};

struct KERecordFileInfo
{
    int fileNo;
    char startTime[6];
    char endTime[6];
    int fileSize;
    char data[80];
};

struct KEDevGetSerialDataHead
{
    unsigned  char protocal;
    unsigned  char msgType;//0x46
    int msgLength;
    int videoID;
    int clientID;
    short dataLen;
};


struct KEPlayRecordFileReq
{
    unsigned char protocal;
    unsigned char msgType;//0x54
    int msgLength;//31
    int videoID;
    int clientID;
    char playSpeed;//speed /0x10 normal ,0x20 2x speed,0x40 4x speed,
    //0x08 1/2 speed,0x04 1/4 speed
    char fileType;//1,new file play 2, file play control; 3,file play stop
    int playPos;//-1, pos not change,0~100 pos set percent. only used at filetype==2
    char startTime[6];//无用
    int clientIp;//0或本机ip
    char protocalType;
    char fileData[80];
};

struct KEPlayRecordFileResp
{
    unsigned char protocal;
    unsigned char msgType;//0x54
    int msgLength;//
    int videoID;
    int playPos;
    char playSpeed;
    char resp;//13 request download success,5 no file or file error
    int frameResolution;
    int frameRate;
//    char fileName[80];
};

struct KEPlayRecordDataHead
{
    unsigned char protocal;
    unsigned char msgType;//0x55
    int msgLength;//
    int videoID;
    int clientID;
    char channelNo;
    char resp;//13 后面有数据 ，6 数据结束,
};


//device ----> client
struct KEAlarmSenserReq
{
    unsigned char protocal;
    unsigned char msgType;//0x0F
    int msgLength;
    int videoID;
    char channelNo;
    char state;//状态0/1=开始/结束
    int second;//时间戳 second
    char msecond;//时间戳millisecond devide 10
    char number;//序号
};

//client ----> device
struct KEAlarmSenserResp
{
    unsigned char protocal;
    unsigned char msgType;//0x0F
    int msgLength;
    int videoID;
    char channelNo;
    char state;//状态0/1=开始/结束
    char resp;//ACK/NAK
    char number;//序号
};

//device ----> client
struct KEAlarmVideoReq
{
    unsigned char protocal;
    unsigned char msgType;//0x0E
    int msgLength;
    int videoID;
    char channelNo;
    char type;//Type： 0/1/2/=移动侦测/遮挡/视频丢失
    char coordinate[9];//坐标： 区域号+区域的坐标 9字节
    char state;//状态0/1=开始/结束
    int second;//时间戳 second
    char msecond;//时间戳millisecond devide 10
    char number;//序号
};

struct KEAlarmVideoResp
{
    unsigned char protocal;
    unsigned char msgType;//0x0E
    int msgLength;
    int videoID;
    char channelNo;
    char type;//Type： 0/1/2/=移动侦测/遮挡/视频丢失
    char coordinate[9];//坐标： 区域号+区域的坐标 9字节
    char state;//状态0/1=开始/结束
    char resp;
    char number;//序号
};

struct KESearchOnlineDeviceReq
{
    unsigned char protocal;
    unsigned char msgType;//0x49
    int msgLength;
    char kaer[10];//KAER970326
};
struct KESearchOnlineDeviceResp
{
    unsigned char protocal;//0xFF
    unsigned char msgType;//0x49
    int msgLength;
    int videoID;
    int devIp;
    int devMask;
    int devGateWay;
    int devDNS;
    char userName[8];
    char userPwd[8];
    char channelCount;//终端类型   1:单路 4:四路
    int devType;//类型  0：DVS, 1：解码器 ,2: DVR
    char devSN[20];//设备标识码20B
    int hardVer;//硬件型号
    char softVer[10];//软件版本号
    unsigned short port;
};

#pragma pack()



#endif // KEMESSAGE_H
