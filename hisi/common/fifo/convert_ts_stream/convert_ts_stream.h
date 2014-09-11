#ifndef __MY_CONVERT_TS_H__
#define __MY_CONVERT_TS_H__

#include "../fifo.h"
#include "../../common_define.h"

#define TS_PACKET_SIZE					188
#define TS_MAX_OUT_BUFF					200*1024
#define TS_SYNC_BYTE					0x47
#define TS_PAT_PID						0x00
#define TS_PMT_PID						0x81
#define TS_H264_PID						0x810
#define TS_AAC_PID						0x814
#define TS_H264_STREAM_ID				0xE0
#define TS_AAC_STREAM_ID				0xC0
#define	FPS								25

typedef struct Tag_PacketHeader
{
    unsigned sync_byte							: 8;
    unsigned transport_error_indicator			: 1;
    unsigned payload_unit_start_indicator		: 1;
    unsigned transport_priority					: 1;
    unsigned PID								: 13;
    unsigned transport_scrambling_control		: 2;
    unsigned adaption_field_control				: 2;
    unsigned continuity_counter					: 4;
}__attribute__((packed))TsPacketHeader;

typedef struct Tag_TsPat
{
    unsigned table_id							: 8;
    unsigned section_syntax_indicator			: 1;
    unsigned zero								: 1;
    unsigned reserved_1							: 2;
    unsigned section_length						: 12;
    unsigned transport_stream_id				: 16;
    unsigned reserved_2							: 2;
    unsigned version_number						: 5;
    unsigned current_next_indicator				: 1;
    unsigned section_number						: 8;
    unsigned last_section_number				: 8;
    	
	unsigned int program_number					: 16;			//节目号
	unsigned char reserved_3					: 3;			//保留位
	unsigned int program_map_PID				: 13;			//节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个
	unsigned long CRC_32						: 32;			//CRC32校验码
} __attribute__((packed))TsPat;

typedef struct TS_PMT
{
    unsigned table_id							: 8;
    unsigned section_syntax_indicator			: 1;
    unsigned zero								: 1;
    unsigned reserved_1							: 2;
    unsigned section_length						: 12;
    unsigned program_number						: 16;
    unsigned reserved_2							: 2;
    unsigned version_number						: 5;
    unsigned current_next_indicator				: 1;
    unsigned section_number						: 8;
    unsigned last_section_number				: 8;
    unsigned reserved_3							: 3;
    unsigned PCR_PID							: 13;
    unsigned reserved_4							: 4;
    unsigned program_info_length				: 12;
   
	unsigned char stream_type_video				: 8;			//指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	unsigned char reserved_5_video				: 3;			//0x07
	unsigned int elementary_PID_video			: 13;			//该域指示TS包的PID值。这些TS包含有相关的节目元素
	unsigned char reserved_6_video				: 4;			//0x0F
	unsigned int ES_info_length_video			: 12;			//前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
	unsigned char stream_type_audio				: 8;			//指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	unsigned char reserved_5_audio				: 3;			//0x07
	unsigned int elementary_PID_audio			: 13;			//该域指示TS包的PID值。这些TS包含有相关的节目元素
	unsigned char reserved_6_audio				: 4;			//0x0F
	unsigned int ES_info_length_audio			: 12;			//前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
	unsigned long CRC_32						: 32;			//CRC32校验码
} __attribute__((packed))TsPmt;

//PTS_DTS结构体：本程序设置都有 “11”
typedef struct Tag_TsPtsDts
{
	unsigned char reserved_1 : 4;
	unsigned char pts_32_30  : 3;                //显示时间戳
	unsigned char marker_bit1: 1;
	unsigned int  pts_29_15 : 15;
	unsigned char marker_bit2 : 1;
	unsigned int  pts_14_0 : 15;
	unsigned char marker_bit3 :1 ;
	unsigned char reserved_2 : 4;
	unsigned char dts_32_30: 3;                  //解码时间戳
	unsigned char marker_bit4 :1;
	unsigned int  dts_29_15 :15;
	unsigned char marker_bit5: 1;
	unsigned int  dts_14_0 :15;
	unsigned char marker_bit6 :1 ;
}__attribute__((packed))TsPtsDts;

//PES包结构体，包括PES包头和ES数据 ,头 19 个字节
typedef struct Tag_TsPes
{
	unsigned int   packet_start_code_prefix : 24;//起始：0x000001
	unsigned char  stream_id : 8;                //基本流的类型和编号
	unsigned int   PES_packet_length : 16;       //包长度,就是帧数据的长度，可能为0,要自己算
	unsigned char  marker_bit:2;                 //必须是：'10'
	unsigned char  PES_scrambling_control:2;     //pes包有效载荷的加扰方式
	unsigned char  PES_priority:1;               //有效负载的优先级
	unsigned char  data_alignment_indicator:1;   //如果设置为1表明PES包的头后面紧跟着视频或音频syncword开始的代码。
	unsigned char  copyright:1;                  //1:靠版权保护，0：不靠
	unsigned char  original_or_copy:1;           //1;有效负载是原始的，0：有效负载时拷贝的
	unsigned char  PTS_DTS_flags:2;              //'10'：PTS字段存在，‘11’：PTD和DTS都存在，‘00’：都没有，‘01’：禁用。
	unsigned char  ESCR_flag:1;                  //1:escr基准字段 和 escr扩展字段均存在，0：无任何escr字段存在
	unsigned char  ES_rate_flag:1;               //1:es_rate字段存在，0 ：不存在
	unsigned char  DSM_trick_mode_flag:1;        //1;8比特特接方式字段存在，0 ：不存在
	unsigned char  additional_copy_info_flag:1;  //1:additional_copy_info存在，0: 不存在
	unsigned char  PES_CRC_flag:1;               //1:crc字段存在，0：不存在
	unsigned char  PES_extension_flag:1;         //1：扩展字段存在，0：不存在
	unsigned char  PES_header_data_length :8;    //后面数据的长度，
	TsPtsDts       tsptsdts;                     //ptsdts结构体对象，10个字节
	unsigned char  *Es;							 //一帧 原始数据
	unsigned int   Pes_Packet_Length_Beyond;
}__attribute__((packed))TsPes;

//自适应段标志
typedef struct Tag_Ts_Adaptation_field
{
	unsigned char discontinuty_indicator:1;                //1表明当前传送流分组的不连续状态为真
	unsigned char random_access_indicator:1;               //表明下一个有相同PID的PES分组应该含有PTS字段和一个原始流访问点
	unsigned char elementary_stream_priority_indicator:1;  //优先级
	unsigned char PCR_flag:1;                              //包含pcr字段
	unsigned char OPCR_flag:1;                             //包含opcr字段
	unsigned char splicing_point_flag:1;                   //拼接点标志       
	unsigned char transport_private_data_flag:1;           //私用字节
	unsigned char adaptation_field_extension_flag:1;       //调整字段有扩展

	unsigned char adaptation_field_length;                 //自适应段长度
	unsigned long long  pcr;                               //自适应段中用到的的pcr
	unsigned long long  opcr;                              //自适应段中用到的的opcr
	unsigned char splice_countdown;
	unsigned char private_data_len;
	unsigned char private_data [256];
}Ts_Adaptation_field;


extern void *convert_ts_stream(void * cvt);

#endif	//__MY_CONVERT_TS_H__
