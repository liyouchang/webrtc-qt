#ifndef __RAYCOMM_IPC_PARAMINFO_H__
#define __RAYCOMM_IPC_PARAMINFO_H__


#ifdef __cplusplus
extern "C" {               
#endif

#define VIDEO_NUM           	"video_num"          	//	支持视频流个数 


#define VIDEO1_MAKE_IFRAME       	"video1_makeIframe"	
#define VIDEO2_MAKE_IFRAME       	"video2_makeIframe"	
#define VIDEO3_MAKE_IFRAME       	"video3_makeIframe"	

#define VIDEO1_DATA				"video1_data"
#define VIDEO1_CODEC 			"video1_codec"      	// 	0:H264, 1:MJPG   
#define VIDEO1_BITRATECTRL 		"video1_control"		// 	0:CBR,  1:VBR
#define VIDEO1_BITRATE 			"video1_bitrate"		// 
#define VIDEO1_QUALITY 			"video1_quality"		// 	5:普通,7:较好,9:最好
#define VIDEO1_FRAMERATE 		"video1_framerate"		//	1,5,10,15,20,25,30
#define VIDEO1_IFRAMEINTERVAL 	"video1_int"			// 	1,4,20,100,200
#define VIDEO1_DEINTERLACE 		"video1_deint"			// 	0:close,1:open反交错 
#define VIDEO1_RESOLUTION 		"video1_resolution"		// 	176x144, 352x288, 320x240 640x480 1280x720

#define VIDEO2_ENABLE 			"video2_enable"			// 	0:disable 1:enable
#define VIDEO2_DATA				"video2_data"
#define VIDEO2_CODEC			"video2_codec"			// 	0:H264, 1:MJPG 0:
#define VIDEO2_BITRATECTRL		"video2_control"		// 	0:CBR,  1:VBR
#define VIDEO2_BITRATE 		 	"video2_bitrate"		//
#define VIDEO2_QUALITY 		 	"video2_quality"     	// 	5:普通,7:较好,9:最好
#define VIDEO2_FRAMERATE		"video2_framerate"		//	1,5,10,15,20,25,30,
#define VIDEO2_IFRAMEINTERVAL	"video2_int"			//	1,4,20,100,200
#define VIDEO2_RESOLUTION		"video2_resolution"		//	176x144, 352x288, 320x240 640x480

#define VIDEO3_ENABLE 			"video3_enable"			// 	0:disable 1:enable
#define VIDEO3_DATA			"video3_data"
#define VIDEO3_CODEC 			"video3_codec"      	// 	0:H264, 1:MJPG   
#define VIDEO3_BITRATECTRL 		"video3_control"		// 	0:CBR,  1:VBR
#define VIDEO3_BITRATE 			"video3_bitrate"		// 
#define VIDEO3_QUALITY 			"video3_quality"		// 	5:普通,7:较好,9:最好
#define VIDEO3_FRAMERATE 		"video3_framerate"		//	1,5,10,15,20,25,30
#define VIDEO3_IFRAMEINTERVAL		"video3_int"			// 	1,4,20,100,200
#define VIDEO3_RESOLUTION 		"video3_resolution"		// 	176x144, 352x288, 320x240 640x480 1280x720

#define VIDEO4_ENABLE 			"video4_enable"			// 	0:disable 1:enable
#define VIDEO4_DATA				"video4_data"
#define VIDEO4_CODEC			"video4_codec"			// 	0:H264, 1:MJPG 0:
#define VIDEO4_BITRATECTRL		"video4_control"		// 	0:CBR,  1:VBR
#define VIDEO4_BITRATE 		 	"video4_bitrate"		//
#define VIDEO4_QUALITY 		 	"video4_quality"     	// 	5:普通,7:较好,9:最好
#define VIDEO4_FRAMERATE		"video4_framerate"		//	1,5,10,15,20,25,30,
#define VIDEO4_IFRAMEINTERVAL	"video4_int"			//	1,4,20,100,200
#define VIDEO4_RESOLUTION		"video4_resolution"		//	176x144, 352x288, 320x240 640x480





#define VIDEO_DIRECTION 		"ipc_install_direction"	// 0:正装   1:反装 

#define VIDEO_BRIGHTNESS        "video_brightness"
#define VIDEO_CONTRAST          "video_contrast"
#define VIDEO_SATURATION        "video_saturation"

#define AUDIO_NUM 		"audio_num"				// 支持音频的个数

#define AUDIO1_DATA		"audio_data"
#define RAWAUDIO1_DATA			"rawaudio_data"
#define AUDIO1_ENABLE 			"audio_enable"  		// 0:close , 1:open
#define AUDIO1_CODEC 			"audio_codec"			// 0:ADPCM, 1:G.711
#define AUDIO1_SAMPLERATE 		"audio_samplerate"		// 8000
#define AUDIO1_CHANNEL 			"audio_channel"			// 1

#define TALK_DATA				"talk_data"


#define WAN_IP_ASSIGNMENT 	 	"wan_ip_assignment"		//0:static IP, 1:DHCP, 2:PPPoE
#define WAN_IP 				   	"wan_ip"				//IP地址
#define WAN_GATEWAY			   	"wan_gateway"			//网关
#define WAN_NETMASK    		 	"wan_netmask"			//子网掩码
#define WAN_MTU    			 	"wan_mtu"				//最大可发送的数据包的字节数 512~1500 （Static IP和DHCP接入方式）
#define WAN_PPPOEMTU    	 	"wan_pppoeMtu"			//PPPoE拨号时最大可发送的数据包的字节数
#define WAN_MRU    			 	"wan_mru"				//PPPoE拨号时拨号时最大可接收的数据包的字节数
#define WAN_AUTODNS    		 	"wan_autodns"			//0：手动设置 1：自动设置
#define WAN_DNS1    			"wan_dns1"				//
#define WAN_DNS2    		 	"wan_dns2"				//
#define WAN_PPPOE_PWSD		 	"wan_pppoe_password"	//pppoe用户名
#define WAN_PPPOE_USER		 	"wan_pppoe_username"	//pppoe密码
#define WAN_DDNS_PROTOCOL	 	"ddns_protocol"			//0:raycomm, 1:花生壳协议, 2;禁用DDNS

#define DDNS_USERNAME	 		"ddns_username"			//
#define DDNS_PWSD		 		"ddns_password"			//
#define DDNS_SERVER		 		"ddns_server"			//	
#define DDNS_PERIOD		 		"ddns_period"			//
#define DDNS_SERVER_PORT	 	"ddns_server_port"		//

#define DATE_TIME_ZONE		 	"date_time_zone"		//是否自动与Internet时间服务器同步 0:禁用, 1:启用
#define DATE_TIME_SERVER	 	"date_time_server"		//
#define DATE_TIME_PERIOD	 	"date_time_period"		//	
#define DATE_TIME_AUTO		 	"date_time_auto"		//

#define UPDATE_AUTO		 		"update_auto"			//是否自动更新固件 0 禁用， 1 启用
#define UPDATE_SERVER			"update_server"			//
#define UPDATE_INT				"update_int"			//

#define WIFI_MODE				"wifi_mode"  			//无线功能   0 禁用  1 启用
#define WIFI_SSID				"wifi_ssid" 			//=Raycomm
#define WIFI_PWSD				"wifi_passwd"			//
#define WIFI_AUTH_TYPE			"wifi_auth_type"		//0：NONE 1：WEP 2：WPA-PSK 3：WPA2-PAK
#define WIFI_WEP_ENCRYPT_PROTCL		"wifi_wep_encrypt_protocol"		//0：OPEN开放系统 1：SHARED共享密钥
#define WIFI_PSK_ENCRYPT_PROTCL		"wifi_psk_encrypt_protocol"		//0：TKIP 1：AES

#define UPNP_ENABLE				"upnp_enable"			//0：禁用UPNP 1：启用UPNP			
#define UPNP_UPDATE_CYCLE		"upnp_update_cycle"		//

#define RTSP_ENABLE        		"rtsp_enable"			//0 禁用 1 启用
#define RTSP_PORT				"rtsp_port"				//rtsp端口
#define RTSP_PROTCL				"rtsp_protocol"			//流媒体服务协议 0：TCP 1：UDP

#define WEB_ENABLE				"web_enable"
#define WEB_PORT				"web_port"				//
#define WEB_MEIDA_PORT			"web_meida_port"		//RTMP端口
	

#define STORAGE_STATUS			"storage_status"		    //0:no storageDev , 1:have storageDev
#define STORAGE_DEVICE_SIZE		"storage_device_size"

#define STORAGE_DEV				"storage_dev"			//
#define STORAGE_WAY				"storage_way"			//
#define STORAGE_ISCSI_SERVER	"storage_iscsi_server"	//
#define STORAGE_ISCSI_IQN		"storage_iscsi_iqn"		//
#define STORAGE_STARTTM			"storage_starttm"		//
#define STORAGE_ENDTM			"storage_endtm"			//
#define STORAGE_REPEAT			"storage_repeat"		//
#define STORAGE_MAXSIZE			"storage_maxsize"		//
#define STORAGE_FORMAT			"storage_format"		//

#define OSD_TIMEENABLE			"osd_timeenable"		//	
#define OSD_TIMEFORMAT			"osd_timeformat"		//
#define OSD_TEXTENABLE			"osd_textenable"		//
#define OSD_TEST				"osd_text"				//
#define OSD_ALPHA				"osd_alpha"				//
#define OSD_X					"osd_x"					//
#define OSD_Y					"osd_y"					//
#define OSD_X1					"osd_x1"				//
#define OSD_Y1					"osd_y1"				//
#define OSD_COLOR1				"osd_color1"			//
#define OSD_COLOR2				"osd_color2"			//
#define OSD_HEIGHT1				"osd_height1"			//
#define OSD_HEIGHT2				"osd_height2"			//

#define PTZ_PROTOCOL 		"ptz_protocol"				// 0:PELCOD , 1:PELCOP , 2:raycomm, 3: disable
#define PTZ_SPEED 			"ptz_speed"					// 0~4,对应 1到5档
#define PTZ_PORT			"ptz_port"					// ptz服务器端口
#define PTZ_BAUDRATE		"ptz_baudrate"				// 
#define PTZ_ADDRESS			"ptz_address"				// ptz服务器ip地址


#define USER_NUM     	 	"user_num"					//
#define USER_NAME0		 	"user_name0"				//
#define USER_PASSWD0	 	"user_passwd0"				//
#define USER_OP0		 	"user_op0"					//
#define USER_NAME1		 	"user_name1"				//
#define USER_PASSWD1	 	"user_passwd1"				//
#define USER_OP1		 	"user_op1"					//
#define USER_NAME2		 	"user_name2"				//
#define USER_PASSWD2	 	"user_passwd2"				//
#define USER_OP2		 	"user_op2"					//

#define STREAM_SERV_PORT	"stream_server_port"		//
#define TELNETD_ENABLE		"telnetd_enable"			//

#define FIRMWARE_VERSION    "firmware_vserion"
#define HARDWARE_ID         "hardware_id"
#define MANUFACTURER        "manufacturer"
#define MODEL               "model"
#define SERIAL_NUMBER       "serial_number"

#define SYSTEM_REBOOT       "system_reboot"
#define WS_DISCOVERY_ENABLE       "ws_discovery_enable"
#define MOTION_DETECTION    "alarm_0_enable"			//动态侦测 开启1  关闭0





#define MAX_NVR_QUERYNUM   30
#define NVR_PATH_LEN   128
#define NVR_DATE_LEN   16

typedef struct
{ 
	char path[NVR_PATH_LEN];
	char date[NVR_DATE_LEN]; //2013-04-25T11:12:00Z
	int  size; 
}t_NVR_FileInfo, *pt_NVR_FileInfo;


typedef struct
{ 
	int file_num;
	t_NVR_FileInfo nvr_file[MAX_NVR_QUERYNUM];

}t_NVR_QueryInfo, *pt_NVR_QueryInfo;


#define RAYCOMM_WIFI_NET_INFRA	0
#define RAYCOMM_WIFI_NET_ADHOC	1

#define RAYCOMM_WIFI_AUTH_NONE	0
#define RAYCOMM_WIFI_AUTH_WEP	1
#define RAYCOMM_WIFI_AUTH_WPA	2
#define RAYCOMM_WIFI_AUTH_WPA2	3

#define RAYCOMM_WIFI_ENC_TKIP	0
#define RAYCOMM_WIFI_ENC_AES	1




#define MAX_WIFI_LISTNUM   30
typedef struct
{
	char sSsID[32];
	char sKey[32];
	unsigned int u32Enable;
	unsigned int u32Auth;
	unsigned int u32Enc;
	unsigned int u32Mode;
}t_WIFI_PARAM, *pt_WIFI_PARAM;

typedef struct
{
	char sSsID[32];
	unsigned int u32Quality;
	unsigned int u32Auth;
	unsigned int u32Enc;
	unsigned int u32Mode;
}t_WIFI_INFO, *pt_WIFI_INFO;


int Raycomm_QueryNVR(char* start_day, char* end_day, pt_NVR_QueryInfo pt_nvr_queryinfo, int offset, int usCount) ;

int Raycomm_SetWifi(pt_WIFI_PARAM params);

int Raycomm_GetWifiList(int* pCount, pt_WIFI_INFO wifi);
//************************************
// Method:    Raycomm_InitParam
// Returns:   返回错误码 0 成功 -1 失败
// Description: 获取参数初始化
// Parameter: [in]void
//************************************
int	Raycomm_InitParam();



//************************************
// Method:    Raycomm_UnInitParam
// Returns:   返回错误码 0 成功 -1 失败
// Description: 释放初始化资源
// Parameter: [in]void
//************************************
int Raycomm_UnInitParam();



//************************************
// Method:    Raycomm_GetParam
// Returns:   返回错误码 0 成功 -1 失败
// Description: 根据关键字 获取参数
// Parameter: [in]char *keyword    	关键字
// Parameter: [out]char *buf		返回参数字符串
//************************************
int Raycomm_GetParam(char *keyword, char *buf, int cam_id);




//************************************
// Method:    Raycomm_SetParam
// Returns:   返回错误码 0 成功 -1 失败
// Description: 根据命令 设置参数
// Parameter: [in]char *command    	关键字组合的命令字符串
//************************************
int Raycomm_SetParam(char *command, int cam_id);



#ifdef __cplusplus
}
#endif


#endif
