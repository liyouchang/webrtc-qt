#ifndef COMMAND_DEFINE_H
#define COMMAND_DEFINE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
/**********************************************************************/
//module clock
/**********************************************************************/
typedef enum
{
	CLOCK_TYPE_HIRTC = 0,										//海思内部RTC时钟
	CLOCK_TYPE_PCF8563,											//外部硬件时钟芯片
	CLOCK_TYPE_NTP,												//网络校时
	CLOCK_TYPE_COUNT	
}	e_clock_type;

typedef struct struct_clock_s
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int week;	
}	st_clock_t;
/**********************************************************************/
//module config
/**********************************************************************/
#define MAXVIDEOCHNS				8					//通道数
#define MAX_ALARMINPUTS				16					//报警输入
#define MAX_ALARMOUTPUTS			16
typedef enum
{
	CONFIG_TYPE_DEV = 0,										//海思内部RTC时钟
	CONFIG_TYPE_NET,											//外部硬件时钟芯片
	CONFIG_TYPE_MEDIA,
	CONFIG_TYPE_INPUT,
	CONFIG_TYPE_OUTPUT,
	CONFIG_TYPE_JPEG,
	CONFIG_TYPE_NAME,
	CONFIG_TYPE_SERIAL,
	CONFIG_TYPE_STORE,
	CONFIG_TYPE_OSDTIME,
	CONFIG_TYPE_OSDTITLE,
	CONFIG_TYPE_OSDPRIVACY,
	CONFIG_TYPE_SENSOR,
	CONFIG_TYPE_SENSE,
	CONFIG_TYPE_OVERLAY,
	CONFIG_TYPE_LOSTSINGLE,
	CONFIG_TYPE_WIRLESS,
	CONFIG_TYPE_VO,
	CONFIG_TYPE_COUNT
}	e_config_type;

/*************************设备参数*************************/
struct DEVHARDPARAM
{
	unsigned int  InputAlarmNum;
	unsigned int  InputAlarmPort[MAX_ALARMINPUTS];		//16个输入报警的gpio端口
	unsigned int  InputAlarmBit[MAX_ALARMINPUTS];		//16个输入报警的gpio bit
	unsigned int  OutputAlarmNum;						//输出报警的个数
	unsigned int  OutputAarmPort[MAX_ALARMOUTPUTS];		//16个输出报警的gpio端口
	unsigned int  OutputAlarmBit[MAX_ALARMOUTPUTS];		//16个输出报警的gpio bit
	unsigned int  VideoAdType;							//视频ad的型号      小于10是标清小于20是720P小于30是960P小于40是1080P小于50是300w小于60是500W
	unsigned int  AudioAiAdType;						//音频ai ad的型号   0=tlv320aic23 1=tw2815 2=aic320aic31
	unsigned int  AudioAoAdType;						//音频ao ad的型号   0=tlv320aic21 1=tw2815 2=aic320aic31
	unsigned int  AudioInputMode;						//音频的输入模式    0=mic         1=line
	unsigned int  VideoChanNum;							//视频通道数
	unsigned int  RecordDeviceType;						//录像设备的类型；
	unsigned int  SD_enable;							//sd卡使能标志
	unsigned int  USB_enable;							//usb设备使能标志;
	unsigned int  SATA_enable;							//SATA设备使能标志
	unsigned int  hardver;								//硬件版本
	unsigned int  softver;								//软件版本
	unsigned int  encodever;							//编码器版本
	unsigned int  LampBit;                              //工作指示灯的位
	unsigned int  LampPort;                             //工作指示灯的端口
	unsigned int  IrCardPort;                           //ircard控制口的组
	unsigned int  IrCardBit;                            //ircard控制口的bit
	unsigned int  LightSensorPort;                      //光敏电阻检测口的组
	unsigned int  LightSensorBit;                       //光敏电阻检测口的bit
	char          DeviceType[20];						//设备类型
	char          OEM_Type[20];                         //OEM型号
	unsigned int  NetWork;
	unsigned int  ModulePowerPort[4];		            //4个模块电源的gpio端口
	unsigned int  ModulePowerbit[4];		            //4个模块电源的gpio bit
	unsigned int  ModuleResetPort[4];		            //4个模块复位的gpio端口
	unsigned int  ModuleResetbit[4];		            //4个模块复位的gpio bit
	unsigned int  ModuleClosePort[4];		            //4个模块关机的gpio端口
	unsigned int  ModuleCLosebit[4];		            //4个模块关机的gpio bit
	unsigned int  ResetKeyPort;                         //恢复出厂设置的按钮的gpio端口
	unsigned int  ResetKeyBit;                          //恢复出厂设置的按钮的gpiobit
	unsigned int  usbControlPort;						//usb控制电源的端口 port
	unsigned int  usbControlBit;						//usb控制电源的端口 bit
	unsigned int  wirelessAlarm;
	char          wifiEth[16];                          //wifi模块使用的设备号
	unsigned int  voltageDetect;                        //是否启用无锡的现实电池电压的功能
	int           vencNumber;                           //手机三码流编码的通道号
	unsigned int  talkCtrl;                             //前端对讲的报警输入通道号设置成100不起作用
	char          infoDevice_1[16];                     //短信查询的卡1设备号
	char          infoDevice_2[16];                     //短信查询的卡2设备号
	unsigned int  uartFlag;								//使用串口的标志，0：485，1：232，尤其对球机既要控制云台又要通过232发送数据
	unsigned int  rtpMode;                              //vlc访问两种模式，一种0：rtsp请求；1：直接rtp发流
	unsigned int  singleton;                            //单兵 将开关量
	unsigned int  tvp5150Reset;							//tvp5150复位功能
	char          doubleModule[4];                      //四个数字两个两个1组分别表示子类型(模块具体型号)和主类型(模块种类跟netWork对应)
	unsigned int  GpsFromModem;                         //双网络时一个模块传视频一个模块传GPS信息
	unsigned int  NetcardType;							//有线使用网卡型号0=rtl8201  1=ksz8041
	unsigned int  P2P_Enable;
}__attribute__((packed));
/*************************网络参数*************************/
struct modem_t
{
    unsigned char   jrh[32];
    unsigned char   usr[32];
    unsigned char   pwd[32];
    unsigned char   vpn[32];
}__attribute__((packed));
struct ctrl_t
{
	char cardEnable;
	char cardNetType;
}__attribute__((packed));
struct DDNSInfo
{
	char user[32];
	char pass[32];
	char server[32];
	unsigned short port;
	char enable;
	char serverType;        							//0-每步；1-花生壳
}__attribute__((packed));
struct NETPARAM											//网络参数
{
    unsigned char   localIP[4];             			//IP地址
    unsigned char   gateIP[4];							//网关
    unsigned char   netMask[4];             			//子网掩码
    unsigned short  dnsPort;                			//DNS端口
    unsigned short	webMediaPort;
    unsigned char   dnsIP[4];               			//DNS服务器IP
    unsigned char   domainName[32];         			//中心(接入服务器)域名
    unsigned char   hostName[32];           			//设备主机名
    unsigned char   username[8];            			//接入服务器登陆用户名
    unsigned char   password[8];            			//接入服务器登陆密码
    unsigned char   pppoeEnable;
    unsigned char   pppoeIPType;
    unsigned char   pppoeUser[30];
    unsigned char   pppoePass[30];
    unsigned char   dhcpEnable;
    unsigned char   wifiEnable;
    unsigned char   wifiNum;
    unsigned char   wifiMode;
    unsigned char   wifiASC;                			//十六进制(0)ASCII(1) TKIP(0) AES(1)
    unsigned char   wifiIP[4];
    unsigned char   wifimask[4];
    unsigned char   wifigateway[4];
    unsigned char   wifiSSID[33];
    unsigned char   wifiCode[33];
    struct modem_t  modem[4];
    unsigned char   autoChange;
    unsigned char   changeTime;
    struct ctrl_t   card_ctrl[2];
    unsigned char   macAddress[32];
	unsigned short  webServicePort;						//web Service端口
	unsigned char   ntpIP[4];
	unsigned short  ntpPort;
	unsigned char	ntpZone[4];
	unsigned int    ntpCycle;
	unsigned char   netTranMode;
	unsigned char   wirelessEnable;
	unsigned short  netSleep;
	unsigned char   telephoneInfo[128];
	unsigned char   bEnablePass; 	  					//dvr 启用密码使能

	struct DDNSInfo ddns;								//ddns
	unsigned char   wifiDhcp;                           //wifi dhcp的启用开关
    unsigned int    routePort;
}__attribute__((packed));
/*************************编码参数*************************/
#define RESO_QCIF 1
#define RESO_CIF 2
#define RESO_HD1 3
#define RESO_D1 4
#define RESO_QVGA 5
#define RESO_VGA 6
#define RESO_720P 7
#define RESO_960P 8
#define RESO_1080P 9
#define RC_VBR 0
#define RC_CBRP 1
struct VENC
{
	char			enable;								//通道是否开启
	unsigned char	resolution;							//图像分辨率:
														//1-QCIF,2-CIF,3-HD1,4-D1,//5-QVGA,6-VGA,//7-720P,8-960P,9-1080P
	unsigned int	frame_rate;							//帧率  1~60
	unsigned int	idr_interval;						//关键帧间隔
	char			rate_ctrl_mode;						//码率控制方式:0-VBR,1-CBR,2-ABR,3-FIXQP
	unsigned int	bitrate;
                                            			//CBR/ABR 模式,表示平均码率。//VBR 模式,表示最大码率。//FIXQP 模式,该值无效。//取值范围:[1, 20000],单位 Kbps。
    char			piclevel;							//图像等级,仅 VBR/CBR 模式下有效。
                                            			//VBR 模式下,表示图像的质量等级。取值范围:[0, 5],值越小,图像质量越好。
                                            			//CBR 模式下,表示码率波动范围。取值范围:[0, 5]。
                                            			//0:自动控制码率,推荐使用。1~5:对应码率波动范围分别为±10%~±50%。
	char			qp_i;								//I 帧 QP。FIXQP 模式下有效。取值范围:[10, 50]。
	char			qp_p;								//P 帧 QP。FIXQP 模式下有效。取值范围:[10, 50]。

	int				rate_statistic;						//码率统计时段。ABR 模式下有效。ABR,即码率短时间波动,长时间平稳。长时间码率的统计,以此时间为准。

	char		    wm_enable;
	char			wm_key[8];
	char			wm_userword[16];
}__attribute__((packed));
struct MEDIAPARAM
{
	unsigned char vFormat;								//视频制式0:P制 1:N制
	unsigned char aFormat;								//音频格式0:G711 1:G726 2:AAC
	struct VENC	main[MAXVIDEOCHNS];
	struct VENC	minor[MAXVIDEOCHNS];
}__attribute__((packed));
/*************************图片参数*************************/
struct JPEGPARAM
{
	unsigned char resolution;							//图像尺寸大小cif,d1,qcif
    unsigned char picLevel;								//量化系数
}__attribute__((packed));
/**********************开关量输入参数**********************/
struct DEFTIME
{
    unsigned short begin[3];                 			//布防的起始时间
    unsigned short end[3];                   			//撤防的停止时间
}__attribute__((packed));
struct PTZACTION
{
    unsigned char enable[16];
	unsigned char pztPosition[16];
}__attribute__((packed));
struct ALMINPUT											//报警输入参数
{
    unsigned char	TypeAlarmor;						//报警器类型(常开=0或常闭=1)
    unsigned char	CallCenter;                         //是否上报中心警方式
    unsigned char	Soundor;                            //声音报警方式
    unsigned char	TypeOutBurst;                       //外部触发报警方式
    unsigned char	OutChan;							//报警器通道
    unsigned char	JpegSheet;							//抓拍张数
    unsigned char	JpegIntv;			                //抓拍间隔
    unsigned int	JpegChn;							//触发抓拍通道选择
    unsigned int	RecChn;				               	//录像通道
    unsigned char	RecDelay;							//录像延时时间
    unsigned char	PreRecord;				            //预录像时间
    unsigned char	Enable;								//是否启用1为启用默认0不启用
    unsigned int	AlmTime;							//输入报警时长
	struct DEFTIME  strategy[7];
	struct PTZACTION ptz;
}__attribute__((packed));
/**********************开关量输出参数**********************/
struct ALMOUTPUT										//报警输出参数
{
	unsigned short AlarmLong;							//报警时长
	unsigned char  Type;								//报警输出方式
	unsigned char  Enable;                              //是否启用1为启用默认0不启用
	struct DEFTIME strategy[7];
}__attribute__((packed));
/*************************名称参数*************************/
struct NAMEINFO
{
    char devName[40];
    char chnName[MAXVIDEOCHNS][40];
	char alarmIn[MAX_ALARMINPUTS][40];
	char alarmOut[MAX_ALARMOUTPUTS][40];
}__attribute__((packed));
struct WEB_USER
{
    char cUser[9];
    char cPass[9];
	char bPTZ;											//控制云台
	char bRecord;										//录像权限
	char bSetParam;										//设置参数
	char bPlayRecord;									//播放录像权限
	char bTools;										//修改工具权限
	char bUser;
}__attribute__((packed));
struct NAMEPARAM										//名称参数
{
	struct NAMEINFO nameInfo;							//设备、通道、报警器等名称

	struct WEB_USER user[16];							//web用户名称
}__attribute__((packed));
/*************************串口参数*************************/
struct PTZ
{
	char Protocol;
	char yuntaiID;
}__attribute__((packed));
struct SERIAL
{
	unsigned short baudRate;							//控制云台波特率
	unsigned char  udata;								//串口参数
	unsigned char  ustop;								// 停止位
	unsigned char  ucheck;								//校验位
	struct PTZ ptz[MAXVIDEOCHNS];
}__attribute__((packed));
/*************************存储参数*************************/
struct STORPARAM
{
    unsigned char   Enable;                 			//存储开关 1:开0:关
    unsigned char   overwrite;              			//循环覆盖存储开关
    unsigned short  SpaceAlarm;             			//空间不足报警阀值
    unsigned int    channel;                			//录像通道选择

    unsigned char   packinterval;						//文件打包间隔
    unsigned char   snapeswitch;						//抓拍存储开关
    unsigned char   alarmswitch;						//传感器报警录像开关
    unsigned char   senseswitch;						//移动侦测录像开关
    unsigned int    diskspace;							//硬盘容量
    unsigned char   StoreCycle[16];         			//存储周期
    unsigned short  valveAlarm;
    unsigned short  saveWithSubChannel;     			//使用子通道进行存储每一位代表一路 0-表示不用子通道 1-表示使用子通道

    struct DEFTIME  strategy[MAXVIDEOCHNS][7];
}__attribute__((packed));
/**********************OSD标题参数*************************/
struct TITLEOSD											//标题OSD显示参数
{
	unsigned short X;
	unsigned short Y;
	unsigned char  Width;
	unsigned char  Height;
	unsigned char  Trans;
	unsigned char  Layer;
	unsigned char  Color[3];
	unsigned char  Enable;
	unsigned short Len;
	unsigned char  Contert[32];
}__attribute__((packed));
/**********************OSD时间参数*************************/
struct TIMEOSD
{
	unsigned char  Switch;								//38-38 时钟显示开关
	unsigned short X;									// x轴座标
	unsigned short Y;									// y轴座标
	unsigned char  Trans;								//透明度
	unsigned char  Layer;								//图层
	unsigned char  Color[3];							//显示颜色
	unsigned char  Format;								//显示格式
	unsigned char  Reso;								//10-10 图像尺寸大小cif,d1,qcif
}__attribute__((packed));
/********************OSD隐私保护参数***********************/
struct PRITOSD
{
	unsigned short X;			  //x轴座标
	unsigned short Y;			  // y轴座标
	unsigned short Width;		  //宽度
	unsigned short Height; 	  //高度
	unsigned char  Type;		  //遮挡区域类型
	unsigned char  Color[3];	  //遮挡区域填充色
}__attribute__((packed));
/*********************图像遮挡参数*************************/
struct OVALM
{
	unsigned char   Level;                                 //报警灵敏度等级0~9   [0=功能关]
	unsigned char   Speed;                                 //防抖动处理，检测速度
	unsigned char   Enable;                                //是否启用
 	unsigned char   CallCenter; 	                       //是否上报中心警方式
	unsigned char   Soundor;    	                       //声音报警方式
	unsigned char   OutChan;		                       //报警器输出通道
	struct DEFTIME  strategy[7];
}__attribute__((packed));
/*********************移动侦测参数*************************/
struct SENSE												//移动侦测参数
{
    unsigned short  RangeStart[2];							//侦测开始坐标
	unsigned short  RangeStop[2];							//侦测停止坐标
	unsigned char   Level;									//报警灵敏度等级0~9
	unsigned char   Speed;									//防抖动处理，检测速度
	unsigned char   Enable;									//是否启用
	unsigned char   CallCenter;								//是否上报中心警方式
    unsigned char   Soundor;								//声音报警方式
    unsigned char   OutChan;								//报警器输出通道
    unsigned char   PreRecord;								//预录像时间
    unsigned char 	JpegSheet;								//抓拍张数
    unsigned char 	JpegIntv;								//抓拍间隔
    unsigned int	JpegChn;								//触发抓拍通道选择
    unsigned int	RecChn;									//8路报警触发录像通道选择
    unsigned char 	RecDelay;								//报警结束后录像延迟时间（秒）
    unsigned char   DrawLine;								//移动报警时画线标志
    unsigned char   NUL[5];
	struct DEFTIME  strategy[7];								//移动侦测布防时间
}__attribute__((packed));
/*********************号丢失报参数*************************/
struct SINGLELOST											//视频信号丢失报警参数
{
	unsigned char  Enable;
	unsigned char  CallCenter;
	unsigned char  Soundor;
	unsigned char  OutChan;
	struct DEFTIME strategy[7];
}__attribute__((packed));
/***********************模组参数***************************/
#define EXPOSURE_COMP		128
struct SENSORPARAM											//模组参数
{
	char resolution;                                        //0-1080P;1-960P;2-720P
    char whiteBalance;                                      //0-自动;1-室内;2-室外;3-预留;4-ATW;5-手动
    char redGain;                                           //0-复位;1-"-";2-"+";3-保持不变
    char blueGain;                                          //0-复位;1-"-";2-"+";3-保持不变
    char focusAuto;                                         //0-自动;1-手动                     聚焦
    char focusChange;                                       //0-停止;1-近;2-远;3-保持不变
    char zoomChange;                                        //0-停止;1-小;2-大;3-保持不变       变倍
    char exposure;                                          //0-自动;1-手动;2-快门优先;3-光圈优先;
    char shutterSpeed;                                      //0-停止;1-慢;2-快;3-保持不变
    char IRISGain;                                          //0-停止;1-小;2-大;3-保持不变
    char exposureGain;                                      //0-停止;1-"-";2-"+";3-保持不变     增益
    char exposureComp;                                      //0-关;1-开
    char exposureCompChange;                                //0-停止;1-"-";2-"+";3-保持不变     背光补偿
    char turn_h;                                            //水平翻转 0-关;1-开
    char turn_v;                                            //垂直翻转 0-关;1-开
    char dayMode;                                           //自动切换 0-关;1-开
    char dayOrNight;                                        //0-夜模式;1-日模式;3-无效
    char apertureChange;                                    //0-停止;1-"-";2-"+";3-保持不变     锐度
    char cameraMode;                                        //0-删除;1-设置预置点;2-预置点调用;3无效
    unsigned char cameraNumber;                             //1-128;预置点号
    char zoomSpeed;

	unsigned short luma;									//亮度
	unsigned short contrast;								//对比度
	unsigned short saturation;								//饱和度
	unsigned short chroma;									//色度或灰度
	unsigned short sharpen;
	unsigned char  resetValue;                              //1-回复默认值使能 0-回复默认值关闭
	unsigned char  drcEnable;                               //宽动态开关0-关 1-开
	unsigned short drcStrength;                             //宽动态强度
	unsigned short denoise2D;                               //2D去噪强度
	unsigned char  dn3DEnable;                              //3D去噪使能
	unsigned short sfStrength;                              //空域去噪强度
	unsigned short tfStrength;                              //时域去噪强度
	unsigned char  antifogEnable;                           //去雾
	unsigned short antifogStrength;                         //去雾强度
	unsigned short rGain;                                   //red gain
	unsigned short gGain;                                   //green gain
	unsigned short bGain; 	                                //blue gain
	unsigned char  exposureTime;
    unsigned int zoomPosition;
}__attribute__((packed));
/***********************视频输出参数***********************/
struct VODEV
{
	unsigned char mode;
	unsigned char serial;
	unsigned char disp_size;
	unsigned char scroll_time;
}__attribute__((packed));
struct VOPARAM
{
	struct VODEV cvbs;
	struct VODEV vga;
}__attribute__((packed));
typedef int (*CONFIG_CALLBACK)(void * pData);

/**********************************************************************/
//module gpio
/**********************************************************************/
typedef enum
{
	GPIO_TYPE_ONCE = 0,									//一次性控制
	GPIO_TYPE_CYCLE,									//周期性控制
	GPIO_TYPE_COUNT	
}	e_gpio_control;

/**********************************************************************/
//module fifo
/**********************************************************************/
typedef enum
{
	FIFO_START_BASE = 0,
	FIFO_START_NEW,
	FIFO_START_SECOND,
	FIFO_START_COUNT
}	e_fifo_start;
typedef enum
{
	FIFO_TYPE_MEDIA = 0,
	FIFO_TYPE_JPEG,
	FIFO_TYPE_COUNT
}	e_fifo_type;
typedef struct st_fifo
{
	e_fifo_type enType;
	int iGrp;
	int iChn;
	
}	st_fifo_t;

typedef enum
{
	FIFO_H264_MAIN = 0,										//主码流
	FIFO_H264_SUB,
	FIFO_H264_EXT,
	FIFO_H264_AUDIO,
	FIFO_H264_COUNT
}	e_fifo_h264;
typedef int (*FIFO_CALLBACK)(char * pFrameData,int iFrameLen);

typedef enum
{
	FIFO_ALARM_SWITCH = 0,									//开关量
	FIFO_ALARM_FAULT,										//故障
	FIFO_ALARM_CAPACITY,									//能力
	FIFO_ALARM_MV,											//移动侦测
	FIFO_ALARM_OD,											//遮挡报警
	FIFO_ALARM_SL,											//信号丢失
	FIFO_ALARM_COUNT
}	e_fifo_alarm;

typedef enum
{
	FIFO_ALARM_READ_CURRENT = 0,							//最新的报警读指针
	FIFO_ALARM_READ_EARLY,									//最早期的报警读指针
	FIFO_ALARM_READ_COUNT
}	e_fifo_read;

typedef struct upload_alarm_info
{
	e_fifo_alarm enAlarm;									//报警的类型
	int iChn;												//通道号
	int iArea;												//子通道，或者区域号
	int iStatus;											//0-停止 1开始
	char cInfo[256];										//附加说明，或者可以存放一些自定义内容
}	st_alarm_upload_t;

/***********************视频输出格式***********************/
typedef enum e_stream_type
{											//类型  0-web stream 1-jpeg stream 2-rtp stream 3-avi stream 4-ts stream 5-ps stream 6-h264 stream
	FIFO_STREAM_RTP = 0,
	FIFO_STREAM_AVI,
	FIFO_STREAM_TS,
	FIFO_STREAM_PS,
	FIFO_FILE_WEB2RTP,
	FIFO_FILE_AVI2RTP,
	FIFO_STREAM_H264,
	FIFO_STREAM_AUDIO,
	FIFO_STREAM_COUNT
}	e_fifo_stream;

/**********************************************************************/
//module net
/**********************************************************************/
typedef int (*NET_RECEIVE_CALLBACK)(int iSock,char *pData,int sock_status);
typedef int (*NET_PROTOCOL_CALLBACK)(int iSock,char *pData,int iLen);
typedef int (*UDP_PROTOCOL_CALLBACK)(int iIp,short sPort,int iSock,char *pData,int iLen);
typedef struct net_card
{
	int sDevice;                            //网卡状态具体见下面描述
	int iIP;
	char devName[16];
}	NET_CARD;

typedef struct net_statue
{
	NET_CARD lan;					        //sDevice 有线网卡状态0-未插网线;1-未找到网卡设备;0x10-10M;0x11-100M;
	NET_CARD adsl;
	NET_CARD wifi;
}	st_net_status_t;

typedef enum e_net_rtsp
{
	NET_RTSP_MEDIA = 0,
	NET_RTSP_FILE,
	NET_RTSP_SETUP,
	NET_RTSP_PLAY,
	NET_RTSP_COUNT
}	e_rtsp_type;

typedef struct st_rtsp_real
{
	char level[128];
	char param[128];
	int  width;
	int  high;
	int  hasAudio;
}	st_rtsp_real_t;

typedef struct st_rtsp_history
{
	char level[128];
	char param[128];
	int  width;
	int  high;
	int  interval;
	int  hasAudio;
}	st_rtsp_history_t;

typedef struct st_rtsp_steup
{
	int  isTcp;
	char cTarget[128];
	int  iClientPort1;
	int  iClientPort2;
	char cSource[128];
	int  iServerPort1;
	int  iServerPort2;
	
}	st_rtsp_setup_t;

typedef struct st_rtsp_play
{
	int  isTcp;
	
	int  iSpeed;
	int  iPullTime;
	char cFileName[128];
	int  v_rtptime;
	int  v_rtpseq;
	int  a_rtptime;
	int  a_rtpseq;
	
}	st_rtsp_play_t;

typedef enum e_encrypt_mode_
{
	NONE = 0,
	WEP,
	WPA,
	WPA2,
	MODE_COUNT
}	e_encrypt_mode;

typedef enum e_encrypt_format_
{
	ASCII = 0,
	HEX,
	TKIP,
	AES,
	FORMAT_COUNT
}	e_encrypt_format;

typedef struct st_wifi_list
{
	char ssid[32];
	char key[32];
	int  enable;							//正在使用的
	e_encrypt_mode encryptMode;				//加密方式
	e_encrypt_format encryptFormat;			//加密格式
	int	 wepPosition;						//wep方式下的密码位置
	int  signalStrength;					//信号强度
	
}	st_wifi_list_t;

/**********************************************************************/
//module Uart
/**********************************************************************/
typedef int (*UART_RECEIVE_CALLBACK)(int handle,char *pData,int iMaxSize);
typedef int (*UART_PROTOCOL_CALLBACK)(int handle,char *pData,int iLen);
typedef struct uart_attr
{
	char ubaud;		//0-9  1200-...
	char udata;
	char ucheck;
	char ustop;
}	st_uart_attr_t;
typedef struct attr_485
{
	char enable;
	char gpio_group;
	char gpio_bit;
}	st_485_ctrl_t;

/**********************************************************************/
//module Utility
/**********************************************************************/
typedef struct ST_IOV
{
	int iov_len;						/**< Size of data */	
	void * iov_base;					/**< Pointer on data */
}__attribute__((packed))st_iov_t;


#ifdef __cplusplus
}
#endif

#endif		//COMMAND_DEFINE_H
