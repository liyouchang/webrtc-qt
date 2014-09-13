#include "net.h"

struct NETPARAM	netParam;
CONFIG_CALLBACK netCallBack=0;

static void setDefaultNet(struct NETPARAM *this)
{
	memset(this,0,sizeof(struct NETPARAM));

	strcpy((char *)this->macAddress, "0090b0123458");
	this->localIP[0]   = 192;
	this->localIP[1]   = 168;
	this->localIP[2]   = 0;
	this->localIP[3]   = 230;
	this->gateIP[0]    = 192;
	this->gateIP[1]    = 168;
	this->gateIP[2]    = 0;
	this->gateIP[3]    = 1;
	this->netMask[0]   = 255;
	this->netMask[1]   = 255;
	this->netMask[2]   = 255;
	this->netMask[3]   = 0;
	this->dnsIP[0]     = 8;
	this->dnsIP[1]     = 8;
	this->dnsIP[2]     = 8;
	this->dnsIP[3]     = 8;
	this->webMediaPort = 22616;
	this->dnsPort      = 80;		//？？？？
	this->webServicePort=80;		//初始化修改
	this->dhcpEnable   = 1;
	this->wifiDhcp     = 0;
	this->routePort    = 0xffffffff;
	memcpy(this->username,"system  ",8);
	memcpy(this->password,"123456  ",8);
	this->card_ctrl[0].cardEnable = 1;
	this->card_ctrl[1].cardEnable = 1;
}

int netFileRead(struct DEVHARDPARAM * dev)
{
	FILE *fp = NULL;

	if((fp=fopen("/dvs/Net.bin","r")) == NULL)
	{
		perror("打开网络参数配置文件 读 失败\n");
		setDefaultNet(&netParam);
	}
	else
	{
		fread(&netParam,1,sizeof(struct NETPARAM),fp);
		fclose(fp);
	}
	if (netParam.dhcpEnable == 1)
        netParam.routePort = 0xffffffff;
    int iIP;
	memcpy(&iIP,netParam.dnsIP,4);
	if ((iIP==-1) || (iIP==0))
	{
		netParam.dnsIP[0] = 8;
		netParam.dnsIP[1] = 8;
		netParam.dnsIP[2] = 8;
		netParam.dnsIP[3] = 8;
	}
	return 0;
}

int netFileGet(struct NETPARAM *net)
{
	memcpy(net,&netParam,sizeof(struct NETPARAM));
	return 0;
}

int netFileSet(struct NETPARAM *net)
{
	if (memcmp(net,&netParam,sizeof(struct NETPARAM)))
	{
		memcpy(&netParam,net,sizeof(struct NETPARAM));
		
		FILE *fp = NULL;
		if ((fp=fopen("/dvs/Net.bin","w+")) == NULL)
			perror("打开网络参数配置文件 写 失败\n");
		else
		{
		    printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
		    fwrite((char *)&netParam,1,sizeof(struct NETPARAM),fp);
			fclose(fp);
		}
		
		if (netCallBack != 0)
			netCallBack((void *)&netParam);
	}
	return 0;
}

