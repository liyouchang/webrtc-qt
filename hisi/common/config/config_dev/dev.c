#include "dev.h"
#include <unistd.h>

struct FIELDINFO
{
	char szFieldName[64];
	int  iArrayIndex;
	char szFieldValue[128];
};
struct DEVHARDPARAM devHardware;
/***************************************************************************
 *	函数名称：	AnalyzeFieldInfo
 *	功能描述：	分析文本行中包含的结构体字段信息
 *	日    期：	2009-01-17 16:48:28
 *	作    者：	廉秀柱
 *	参数说明：	szLineText - 文本行内容
 *				fieldinfo  - 接收返回的字段信息的结构体
 *	返 回 值：	 0	- 成功分析得到一条字段信息
 *				-1	- 分析失败，可能是格式不正确
 ***************************************************************************/
int AnalyzeFieldInfo(const char* szLineText, struct FIELDINFO *pFieldInfo)
{
	int i = 0, j = 0;
	unsigned char* ptrChar = (unsigned char*)szLineText;
	while(ptrChar[i] == 0x09 || ptrChar[i] == 0x0A || ptrChar[i] == 0x0D || ptrChar[i] == 0x20 || ptrChar[i] == 0xA1) i++;
	if(ptrChar[i] == '\0' || ptrChar[i] == '#')
        return -1;
	else
	{

		memset(pFieldInfo, 0, sizeof(struct FIELDINFO));//初始化结构体内容为空
		char szArrayIndex[8] = "";						//开始分析当前配置字符串
		char* ptrEqual = strchr(&szLineText[i], '=');
		char* ptrLQuotation = strchr(&szLineText[i], '\"');
		char* ptrRQuotation = NULL;
		if(ptrLQuotation != NULL)
		{
			ptrRQuotation = strchr(ptrLQuotation + 1, '\"');
		}
		if(ptrEqual != NULL && ptrLQuotation != NULL && ptrLQuotation != NULL && ptrLQuotation > ptrEqual)
		{
			char* ptrLSquare = strchr(&szLineText[i], '[');
			char* ptrRSquare = strchr(&szLineText[i], ']');
			if(ptrLSquare != NULL && ptrRSquare != NULL)
			{
				if(ptrLSquare < ptrRSquare && ptrRSquare < ptrEqual)
				{
					j = 0;
					for(ptrChar = (unsigned char*)szLineText; ptrChar < (unsigned char*)ptrLSquare; ptrChar++)
					{
						if(ptrChar[0] != 0x09 && ptrChar[0] != 0x0A && ptrChar[0] != 0x0D && ptrChar[0] != 0x20 && ptrChar[0] != 0xA1)
						{
							pFieldInfo->szFieldName[j++] = ptrChar[0];
						}
					}
					memcpy(szArrayIndex, ptrLSquare + 1, ptrRSquare - ptrLSquare - 1);
					pFieldInfo->iArrayIndex = atoi(szArrayIndex);
				}
			}
			else if(ptrLSquare == NULL && ptrRSquare == NULL)
			{
				j = 0;
				for(ptrChar = (unsigned char*)szLineText; ptrChar < (unsigned char*)ptrEqual; ptrChar++)
				{
					if(ptrChar[0] != 0x09 && ptrChar[0] != 0x0A && ptrChar[0] != 0x0D && ptrChar[0] != 0x20 && ptrChar[0] != 0xA1)
					{
						pFieldInfo->szFieldName[j++] = ptrChar[0];
					}
				}
				pFieldInfo->iArrayIndex = -1;
			}
			else
			{
				return -1;
			}
			memcpy(pFieldInfo->szFieldValue, ptrLQuotation + 1, ptrRQuotation - ptrLQuotation - 1);
			return 0;
		}
		return -1;
	}
}

int devFileRead(struct DEVHARDPARAM ** dev)
{
	FILE *fpDevTxt = fopen("/dvs/devHardware.txt", "r");
	if (fpDevTxt==NULL)
	{
		perror("open devHardware.txt r failed\n");
		return -1;
	}

	int i;
	memset(&devHardware,0,sizeof(struct DEVHARDPARAM));
	for (i=0; i<MAX_ALARMINPUTS; i++)
		devHardware.InputAlarmPort[i]=12;
	for (i=0; i<MAX_ALARMOUTPUTS; i++)
		devHardware.OutputAarmPort[i]=12;
	for (i=0; i<4; i++)
	{
		devHardware.ModulePowerPort[i]=12;
		devHardware.ModuleResetPort[i]=12;
		devHardware.ModuleClosePort[i]=12;
	}
	memset(devHardware.doubleModule,0xff,4);
	devHardware.vencNumber    = -1;
	devHardware.talkCtrl      = 100;
	devHardware.ResetKeyBit   = 8;
	devHardware.ResetKeyPort  = 12;
	devHardware.usbControlPort= 12;
	devHardware.usbControlBit = 8;
	devHardware.GpsFromModem  = 4;
	devHardware.IrCardPort    = 12;
	devHardware.IrCardBit     = 12;
	devHardware.LightSensorPort= 12;
	devHardware.LightSensorBit= 12;
	devHardware.P2P_Enable    = 0;
	strcpy(devHardware.wifiEth,"eth1");

	char   szLineText[512];
    struct FIELDINFO fieldinfo;
	while(!feof(fpDevTxt))
	{
		fgets(szLineText,sizeof(szLineText),fpDevTxt);
		if(AnalyzeFieldInfo(szLineText,&fieldinfo) == 0)
		{
			if(strcmp(fieldinfo.szFieldName, "AudioAiAdType") == 0)
			{
				devHardware.AudioAiAdType = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "AudioAoAdType") == 0)
			{
				devHardware.AudioAoAdType = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "AudioInputMode") == 0)
			{
				devHardware.AudioInputMode = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "DeviceType") == 0)
			{
				strcpy(devHardware.DeviceType, fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "EncoderVersion") == 0)
			{
				devHardware.encodever = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "HardwareVersion") == 0)
			{
				devHardware.hardver = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "InputAlarmBit") == 0)
			{
				devHardware.InputAlarmBit[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "InputAlarmNum") == 0)
			{
				devHardware.InputAlarmNum = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "InputAlarmPort") == 0)
			{
				devHardware.InputAlarmPort[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "LampBit") == 0)
			{
				devHardware.LampBit = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "LampPort") == 0)
			{
				devHardware.LampPort = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "OutputAlarmBit") == 0)
			{
				devHardware.OutputAlarmBit[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "OutputAlarmNum") == 0)
			{
				devHardware.OutputAlarmNum = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "OutputAlarmPort") == 0)
			{
				devHardware.OutputAarmPort[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "IrCardPort") == 0)
			{
				devHardware.IrCardPort = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "IrCardBit") == 0)
			{
				devHardware.IrCardBit = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "LightSensorPort") == 0)
			{
				devHardware.LightSensorPort = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "LightSensorBit") == 0)
			{
				devHardware.LightSensorBit = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "RecordDeviceType") == 0)
			{
				devHardware.RecordDeviceType = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "SATA_Enable") == 0)
			{
				devHardware.SATA_enable = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "SD_Enable") == 0)
			{
				devHardware.SD_enable = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "SoftwareVersion") == 0)
			{
				devHardware.softver = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "USB_Enable") == 0)
			{
				devHardware.USB_enable = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "VideoAdType") == 0)
			{
				devHardware.VideoAdType = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "VideoChanNum") == 0)
			{
				devHardware.VideoChanNum = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "NetWork") == 0)
			{
				devHardware.NetWork = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "ModulePowerPort") == 0)
			{
				devHardware.ModulePowerPort[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "ModulePowerbit") == 0)
			{
				devHardware.ModulePowerbit[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "ModuleResetPort") == 0)
			{
				devHardware.ModuleResetPort[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "ModuleResetbit") == 0)
			{
				devHardware.ModuleResetbit[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "ModuleClosePort") == 0)
			{
				devHardware.ModuleClosePort[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "ModuleCLosebit") == 0)
			{
				devHardware.ModuleCLosebit[fieldinfo.iArrayIndex] = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "ResetKeyPort") == 0)
			{
				devHardware.ResetKeyPort = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "ResetKeyBit") == 0)
			{
				devHardware.ResetKeyBit = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "usbControlPort") == 0)
			{
				devHardware.usbControlPort = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "usbControlBit") == 0)
			{
				devHardware.usbControlBit  = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "wifiEth") == 0)
			{
				strcpy(devHardware.wifiEth,fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "voltageDetect") == 0)
			{
				devHardware.voltageDetect = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "vencNumber") == 0)
			{
				devHardware.vencNumber = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "talkCtrl") == 0)
			{
				devHardware.talkCtrl = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "infoDevice_1") == 0)
			{
				strcpy(devHardware.infoDevice_1,fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "infoDevice_2") == 0)
			{
				strcpy(devHardware.infoDevice_2,fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "uartFlag") == 0)
			{
				devHardware.uartFlag = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "wirelessAlarm") == 0)
			{
				devHardware.wirelessAlarm = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "doubleModule") == 0)
			{
				memcpy(devHardware.doubleModule,fieldinfo.szFieldValue,4);
			}
			else if(strcmp(fieldinfo.szFieldName, "rtpMode") == 0)
			{
				devHardware.rtpMode = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "singleton") == 0)
			{
				devHardware.singleton = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "GpsFromModem") == 0)
			{
				devHardware.GpsFromModem = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "OEM_Type") == 0)
			{
				strcpy(devHardware.OEM_Type,fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "tvp5150Reset") == 0)
			{
				devHardware.tvp5150Reset = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "NetcardType") == 0)
			{
				devHardware.NetcardType = atoi(fieldinfo.szFieldValue);
			}
			else if(strcmp(fieldinfo.szFieldName, "P2P_Enable") == 0)
			{
				devHardware.P2P_Enable = atoi(fieldinfo.szFieldValue);
			}
		}
	}
	if (system("iwconfig > /tmp/phy_wifi") < 0)
		system("iwconfig > /tmp/phy_wifi");
	usleep(300000);
	char cBuf[2048];
	FILE *phyFd = fopen("/tmp/phy_wifi","r");
	if (phyFd != NULL)
	{
		if (fread(cBuf,1,2048,phyFd) > 0)
		{
			cBuf[2047] = 0;
			if (strstr(cBuf,"ra0") != NULL)
				strcpy(devHardware.wifiEth,"ra0");
			else if (strstr(cBuf,"wlan0") != NULL)
				strcpy(devHardware.wifiEth,"wlan0");
		}
		fclose(phyFd);
	}
	fclose(fpDevTxt);
	if (devHardware.P2P_Enable == 1)
		devHardware.softver = 0x0547;
	else
		devHardware.softver = 0x0447;
#ifndef	SAFE_MODE
	if ((devHardware.softver&0x000f)==0x0009)
		devHardware.softver = (devHardware.softver&0xfff0)+0x10;
	else
    	devHardware.softver += 1;
#endif
	*dev = (struct DEVHARDPARAM *)&devHardware;
	
	return 0;
}

int devFileGet(struct DEVHARDPARAM *dev)
{
	memcpy(dev,&devHardware,sizeof(struct DEVHARDPARAM));
	return 0;
}
