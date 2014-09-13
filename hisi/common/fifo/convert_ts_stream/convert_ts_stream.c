#include "convert_ts_stream.h"

static unsigned long crc_table[256] = 
{
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};
/******************************************************************************/
//crc32
/******************************************************************************/
unsigned long crc32 (char *data, int len)
{
	int i;
	unsigned long crc = 0xffffffff;
	for (i=0; i<len; i++) 
		crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *data++) & 0xff];
	return (~(crc^0xFFFFFFFF));
}

/******************************************************************************/
//TsHeader2buffer
/******************************************************************************/
TsPacketHeader * CreateTsHeader(TsPacketHeader * ts_header,unsigned int PID,unsigned char play_init,unsigned char ada_field_C,unsigned char conti_cter)
{
	ts_header->sync_byte = TS_SYNC_BYTE;
	ts_header->transport_error_indicator = 0x00;
	ts_header->payload_unit_start_indicator = play_init;
	ts_header->transport_priority = 0x00;
	ts_header->PID = PID;
	ts_header->transport_scrambling_control = 0x00;
	ts_header->adaption_field_control = ada_field_C;
	ts_header->continuity_counter = conti_cter;
	return ts_header;
}

int TsHeader2buffer(TsPacketHeader * ts_header,unsigned char *buffer)
{
	buffer[0]=ts_header->sync_byte;
	buffer[1]=ts_header->transport_error_indicator<<7|ts_header->payload_unit_start_indicator<<6|ts_header->transport_priority<<5|((ts_header->PID>>8)&0x1f);
	buffer[2]=(ts_header->PID&0x00ff);
	buffer[3]=ts_header->transport_scrambling_control<<6|ts_header->adaption_field_control<<4|ts_header->continuity_counter;
	return 1;
}

int iPoint=0;
int tsWrite(st_fifoStream_t * pFifoTs,char * buf,int iLen)
{
	memcpy(pFifoTs->stFifoData[pFifoTs->iWrite].pData+iPoint,buf,iLen);
	pFifoTs->stFifoData[pFifoTs->iWrite].iLen += iLen;
	iPoint += iLen;
	if (iPoint >= 940)
	{
		iPoint = 0;
		if ((pFifoTs->iWrite+1) >= FIFO_MAX_BUFFER)
			pFifoTs->iWrite=0;
		else
			pFifoTs->iWrite++;
	}
	return 0;
}
unsigned long Timestamp_video=0;
unsigned long Timestamp_audio=0;
/******************************************************************************/
//CreatePAT
/******************************************************************************/
int CreatePAT(st_fifoStream_t * pFifoTs,int iPatNum)
{
	TsPacketHeader * ts_header;
	ts_header = (TsPacketHeader *)malloc(sizeof(TsPacketHeader));
	if (ts_header == NULL)
		return 0;
	unsigned char TSbuf[TS_PACKET_SIZE];
	memset(TSbuf,0xff,TS_PACKET_SIZE);
	ts_header = CreateTsHeader(ts_header,TS_PAT_PID,0x01,0x01,iPatNum);         //PID = 0x00,有效荷载单元起始指示符_play_init = 0x01, ada_field_C,0x01,仅有有效负载，conti_cter 计数为 0 ；
	TsHeader2buffer(ts_header,TSbuf);
	free(ts_header);
	
	TsPat ts_pat;
	unsigned long PAT_CRC = 0xFFFFFFFF; 
	ts_pat.table_id = 0x00;
	ts_pat.section_syntax_indicator = 0x01;
	ts_pat.zero = 0x00;
	ts_pat.reserved_1 = 0x03;                                               //设置为11；
	ts_pat.section_length = 0x0d;                                           //pat结构体长度 16个字节减去上面的3个字节
	ts_pat.transport_stream_id = 0x01;
	ts_pat.reserved_2 = 0x03;                                               //设置为11；
	ts_pat.version_number = 0x00;
	ts_pat.current_next_indicator = 0x01;                                   //当前的pat 有效
	ts_pat.section_number = 0x00;
	ts_pat.last_section_number = 0x00;
	ts_pat.program_number = 0x01;
	ts_pat.reserved_3 = 0x07;                                               //设置为111；
	ts_pat.program_map_PID = TS_PMT_PID;                                    //PMT的PID
	ts_pat.CRC_32 = PAT_CRC;                                                //传输过程中检测的一种算法值 先设定一个填充值
	
	unsigned char * pointer_pat=TSbuf;
	pointer_pat[4] = 0;                                                      //自适应段的长度为0
	pointer_pat += 5;
	pointer_pat[0] = ts_pat.table_id;
	pointer_pat[1] = ts_pat.section_syntax_indicator << 7 | ts_pat.zero  << 6 | ts_pat.reserved_1 << 4 | ((ts_pat.section_length >> 8) & 0x0F);
	pointer_pat[2] = ts_pat.section_length & 0x00FF;
	pointer_pat[3] = ts_pat.transport_stream_id >> 8;
	pointer_pat[4] = ts_pat.transport_stream_id & 0x00FF;
	pointer_pat[5] = ts_pat.reserved_2 << 6 | ts_pat.version_number << 1 | ts_pat.current_next_indicator;
	pointer_pat[6] = ts_pat.section_number;
	pointer_pat[7] = ts_pat.last_section_number;
	pointer_pat[8] = ts_pat.program_number>>8;
	pointer_pat[9] = ts_pat.program_number & 0x00FF;
	pointer_pat[10]= ts_pat.reserved_3 << 5 | ((ts_pat.program_map_PID >> 8) & 0x0F);
	pointer_pat[11]= ts_pat.program_map_PID & 0x00FF;

	PAT_CRC = crc32 ((char *)TSbuf+5,12);
	pointer_pat[12]= PAT_CRC >> 24;
	pointer_pat[13]= PAT_CRC >> 16;
	pointer_pat[14]= PAT_CRC >> 8;
	pointer_pat[15]= PAT_CRC & 0x000000FF;
	
	tsWrite(pFifoTs,(char *)TSbuf,188);
	
	return 188;
}

int CreatePMT(st_fifoStream_t * pFifoTs,int iPatNum)
{
	TsPacketHeader * ts_header;
	ts_header = (TsPacketHeader *)malloc(sizeof(TsPacketHeader));
	if (ts_header == NULL)
		return 0;
	unsigned char TSbuf[TS_PACKET_SIZE];
	memset(TSbuf,0xff,TS_PACKET_SIZE);
	ts_header = CreateTsHeader(ts_header,TS_PMT_PID,0x01,0x01,iPatNum);         //PID = 0x00,有效荷载单元起始指示符_play_init = 0x01, ada_field_C,0x01,仅有有效负载，conti_cter 计数为 0 ；
	TsHeader2buffer(ts_header,TSbuf);
	free(ts_header);
	
	TsPmt ts_pmt;
	unsigned long PMT_CRC = 0xFFFFFFFF;
	ts_pmt.table_id = 0x02;
	ts_pmt.section_syntax_indicator = 0x01;
	ts_pmt.zero = 0x00;
	ts_pmt.reserved_1 = 0x03;
	ts_pmt.section_length = 0x17;                                           //PMT结构体长度 16 + 5 + 5个字节减去上面的3个字节
	ts_pmt.program_number = 01;                                             //只有一个节目
	ts_pmt.reserved_2 = 0x03;
	ts_pmt.version_number = 0x00;
	ts_pmt.current_next_indicator = 0x01;                                   //当前的PMT有效
	ts_pmt.section_number = 0x00;
	ts_pmt.last_section_number = 0x00;
	ts_pmt.reserved_3 = 0x07;
	ts_pmt.PCR_PID = TS_H264_PID ;                                          //视频PID                                   
	ts_pmt.reserved_4 = 0x0F;
	ts_pmt.program_info_length = 0x00;                                      //后面无 节目信息描述
	ts_pmt.stream_type_video = 0x1B;                                        //视频的类型
	ts_pmt.reserved_5_video = 0x07;
	ts_pmt.elementary_PID_video = TS_H264_PID;                              //视频的PID
	ts_pmt.reserved_6_video= 0x0F;
	ts_pmt.ES_info_length_video = 0x00;                                     //视频无跟随的相关信息
	ts_pmt.stream_type_audio = 0x0f;                                        //音频类型
	ts_pmt.reserved_5_audio = 0x07;
	ts_pmt.elementary_PID_audio = TS_AAC_PID;                               //音频PID 
	ts_pmt.reserved_6_audio = 0x0F;
	ts_pmt.ES_info_length_audio = 0x00;                                     //音频无跟随的相关信息
	ts_pmt.CRC_32 = PMT_CRC; 

	unsigned char * pointer_pmt=TSbuf;
	pointer_pmt[4] = 0;                                                      //自适应段的长度为0
	pointer_pmt += 5;
	pointer_pmt[0] = ts_pmt.table_id;
	pointer_pmt[1] = ts_pmt.section_syntax_indicator << 7 | ts_pmt.zero  << 6 | ts_pmt.reserved_1 << 4 | ((ts_pmt.section_length >> 8) & 0x0F);
	pointer_pmt[2] = ts_pmt.section_length & 0x00FF;
	pointer_pmt[3] = ts_pmt.program_number >> 8;
	pointer_pmt[4] = ts_pmt.program_number & 0x00FF;
	pointer_pmt[5] = ts_pmt.reserved_2 << 6 | ts_pmt.version_number << 1 | ts_pmt.current_next_indicator;
	pointer_pmt[6] = ts_pmt.section_number;
	pointer_pmt[7] = ts_pmt.last_section_number;
	pointer_pmt[8] = ts_pmt.reserved_3 << 5  | ((ts_pmt.PCR_PID >> 8) & 0x1F);
	pointer_pmt[9] = ts_pmt.PCR_PID & 0x0FF;
	pointer_pmt[10]= ts_pmt.reserved_4 << 4 | ((ts_pmt.program_info_length >> 8) & 0x0F);
	pointer_pmt[11]= ts_pmt.program_info_length & 0xFF;
	pointer_pmt[12]= ts_pmt.stream_type_video;                               //视频流的stream_type
	pointer_pmt[13]= ts_pmt.reserved_5_video << 5 | ((ts_pmt.elementary_PID_video >> 8 ) & 0x1F);
	pointer_pmt[14]= ts_pmt.elementary_PID_video & 0x00FF;
	pointer_pmt[15]= ts_pmt.reserved_6_video<< 4 | ((ts_pmt.ES_info_length_video >> 8) & 0x0F);
	pointer_pmt[16]= ts_pmt.ES_info_length_video & 0x0FF;
	pointer_pmt[17]= ts_pmt.stream_type_audio;                               //视频流的stream_type
	pointer_pmt[18]= ts_pmt.reserved_5_audio<< 5 | ((ts_pmt.elementary_PID_audio >> 8 ) & 0x1F);
	pointer_pmt[19]= ts_pmt.elementary_PID_audio & 0x00FF;
	pointer_pmt[20]= ts_pmt.reserved_6_audio << 4 | ((ts_pmt.ES_info_length_audio >> 8) & 0x0F);
	pointer_pmt[21]= ts_pmt.ES_info_length_audio & 0x0FF;

	PMT_CRC = crc32 ((char *)TSbuf+5,22);
	pointer_pmt[22]= PMT_CRC >> 24;
	pointer_pmt[23]= PMT_CRC >> 16;
	pointer_pmt[24]= PMT_CRC >> 8;
	pointer_pmt[25]= PMT_CRC & 0x000000FF;
	
	tsWrite(pFifoTs,(char *)TSbuf,188);
	
	return 188;
}

FILE * tsFd=NULL;
Ts_Adaptation_field  ts_adaptation_field_Head; 
Ts_Adaptation_field  ts_adaptation_field_Tail;
int WriteAdaptive_flags_Head(Ts_Adaptation_field * ts_adaptation_field,unsigned int Videopts)
{
	//填写自适应段
	ts_adaptation_field->discontinuty_indicator = 0;
	ts_adaptation_field->random_access_indicator = 0;
	ts_adaptation_field->elementary_stream_priority_indicator = 0;
	ts_adaptation_field->PCR_flag = 1;                                          //只用到这个
	ts_adaptation_field->OPCR_flag = 0;
	ts_adaptation_field->splicing_point_flag = 0;
	ts_adaptation_field->transport_private_data_flag = 0;
	ts_adaptation_field->adaptation_field_extension_flag = 0;

	//需要自己算
	ts_adaptation_field->pcr  = Videopts;
	ts_adaptation_field->adaptation_field_length = 7;                          //占用7位

	ts_adaptation_field->opcr = Videopts;
	ts_adaptation_field->splice_countdown = 0;
	ts_adaptation_field->private_data_len = 0;
	return 1;
}

int WriteAdaptive_flags_Tail(Ts_Adaptation_field  * ts_adaptation_field)
{
	//填写自适应段
	ts_adaptation_field->discontinuty_indicator = 0;
	ts_adaptation_field->random_access_indicator = 0;
	ts_adaptation_field->elementary_stream_priority_indicator = 0;
	ts_adaptation_field->PCR_flag = 0;                                          //只用到这个
	ts_adaptation_field->OPCR_flag = 0;
	ts_adaptation_field->splicing_point_flag = 0;
	ts_adaptation_field->transport_private_data_flag = 0;
	ts_adaptation_field->adaptation_field_extension_flag = 0;

	//需要自己算
	ts_adaptation_field->pcr  = 0;
	ts_adaptation_field->adaptation_field_length = 1;                          //占用1位标志所用的位

	ts_adaptation_field->opcr = 0;
	ts_adaptation_field->splice_countdown = 0;
	ts_adaptation_field->private_data_len = 0;                    
	return 1;
}

int CreateAdaptive_Ts(Ts_Adaptation_field * ts_adaptation_field,unsigned char * buf,unsigned int AdaptiveLength)
{
	unsigned int CurrentAdaptiveLength = 1;                                 //当前已经用的自适应段长度  
	unsigned char Adaptiveflags = 0;                                        //自适应段的标志
	unsigned int adaptive_pos = 0;

	//填写自适应字段
	if (ts_adaptation_field->adaptation_field_length > 0)
	{
		adaptive_pos += 1;                                                  //自适应段的一些标志所占用的1个字节
		CurrentAdaptiveLength += 1;

		if (ts_adaptation_field->discontinuty_indicator)
		{
			Adaptiveflags |= 0x80;
		}
		if (ts_adaptation_field->random_access_indicator)
		{
			Adaptiveflags |= 0x40;
		}
		if (ts_adaptation_field->elementary_stream_priority_indicator)
		{
			Adaptiveflags |= 0x20;
		}
		if (ts_adaptation_field->PCR_flag)
		{
			unsigned long long pcr_base;
			unsigned int pcr_ext;

			pcr_base = (ts_adaptation_field->pcr);
			pcr_ext = (ts_adaptation_field->pcr%300);
			Adaptiveflags |= 0x10;

			buf[adaptive_pos + 0] = (pcr_base >> 25) & 0xff;
			buf[adaptive_pos + 1] = (pcr_base >> 17) & 0xff;
			buf[adaptive_pos + 2] = (pcr_base >> 9) & 0xff;
			buf[adaptive_pos + 3] = (pcr_base >> 1) & 0xff;
			buf[adaptive_pos + 4] = pcr_base << 7 | pcr_ext >> 8 | 0x7e;
			buf[adaptive_pos + 5] = (pcr_ext) & 0xff;
			adaptive_pos += 6;

			CurrentAdaptiveLength += 6;
		}
		if (ts_adaptation_field->OPCR_flag)
		{
			unsigned long long opcr_base;
			unsigned int opcr_ext;

			opcr_base = (ts_adaptation_field->opcr);
			opcr_ext = (ts_adaptation_field->opcr%300);
			Adaptiveflags |= 0x08;

			buf[adaptive_pos + 0] = (opcr_base >> 25) & 0xff;
			buf[adaptive_pos + 1] = (opcr_base >> 17) & 0xff;
			buf[adaptive_pos + 2] = (opcr_base >> 9) & 0xff;
			buf[adaptive_pos + 3] = (opcr_base >> 1) & 0xff;
			buf[adaptive_pos + 4] = ((opcr_base << 7) & 0x80) | ((opcr_ext >> 8) & 0x01);
			buf[adaptive_pos + 5] = (opcr_ext) & 0xff;
			adaptive_pos += 6;
			CurrentAdaptiveLength += 6;
		}
		if (ts_adaptation_field->splicing_point_flag)
		{
			buf[adaptive_pos] = ts_adaptation_field->splice_countdown;

			Adaptiveflags |= 0x04;

			adaptive_pos += 1;
			CurrentAdaptiveLength += 1;
		}
		if (ts_adaptation_field->private_data_len > 0)
		{
			Adaptiveflags |= 0x02;
			if (1+ ts_adaptation_field->private_data_len > AdaptiveLength - CurrentAdaptiveLength)
			{
				printf("private_data_len error !\n");
				return getchar();
			}
			else
			{
				buf[adaptive_pos] = ts_adaptation_field->private_data_len;
				adaptive_pos += 1;
				memcpy (buf + adaptive_pos, ts_adaptation_field->private_data, ts_adaptation_field->private_data_len);
				adaptive_pos += ts_adaptation_field->private_data_len;

				CurrentAdaptiveLength += (1 + ts_adaptation_field->private_data_len) ;
			}
		}
		if (ts_adaptation_field->adaptation_field_extension_flag)
		{
			Adaptiveflags |= 0x01;
			buf[adaptive_pos + 1] = 1;
			buf[adaptive_pos + 2] = 0;
			CurrentAdaptiveLength += 2;
		}
		buf[0] = Adaptiveflags;                        //将标志放入内存
	}
	return 1;
}

int WriteStruct_Packetheader(unsigned char * Buf,unsigned int PID,unsigned char play_init,unsigned char ada_field_C,int iPatNum)
{
	TsPacketHeader tspacketheader;

	tspacketheader.sync_byte = TS_SYNC_BYTE;
	tspacketheader.transport_error_indicator = 0x00;
	tspacketheader.payload_unit_start_indicator = play_init;
	tspacketheader.transport_priority = 0x00;
	tspacketheader.PID = PID;
	tspacketheader.transport_scrambling_control = 0x00;
	tspacketheader.adaption_field_control = ada_field_C;
	tspacketheader.continuity_counter = (iPatNum%16);
	
	Buf[0] = tspacketheader.sync_byte;
	Buf[1] = tspacketheader.transport_error_indicator << 7 | tspacketheader.payload_unit_start_indicator << 6  | tspacketheader.transport_priority << 5 | ((tspacketheader.PID >> 8) & 0x1f);
	Buf[2] = (tspacketheader. PID & 0x00ff);
	Buf[3] = tspacketheader.transport_scrambling_control << 6 | tspacketheader.adaption_field_control << 4 | tspacketheader.continuity_counter;
	return 4;
}

int PES2TS(st_fifoStream_t * pFifoTs,TsPes * ts_pes,Ts_Adaptation_field * ts_adaptation_field_Head ,Ts_Adaptation_field * ts_adaptation_field_Tail,unsigned long i_pts)
{
	unsigned int ts_pos = 0;
	unsigned int FirstPacketLoadLength = 0 ;                                   //分片包的第一个包的负载长度
	unsigned int NeafPacketCount = 0;                                          //分片包的个数
	unsigned int AdaptiveLength = 0;                                           //要填写0XFF的长度
	unsigned char * NeafBuf = NULL;                                            //分片包 总负载的指针
	unsigned char TSbuf[TS_PACKET_SIZE];

	memset(TSbuf,0,TS_PACKET_SIZE); 
	FirstPacketLoadLength = 188-4-1-ts_adaptation_field_Head->adaptation_field_length-14; //计算分片包的第一个包的负载长度
	NeafPacketCount += 1;                                                                   //第一个分片包  

	if (ts_pes->Pes_Packet_Length_Beyond < FirstPacketLoadLength)                           //这里是 sps ，pps ，sei等//一个包的情况
	{
		memset(TSbuf,0xFF,TS_PACKET_SIZE);
		if (ts_pes->stream_id == TS_H264_STREAM_ID)
		{	
			WriteStruct_Packetheader(TSbuf,TS_H264_PID,0x01,0x03,pFifoTs->tsVideoCnt);                          //PID = TS_H264_PID,有效荷载单元起始指示符_play_init = 0x01, ada_field_C,0x03,含有调整字段和有效负载 ；
			pFifoTs->tsVideoCnt++;
		}
		else
		{	
			WriteStruct_Packetheader(TSbuf,TS_AAC_PID,0x01,0x03,pFifoTs->tsAudioCnt);                          //PID = TS_H264_PID,有效荷载单元起始指示符_play_init = 0x01, ada_field_C,0x03,含有调整字段和有效负载 ；
			pFifoTs->tsAudioCnt++;
		}
		ts_pos += 4;
		TSbuf[ts_pos + 0] = 184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 1 ;
		TSbuf[ts_pos + 1] = 0x00;
		ts_pos += 2; 
		memset(TSbuf + ts_pos,0xFF,(184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 2));
		ts_pos += (184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 2);

		TSbuf[ts_pos + 0] = (ts_pes->packet_start_code_prefix >> 16) & 0xFF;
		TSbuf[ts_pos + 1] = (ts_pes->packet_start_code_prefix >> 8) & 0xFF; 
		TSbuf[ts_pos + 2] = ts_pes->packet_start_code_prefix & 0xFF;
		TSbuf[ts_pos + 3] = ts_pes->stream_id;
		TSbuf[ts_pos + 4] = ((ts_pes->PES_packet_length) >> 8) & 0xFF;
		TSbuf[ts_pos + 5] = (ts_pes->PES_packet_length) & 0xFF;
		TSbuf[ts_pos + 6] = ts_pes->marker_bit << 6 | ts_pes->PES_scrambling_control << 4 | ts_pes->PES_priority << 3 |
			ts_pes->data_alignment_indicator << 2 | ts_pes->copyright << 1 |ts_pes->original_or_copy;
		TSbuf[ts_pos + 7] = ts_pes->PTS_DTS_flags << 6 |ts_pes->ESCR_flag << 5 | ts_pes->ES_rate_flag << 4 |
			ts_pes->DSM_trick_mode_flag << 3 | ts_pes->additional_copy_info_flag << 2 | ts_pes->PES_CRC_flag << 1 | ts_pes->PES_extension_flag;
		TSbuf[ts_pos + 8] = ts_pes->PES_header_data_length;
		ts_pos += 9;

		TSbuf[ts_pos + 0] = (((0x3 << 4) | ((i_pts>> 29) & 0x0E) | 0x01) & 0xff);
		TSbuf[ts_pos + 1]= (((((i_pts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
		TSbuf[ts_pos + 2]= ((((i_pts >> 14) & 0xfffe) | 0x01) & 0xff);
		TSbuf[ts_pos + 3]= (((((i_pts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
		TSbuf[ts_pos + 4]= ((((i_pts << 1) & 0xfffe) | 0x01) & 0xff);
		ts_pos += 5;
		memcpy(TSbuf + ts_pos,ts_pes->Es,ts_pes->Pes_Packet_Length_Beyond);
		
		tsWrite(pFifoTs,(char *)TSbuf,188);
		
		return 0;
	}
	
	NeafPacketCount += (ts_pes->Pes_Packet_Length_Beyond - FirstPacketLoadLength)/ 184;     
	NeafPacketCount += 1;                                                                   //最后一个分片包
	AdaptiveLength = 188 - 4 - 1 - ((ts_pes->Pes_Packet_Length_Beyond - FirstPacketLoadLength)% 184)  ;  //要填写0XFF的长度
	//开始处理第一个包,分片包的个数最少也会是两个 
	if (ts_pes->stream_id == TS_H264_STREAM_ID)
	{	
		WriteStruct_Packetheader(TSbuf,TS_H264_PID,0x01,0x03,pFifoTs->tsVideoCnt);
		pFifoTs->tsVideoCnt++;
	}
	else
	{	
		WriteStruct_Packetheader(TSbuf,TS_AAC_PID,0x01,0x03,pFifoTs->tsAudioCnt);                          //PID = TS_H264_PID,有效荷载单元起始指示符_play_init = 0x01, ada_field_C,0x03,含有调整字段和有效负载 ；
		pFifoTs->tsAudioCnt++;
	}
	ts_pos += 4;
	TSbuf[ts_pos] = ts_adaptation_field_Head->adaptation_field_length;                      //自适应字段的长度，自己填写的
	ts_pos += 1;                                                       

	CreateAdaptive_Ts(ts_adaptation_field_Head,TSbuf + ts_pos,(188 - 4 - 1 - 14));          //填写自适应字段
	ts_pos += ts_adaptation_field_Head->adaptation_field_length;                            //填写自适应段所需要的长度

	TSbuf[ts_pos + 0] = (ts_pes->packet_start_code_prefix >> 16) & 0xFF;
	TSbuf[ts_pos + 1] = (ts_pes->packet_start_code_prefix >> 8) & 0xFF; 
	TSbuf[ts_pos + 2] = ts_pes->packet_start_code_prefix & 0xFF;
	TSbuf[ts_pos + 3] = ts_pes->stream_id;
	TSbuf[ts_pos + 4] = ((ts_pes->PES_packet_length) >> 8) & 0xFF;
	TSbuf[ts_pos + 5] = (ts_pes->PES_packet_length) & 0xFF;
	TSbuf[ts_pos + 6] = ts_pes->marker_bit << 6 | ts_pes->PES_scrambling_control << 4 | ts_pes->PES_priority << 3 |
		ts_pes->data_alignment_indicator << 2 | ts_pes->copyright << 1 |ts_pes->original_or_copy;
	TSbuf[ts_pos + 7] = ts_pes->PTS_DTS_flags << 6 |ts_pes->ESCR_flag << 5 | ts_pes->ES_rate_flag << 4 |
		ts_pes->DSM_trick_mode_flag << 3 | ts_pes->additional_copy_info_flag << 2 | ts_pes->PES_CRC_flag << 1 | ts_pes->PES_extension_flag;
	TSbuf[ts_pos + 8] = ts_pes->PES_header_data_length;
	ts_pos += 9;

	TSbuf[ts_pos + 0] = (((0x3 << 4) | ((i_pts>> 29) & 0x0E) | 0x01) & 0xff);
	TSbuf[ts_pos + 1]= (((((i_pts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
	TSbuf[ts_pos + 2]= ((((i_pts >> 14) & 0xfffe) | 0x01) & 0xff);
	TSbuf[ts_pos + 3]= (((((i_pts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
	TSbuf[ts_pos + 4]= ((((i_pts << 1) & 0xfffe) | 0x01) & 0xff);
	ts_pos += 5;

	NeafBuf = ts_pes->Es ;
	memcpy(TSbuf + ts_pos,NeafBuf,FirstPacketLoadLength);  

	NeafBuf += FirstPacketLoadLength;
	ts_pes->Pes_Packet_Length_Beyond -= FirstPacketLoadLength;
	
	tsWrite(pFifoTs,(char *)TSbuf,188);

	while(ts_pes->Pes_Packet_Length_Beyond)
	{
		ts_pos = 0;
		memset(TSbuf,0,TS_PACKET_SIZE); 
		if(ts_pes->Pes_Packet_Length_Beyond >= 184)
		{	//处理中间包   
			if (ts_pes->stream_id == TS_H264_STREAM_ID)
			{	
				WriteStruct_Packetheader(TSbuf,TS_H264_PID,0x00,0x01,pFifoTs->tsVideoCnt);
				pFifoTs->tsVideoCnt++;
			}
			else
			{	
				WriteStruct_Packetheader(TSbuf,TS_AAC_PID,0x00,0x01,pFifoTs->tsAudioCnt);
				pFifoTs->tsAudioCnt++;
			}
			
			ts_pos += 4;
            memcpy(TSbuf + ts_pos,NeafBuf,184); 
			NeafBuf += 184;
			ts_pes->Pes_Packet_Length_Beyond -= 184;
			tsWrite(pFifoTs,(char *)TSbuf,188);
		}
		else
		{
			if(ts_pes->Pes_Packet_Length_Beyond == 183||ts_pes->Pes_Packet_Length_Beyond == 182)
			{
				if (ts_pes->stream_id == TS_H264_STREAM_ID)
				{	
					WriteStruct_Packetheader(TSbuf,TS_H264_PID,0x00,0x03,pFifoTs->tsVideoCnt); 
					pFifoTs->tsVideoCnt++;
				}
				else
				{	
					WriteStruct_Packetheader(TSbuf,TS_AAC_PID,0x00,0x03,pFifoTs->tsAudioCnt); 
					pFifoTs->tsAudioCnt++;
				}
				ts_pos += 4;
				TSbuf[ts_pos + 0] = 0x01;
				TSbuf[ts_pos + 1] = 0x00;
				ts_pos += 2;
				memcpy(TSbuf + ts_pos,NeafBuf,182); 
				  
				NeafBuf += 182;
				ts_pes->Pes_Packet_Length_Beyond -= 182;
				tsWrite(pFifoTs,(char *)TSbuf,188);
			}
			else
			{
				if (ts_pes->stream_id == TS_H264_STREAM_ID)
				{	
					WriteStruct_Packetheader(TSbuf,TS_H264_PID,0x00,0x03,pFifoTs->tsVideoCnt);
					pFifoTs->tsVideoCnt++;
				}
				else
				{	
					WriteStruct_Packetheader(TSbuf,TS_AAC_PID,0x00,0x03,pFifoTs->tsAudioCnt);
					pFifoTs->tsAudioCnt++;
				}
				ts_pos += 4;
				TSbuf[ts_pos + 0] = 184-ts_pes->Pes_Packet_Length_Beyond-1 ;
				TSbuf[ts_pos + 1] = 0x00;
				ts_pos += 2;
				memset(TSbuf + ts_pos,0xFF,(184 - ts_pes->Pes_Packet_Length_Beyond - 2)); 
				ts_pos += (184-ts_pes->Pes_Packet_Length_Beyond-2);
				memcpy(TSbuf + ts_pos,NeafBuf,ts_pes->Pes_Packet_Length_Beyond);
				ts_pes->Pes_Packet_Length_Beyond = 0;
				tsWrite(pFifoTs,(char *)TSbuf,188);
			}
		}	
	}

	return 0;
}

/******************************************************************************/
//264 打包成TS流的分包
/******************************************************************************/
void H264ToTs(st_fifoStream_t * pFifoTs,char * pBuf,int iFrameLen,unsigned long pts_s,int pts_ms)
{
	if ((pFifoTs ->tsFrameCnt%5) == 0)
	{
		CreatePAT(pFifoTs,pFifoTs->tsPtmCnt);
		CreatePMT(pFifoTs,pFifoTs->tsPtmCnt);
		pFifoTs->tsPtmCnt++;
	}
	pFifoTs ->tsFrameCnt++;
	
	TsPes * pst_pes;
	pst_pes = (TsPes *)malloc(sizeof(TsPes));
	if (pst_pes != NULL)
	{
		pst_pes->Es = malloc(iFrameLen+1024);
		if (pst_pes->Es != NULL)
		{
			unsigned int pts = 0;
			memset(pst_pes->Es,0,iFrameLen+1024);
			
			pst_pes->packet_start_code_prefix = 0x000001;
			pst_pes->stream_id = (char)TS_H264_STREAM_ID;							//E0~EF表示是视频的,C0~DF是音频,H264-- E0
			if ((unsigned int)iFrameLen > 65535)
				pst_pes->PES_packet_length = 0;
			else 
				pst_pes->PES_packet_length = iFrameLen;								//一帧数据的长度 不包含 PES包头
			pst_pes->Pes_Packet_Length_Beyond = iFrameLen;
			pst_pes->marker_bit = 0x02;
			pst_pes->PES_scrambling_control = 0x00;								//人选字段 存在，不加扰
			pst_pes->PES_priority = 0x00;
			pst_pes->data_alignment_indicator = 0x00;
			pst_pes->copyright = 0x00;
			pst_pes->original_or_copy = 0x00;
			pst_pes->PTS_DTS_flags = 0x02;
			pst_pes->ESCR_flag = 0x00;
			pst_pes->ES_rate_flag = 0x00;
			pst_pes->DSM_trick_mode_flag = 0x00;
			pst_pes->additional_copy_info_flag = 0x00;
			pst_pes->PES_CRC_flag = 0x00;
			pst_pes->PES_extension_flag = 0x00;
			pst_pes->PES_header_data_length = 0x05;                                //后面的数据 包括了	PTS和 DTS所占的字节数
		
			pts = Timestamp_video;
			pst_pes->tsptsdts.pts_32_30 = 0;
			pst_pes->tsptsdts.pts_29_15 = 0;
			pst_pes->tsptsdts.pts_14_0 = 0;
			pst_pes->tsptsdts.reserved_1 = 0x03;
			if(pts > 0x7FFFFFFF)
			{
				pst_pes->tsptsdts.pts_32_30 = (pts >> 30) & 0x07;                 
				pst_pes->tsptsdts.marker_bit1 = 0x01;
			}
			else 
				pst_pes->tsptsdts.marker_bit1 = 0;
			
			if(pts > 0x7FFF)
			{
				pst_pes->tsptsdts.pts_29_15 = (pts >> 15) & 0x007FFF ;
				pst_pes->tsptsdts.marker_bit2 = 0x01;
			}
			else
				pst_pes->tsptsdts.marker_bit2 = 0;
			pst_pes->tsptsdts.pts_14_0 = pts & 0x007FFF;
			pst_pes->tsptsdts.marker_bit3 = 0x01;

			memcpy(pst_pes->Es,pBuf,iFrameLen);
			if (((pBuf[4]&0x1f)==1) || ((pBuf[4]&0x1f)==5))
			{
				WriteAdaptive_flags_Head(&ts_adaptation_field_Head,Timestamp_video); //填写自适应段标志帧头
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //填写自适应段标志帧尾
			}
			else
			{
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //填写自适应段标志  ,这里注意 其它帧类型不要算pcr 所以都用帧尾代替就行
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //填写自适应段标志帧尾
			}
			PES2TS(pFifoTs,pst_pes,&ts_adaptation_field_Head,&ts_adaptation_field_Tail,Timestamp_video); 
			if (((pBuf[4]&0x1f)==1) || ((pBuf[4]&0x1f)==5))
				Timestamp_video += 3600;
					
			free(pst_pes->Es);
		}
		free(pst_pes);
	}
}

/******************************************************************************/
//264 打包成TS流的分包
/******************************************************************************/
void AACToTs(st_fifoStream_t * pFifoTs,char * pBuf,int iFrameLen,unsigned long pts_s,int pts_ms)
{
	TsPes * pst_pes;
	pst_pes = (TsPes *)malloc(sizeof(TsPes));
	if (pst_pes != NULL)
	{
		pst_pes->Es = malloc(iFrameLen+1024);
		if (pst_pes->Es != NULL)
		{
			unsigned int pts = 0;
			memset(pst_pes->Es,0,iFrameLen+1024);
			
			pst_pes->packet_start_code_prefix = 0x000001;
			pst_pes->stream_id = (char)TS_AAC_STREAM_ID;							//E0~EF表示是视频的,C0~DF是音频,H264-- E0
			if ((unsigned int)iFrameLen > 65535)
				pst_pes->PES_packet_length = 0;
			else 
				pst_pes->PES_packet_length = iFrameLen;								//一帧数据的长度 不包含 PES包头
			pst_pes->Pes_Packet_Length_Beyond = iFrameLen;
			pst_pes->marker_bit = 0x02;
			pst_pes->PES_scrambling_control = 0x00;								//人选字段 存在，不加扰
			pst_pes->PES_priority = 0x00;
			pst_pes->data_alignment_indicator = 0x00;
			pst_pes->copyright = 0x00;
			pst_pes->original_or_copy = 0x00;
			pst_pes->PTS_DTS_flags = 0x02;
			pst_pes->ESCR_flag = 0x00;
			pst_pes->ES_rate_flag = 0x00;
			pst_pes->DSM_trick_mode_flag = 0x00;
			pst_pes->additional_copy_info_flag = 0x00;
			pst_pes->PES_CRC_flag = 0x00;
			pst_pes->PES_extension_flag = 0x00;
			pst_pes->PES_header_data_length = 0x05;                                //后面的数据 包括了	PTS和 DTS所占的字节数
		
			pts = Timestamp_video;//Timestamp_audio;
			
			pst_pes->tsptsdts.pts_32_30 = 0;
			pst_pes->tsptsdts.pts_29_15 = 0;
			pst_pes->tsptsdts.pts_14_0 = 0;
			pst_pes->tsptsdts.reserved_1 = 0x03;
			if(pts > 0x7FFFFFFF)
			{
				pst_pes->tsptsdts.pts_32_30 = (pts >> 30) & 0x07;                 
				pst_pes->tsptsdts.marker_bit1 = 0x01;
			}
			else 
				pst_pes->tsptsdts.marker_bit1 = 0;
			
			if(pts > 0x7FFF)
			{
				pst_pes->tsptsdts.pts_29_15 = (pts >> 15) & 0x007FFF ;
				pst_pes->tsptsdts.marker_bit2 = 0x01;
			}
			else
				pst_pes->tsptsdts.marker_bit2 = 0;
			pst_pes->tsptsdts.pts_14_0 = pts & 0x007FFF;
			pst_pes->tsptsdts.marker_bit3 = 0x01;
			
			memcpy(pst_pes->Es,pBuf,iFrameLen);
			WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //填写自适应段标志  ,这里注意 其它帧类型不要算pcr 所以都用帧尾代替就行
			WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //填写自适应段标志帧尾
			PES2TS(pFifoTs,pst_pes,&ts_adaptation_field_Head,&ts_adaptation_field_Tail,Timestamp_video); 
			Timestamp_audio += 1920;
			free(pst_pes->Es);
		}
		free(pst_pes);
	}
}

/******************************************************************************/
//提供给外面的读取TS流包的接口
/******************************************************************************/
void *convert_ts_stream(void * cvt)
{
	int iHandle;
	int i=(int)cvt;
	
	st_fifo_t stFifo;	
	stFifo.iGrp   = g_stFifoStream[i].iGrp;
	stFifo.iChn   = g_stFifoStream[i].iChn;
	stFifo.enType = FIFO_TYPE_MEDIA;//g_stFifoStream[i].enType;
	int iFifo = FIFO_Request(stFifo,&iHandle,0);
	
	int iGlen;
	int iPoint = 0;
	unsigned short usframeCur=0;
	unsigned short usframeCmp=0;
	char cBuf[1600];
	char * cNalu=malloc(0x80000);

    int isIDR  = 0;
    int hasIDR = 0;
    int iPts_s = 0;
    int iPts_ms= 0;    
    unsigned int   iNalu_cnt=0;
	unsigned int   iNalu_len[8];
	while(g_stFifoStream[i].iConvertThread == 1)
	{
		iGlen = FIFO_Read(iHandle,iFifo,cBuf);
		if (iGlen > 11)
		{
			if (cBuf[1] == 0x38)
			{
				;
			}
			else if (cBuf[1] == 0x39)
			{
				if (hasIDR == 1)
				{
					memcpy(&usframeCur,cNalu+11,2);
					if (usframeCur == usframeCmp)
					{
						if ((cNalu[20]&0x80) != 0)
						{
							memcpy(cNalu+iPoint,&cNalu[27],iGlen-27);
							iNalu_len[iNalu_cnt] += iGlen-27;
							iPoint += iGlen-27;
						}
						else
						{
							memcpy(cNalu+iPoint,&cNalu[23],iGlen-23);
							iPoint += iGlen-23;
							iNalu_cnt++;
							iNalu_len[iNalu_cnt] = iGlen-23;
						}
					}
					else
					{
						if ((cNalu[4]&0x1f) == 7)
							isIDR = 1;
						else
							isIDR = 0;
						
						int j;
						for (j=0; j<iNalu_cnt; j++)
						{
							if (iNalu_len[j] > 0)
								H264ToTs(&g_stFifoStream[i],cNalu+iNalu_len[j],iNalu_len[j],iPts_s,iPts_ms);							
						}
						
						iNalu_cnt=0;
						memcpy(cNalu,&cNalu[23],iGlen-23);
						iPoint  = iNalu_len[0] = iGlen-23;
						memcpy(&iPts_s,&cNalu[15],4);
	            		iPts_ms = (int)cNalu[19];
					}
				}
				else
				{
					if ((cNalu[20]&0x80) == 0)
					{
						if ((cNalu[27]&0x1f) == 7)
						{
							hasIDR  = 1;
							iNalu_cnt=0;
							memcpy(cNalu,&cNalu[23],iGlen-23);
							iPoint  = iNalu_len[0] = iGlen-23;
							memcpy(&iPts_s,&cNalu[15],4);
		            		iPts_ms = (int)cNalu[19];
						}
					}
					memcpy(&usframeCmp,cNalu+11,2);
					usframeCur = usframeCmp;
				}
			}
			else if (cBuf[1] == 0x40)
			{
				if (hasIDR == 1)
					AACToTs(&g_stFifoStream[i],&cBuf[23],iGlen-23,iPts_s,iPts_ms);
			}			
		}
		else
			usleep(10000);
	}
	FIFO_Release(iHandle,iFifo);
	if (cNalu != NULL)
		free(cNalu);
	return cvt;
}
