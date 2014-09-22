#include "manage.h"
#include "../../config/config_dev/dev.h"
#include "../../config/config_net/net.h"

int g_iHostAPEnable = 0;
int g_iWifiLinkStatus=0;

static struct NETPARAM stNetParam;
static struct DEVHARDPARAM stDevFile;
/**********************************************************/
//名称:					MySystemCmd
//功能:网络诊断
//参数:	input	命令行字符串
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
pthread_mutex_t mutex_command;
int MySystemCmd(char *command)
{
	int iRet;
	pthread_mutex_lock(&mutex_command);
	iRet = system(command);
	pthread_mutex_unlock(&mutex_command);
	if (iRet < 0)
    {
    	printf("执行%s失败\n",command);
    	perror("\n");
    }
    else
        printf("执行%s完毕\n",command);
	return iRet;
}
/**********************************************************/
//名称:					getNetStatus
//功能:获取网络状态
//参数:	input	网络落状态参数
//		output	0-成功；-1-失败
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
st_net_status_t stNetStatus;
int NET_Get_Status(st_net_status_t * status)
{
    memcpy(status,&stNetStatus,sizeof(st_net_status_t));
    return 0;
}
/**********************************************************/
//名称:					getNetStatus
//功能:获取网络状态
//参数:	input	网络落状态参数
//		output	0-成功；-1-失败
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
int NET_Get_WanStatus(void)
{
	return isWanLinked;
}
/**********************************************************/
//名称:					net_set_default
//功能:恢复网络设置
//参数:	input	网络参数
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
int net_get_RouteIP(void)
{
    FILE * fp=fopen("/proc/net/route", "r");
    if (fp == NULL)
        return 0;
    int iRet;
    char buf[256];                      // 128 is enough for linux
    char iFace[32];
    unsigned long destAddr,gateAddr;
    int ulFlags;

    fgets(buf,sizeof(buf),fp);          //先读一行标题   Skip title line
    while (fgets(buf,sizeof(buf),fp))   //开始读每一行路由
    {
        gateAddr = 0;
        iRet = sscanf(buf,"%s\t%lX\t%lX\t%d",iFace,&destAddr,&gateAddr,&ulFlags);
        if ((iRet==4) && (gateAddr!=0) && (ulFlags==3))
            break;
        if ((iRet==4) && (!strcmp(iFace,"ppp0")) && (ulFlags==5))//PPPOE拨号时 gateAddr=0 ulFlags=5
        {    
        	gateAddr = 1;
        	break;
        }
    }
    fclose(fp);

    return gateAddr;
}

/**********************************************************/
//名称:					net_set_default
//功能:恢复网络设置
//参数:	input	网络参数
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
int g_iAdsl_Running= 0;
int g_iDhcp_Running= 0;
int g_iWifi_Running= 0;
void net_set_default(void)
{
	g_iDhcp_Running = 0;
	if (MySystemCmd("killall udhcpc") == -1)
		MySystemCmd("killall udhcpc");
	g_iAdsl_Running = 0;
	if (MySystemCmd("pppoe-stop") == -1)
		MySystemCmd("pppoe-stop");
	sleep(1);

    remove("/tmp/set_gw");
	if (MySystemCmd("route del default eth0") == -1)
		MySystemCmd("route del default eth0");
	char file[128]={0};
	sprintf(file,"route del default %s",stDevFile.wifiEth);
    if (MySystemCmd(file) == -1)
        MySystemCmd(file);
    if (MySystemCmd("route del default ppp0") == -1)
        MySystemCmd("route del default ppp0");
}
/**********************************************************/
//名称:					start_eth0
//功能:启动eth0有线网卡的ip
//参数:	input	网络参数
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
int g_iAdslTimeout = 0;
int g_iAdslEnable  = 0;
void start_adsl(void)
{
	net_set_default();
        
    char cCmd[256];
    FILE * pFd = NULL;
    pFd = fopen("/tmp/PPPOE","w+");
    if (pFd != NULL)
    {
        if (stNetParam.wifiEnable==1)
            sprintf(cCmd,"%s\n%s\n\nserver\n%s\n%s\n0\ny\n",stNetParam.pppoeUser,stDevFile.wifiEth,stNetParam.pppoePass,stNetParam.pppoePass);
        else
            sprintf(cCmd,"%s\n%s\n\nserver\n%s\n%s\n0\ny\n",stNetParam.pppoeUser,"eth0",stNetParam.pppoePass,stNetParam.pppoePass);
        fwrite(cCmd,strlen(cCmd),1,pFd);
        fclose(pFd);
        
        remove("/tmp/set_gw");
        if (MySystemCmd("route del default ppp0") == -1)
            MySystemCmd("route del default ppp0");        
        MySystemCmd("route del default");
        MySystemCmd("rm /etc/ppp/*-secrets*");
        MySystemCmd("pppoe-setup < /tmp/PPPOE");
        MySystemCmd("pppoe-start");
        g_iAdslTimeout = 60;
        g_iAdslEnable  = 1;
    }
}
/**********************************************************/
//名称:					diagnose_adsl
//功能:诊断ppp0 获取ADSL拨号
//参数:	input	网络参数
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
void diagnose_adsl(void)
{
	int sock;
	struct ifreq ifr;
	struct sockaddr_in addr;

    sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0)
	{
		perror("diagnose_adsl:创建sock失败");
		return;
	}
    strcpy(ifr.ifr_name,"ppp0");
    if (ioctl(sock,SIOCGIFFLAGS,(char *)&ifr) >= 0)
    {
        if (ifr.ifr_flags & IFF_POINTOPOINT)
            stNetStatus.adsl.sDevice = PHY_STATUS_LINK100M;
        else
            stNetStatus.adsl.sDevice = PHY_STATUS_NOCARD;
    }
    else
        stNetStatus.adsl.sDevice = PHY_STATUS_IOCTRL_ERR;
    if (stNetStatus.adsl.sDevice >= PHY_STATUS_LINK10M)
    {
        if (ioctl(sock,SIOCGIFADDR,(char *)&ifr) < 0)
            stNetStatus.adsl.iIP = -1;
        else
        {
            struct in_addr in=((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr;
        	inet_aton(inet_ntoa(in),&addr.sin_addr);
            if (stNetStatus.adsl.iIP==-1)
            {
                printf("adsl ip is %d.%d.%d.%d\n",*((char *)(&addr.sin_addr.s_addr)),*(((char *)(&addr.sin_addr.s_addr))+1),*(((char *)(&addr.sin_addr.s_addr))+2),*(((char *)(&addr.sin_addr.s_addr))+3));
                char cCmd[128];
                if ((stNetParam.dhcpEnable==1) || (stNetParam.wifiDhcp==1))
                {
                    FILE * tmp=fopen("/tmp/set_gw","w+");
                    if (tmp != NULL)
                        fclose(tmp);
                }
                sprintf(cCmd,"route del default eth0");
                if (MySystemCmd(cCmd) == -1)
                    MySystemCmd(cCmd);
                if (stNetParam.wifiEnable==1)
                {
                    sprintf(cCmd,"route del default %s",stDevFile.wifiEth);
                    if (MySystemCmd(cCmd) == -1)
                        MySystemCmd(cCmd);
                }
                if (MySystemCmd("route add default ppp0") == -1)
                    MySystemCmd("route add default ppp0");
            }
            stNetStatus.adsl.iIP = addr.sin_addr.s_addr;
        }
    }
    else
        stNetStatus.adsl.iIP = -1;

    close(sock);
}

/**********************************************************/
//名称:					start_eth0
//功能:启动eth0功能
//参数:	input	网络参数
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
void check_eth0_link(void)
{
    int sock;
	struct ifreq ifr;

    sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0)
	{
		perror("check_eth0_link:创建sock失败");
		return;
	}
    strcpy(ifr.ifr_name,"eth0");
    if (ioctl(sock,SIOCGIFFLAGS,(char *)&ifr) >= 0)
    {
    	if (ifr.ifr_flags & IFF_UP)
        {
            struct ethtool_value edata;
            edata.cmd    = ETHTOOL_GLINK;
            ifr.ifr_data = (char *)&edata;
            if (!(ioctl(sock, SIOCETHTOOL, &ifr)<0))
            {
                if (edata.data == 0)
                {
                	printf("注意: 网线已经被拔出\n");
                	stNetStatus.lan.sDevice = PHY_STATUS_UNLINK;
                }
                else if (edata.data > 0)
                {
                	stNetStatus.lan.sDevice = PHY_STATUS_LINK100M;
                }
                else
                {
                    stNetStatus.lan.sDevice = PHY_STATUS_IOCTRL_ILLEGAL;
                    printf("ioctl 网卡状态非法值 %d\n",edata.data);
                }
            }
            else
            {
                stNetStatus.lan.sDevice = PHY_STATUS_IOCTRL_ERR;
                perror("SIOCETHTOOL ioctl 执行失败\n");
            }
        }
        else
        {
        	stNetStatus.lan.sDevice = PHY_STATUS_NOCARD;
        	printf("ifr.ifr_flags %08x IFF_UP %08x\n",ifr.ifr_flags,IFF_UP);
        }
    }
    else
    {
    	stNetStatus.lan.sDevice = PHY_STATUS_IOCTRL_ERR;
		perror("SIOCGIFFLAGS ioctl 执行失败\n");
    }

    close(sock);
}
void start_eth0(void)
{
	net_set_default();

    remove("/tmp/set_gw");
    MySystemCmd("route del default eth0");
    char cCmd[256];
	sprintf(cCmd,"route del default %s",stDevFile.wifiEth);
    MySystemCmd(cCmd);    
    MySystemCmd("route del default ppp0");
    
	sprintf(cCmd,"ifconfig eth0 hw ether %s",stNetParam.macAddress);
    MySystemCmd(cCmd);
	sprintf(cCmd,"ifconfig eth0 %d.%d.%d.%d netmask %d.%d.%d.%d broadcast %d.%d.%d.255",
            stNetParam.localIP[0],stNetParam.localIP[1],stNetParam.localIP[2],stNetParam.localIP[3],
            stNetParam.netMask[0],stNetParam.netMask[1],stNetParam.netMask[2],stNetParam.netMask[3],
            stNetParam.localIP[0],stNetParam.localIP[1],stNetParam.localIP[2]);
    MySystemCmd(cCmd);
	sprintf(cCmd,"route add default gw %d.%d.%d.%d eth0",stNetParam.gateIP[0],stNetParam.gateIP[1],stNetParam.gateIP[2],stNetParam.gateIP[3]);
    MySystemCmd(cCmd);
	MySystemCmd("route add -host 255.255.255.255 eth0");
	MySystemCmd("ifconfig eth0:1 192.168.254.230");
}
void diagnose_eth0(void)
{
	int sock;
	struct ifreq ifr;
	struct sockaddr_in addr;

    sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0)
	{
		perror("diagnose_eth0:创建sock失败");
		return;
	}
    strcpy(stNetStatus.lan.devName,"eth0");
	strcpy(ifr.ifr_name,"eth0");
    if (ioctl(sock,SIOCGIFADDR,(char *)&ifr) < 0)
		stNetStatus.lan.iIP = -1;
	else
	{
		struct in_addr in=((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr;
		inet_aton(inet_ntoa(in),&addr.sin_addr);
		if ((stNetStatus.lan.iIP==-1) || (stNetStatus.lan.iIP!=addr.sin_addr.s_addr))
		    printf("lan ip is %d.%d.%d.%d\n",*((char *)(&addr.sin_addr.s_addr)),*(((char *)(&addr.sin_addr.s_addr))+1),*(((char *)(&addr.sin_addr.s_addr))+2),*(((char *)(&addr.sin_addr.s_addr))+3));
		stNetStatus.lan.iIP = addr.sin_addr.s_addr;
	}

    close(sock);
}

/**********************************************************/
//名称:					start_dhcp
//功能:启动DHCP功能，可以通过有线网卡和wifi网卡获取ip
//参数:	input	网络参数
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
int g_iDhcpTimeout=0;
int g_iDhcpEnable =0;
int g_iDhcpResume =0;
void start_dhcp(int iFlag)
{
	net_set_default();

    remove("/tmp/set_gw");
	if (MySystemCmd("route del default eth0") == -1)
		MySystemCmd("route del default eth0");
	char file[128]={0};
	sprintf(file,"route del default %s",stDevFile.wifiEth);
    if (MySystemCmd(file) == -1)
        MySystemCmd(file);
    if (MySystemCmd("route del default ppp0") == -1)
        MySystemCmd("route del default ppp0");
    
    if (iFlag == 0)
        goto ETH0_DHCP;
    if (stNetParam.wifiEnable == 1)
    {
        sprintf(file,"udhcpc -i %s &",stDevFile.wifiEth);
        if (MySystemCmd(file) == -1)
            MySystemCmd(file);
    }
    else
    {
ETH0_DHCP:
        if (MySystemCmd("udhcpc -i eth0 &") == -1)
            MySystemCmd("udhcpc -i eth0 &");
        g_iDhcpTimeout= 12;
        g_iDhcpEnable = 1;
    }
}

/**********************************************************/
//名称:					start_wifi
//功能:启动ADSL功能，可以通过有线网卡和wifi网卡进行拨号
//参数:	input	网络参数
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
int g_iWifiTimeout=60;
int g_iWifiEnable =0;
void start_wifi(void)
{
	net_set_default();
	
	char cCmd[1024]={0};	                
	if (stNetParam.wifiDhcp==1)
		sprintf(cCmd,"ifconfig %s 0.0.0.0 up",stDevFile.wifiEth);
	else
		sprintf(cCmd,"ifconfig %s %d.%d.%d.%d up",stDevFile.wifiEth,stNetParam.wifiIP[0],stNetParam.wifiIP[1],stNetParam.wifiIP[2],stNetParam.wifiIP[3]);
	if (MySystemCmd(cCmd) == -1)                        //初始化wifi的内部ip
        MySystemCmd(cCmd);
    unsigned int wifi_ip;
    memcpy(&wifi_ip,stNetParam.wifiIP,4);
    if ((wifi_ip==0) || (wifi_ip==0xffffffff))          //如果wifi的ip没有设置那么自动开启wifi DHCP
        stNetParam.wifiDhcp = 1;
    
    unsigned int gateway;
    memcpy(&gateway,stNetParam.wifigateway,4);
    if ((gateway!=0) && (gateway!=0xffffffff))
    {
        sprintf(cCmd,"route add default gw %d.%d.%d.%d %s",stNetParam.wifigateway[0],stNetParam.wifigateway[1],stNetParam.wifigateway[2],stNetParam.wifigateway[3],stDevFile.wifiEth);
        if (MySystemCmd(cCmd) == -1)
            MySystemCmd(cCmd);
    }

	if (stNetParam.wifiMode == 0)
	{
		if (!memcmp(stDevFile.wifiEth, "ra", 2))
		{
			sprintf(cCmd,"iwpriv %s set NetworkType=Infra\niwpriv %s set AuthMode=OPEN\niwpriv %s set EncrypType=NONE\niwpriv %s set SSID=\"%s\"",
				stDevFile.wifiEth,stDevFile.wifiEth,stDevFile.wifiEth,stDevFile.wifiEth,stNetParam.wifiSSID);
		}
		else
		{
			sprintf(cCmd,"iwconfig %s mode managed\niwconfig %s essid \"%s\"\niwconfig %s rate auto\niwconfig %s essid \"%s\"\n",
			stDevFile.wifiEth,stDevFile.wifiEth,stNetParam.wifiSSID,stDevFile.wifiEth,stDevFile.wifiEth,stNetParam.wifiSSID);
		}
	}
	else if (stNetParam.wifiMode == 1)
	{
		if (!memcmp(stDevFile.wifiEth, "ra", 2))
		{
			sprintf(cCmd,"iwpriv %s set NetworkType=Infra\niwpriv %s set AuthMode=OPEN\niwpriv %s set EncrypType=WEP\niwpriv %s set DefaultKeyID=%d\niwpriv %s set Key%d=\"%s\"\niwpriv %s set SSID=\"%s\"\n",
				stDevFile.wifiEth,stDevFile.wifiEth,stDevFile.wifiEth,stDevFile.wifiEth,stNetParam.wifiNum,stDevFile.wifiEth,stNetParam.wifiNum,stNetParam.wifiCode,stDevFile.wifiEth,stNetParam.wifiSSID);
		}
		else
		{
	        FILE *fd;
	        fd = fopen("/tmp/wpa_supplicant.conf", "w+");
	        if (fd != NULL)
	        {
	            char keycode[48]={0};
	            if ((strlen((char*)(stNetParam.wifiCode)) == 10) || (strlen((char*)(stNetParam.wifiCode)) == 26))
	                stNetParam.wifiASC = 0;
	            else
	                stNetParam.wifiASC = 1;
	            
	            if (stNetParam.wifiASC==1)
	            {
	                sprintf(keycode,"\"%s\"",stNetParam.wifiCode);
	            }
	            else
	                sprintf(keycode,"%s",stNetParam.wifiCode);
	            
	            sprintf(cCmd,"network={\nssid=\"%s\"\nkey_mgmt=NONE\ngroup=WEP40 WEP104\nwep_key%d=%s\n wep_tx_keyidx=%d\npriority=5\n}\n",
	                    stNetParam.wifiSSID,stNetParam.wifiNum-1,keycode,stNetParam.wifiNum-1);
	            fwrite(cCmd, 1, strlen(cCmd), fd);
	            fclose(fd);
	            if (!memcmp(stDevFile.wifiEth, "wlan", 4))
	                sprintf(cCmd,"wpa_supplicant -i %s -D wext -c /tmp/wpa_supplicant.conf -B", stDevFile.wifiEth);
	            else
	                sprintf(cCmd,"wpa_supplicant -B -i%s -c/tmp/wpa_supplicant.conf -Dzydas &", stDevFile.wifiEth);
	        }
	        else
	            cCmd[0] = 0;
		}
	}
	else if ((stNetParam.wifiMode == 2) || (stNetParam.wifiMode == 3))
	{
		if (!memcmp(stDevFile.wifiEth, "ra", 2))
		{
			sprintf(cCmd,"iwpriv %s set NetworkType=Infra\niwpriv %s set AuthMode=%s\niwpriv %s set EncrypType=%s\niwpriv %s set SSID=\"%s\"\niwpriv %s set WPAPSK=\"%s\"\niwpriv %s set SSID=\"%s\"\n",
				stDevFile.wifiEth,stDevFile.wifiEth,(stNetParam.wifiMode==2)?"WPAPSK":"WPA2PSK",stDevFile.wifiEth,(stNetParam.wifiNum==2)?"TKIP":"AES",stDevFile.wifiEth,stNetParam.wifiSSID,stDevFile.wifiEth,stNetParam.wifiCode,stDevFile.wifiEth,stNetParam.wifiSSID);
		}
		else
		{
			FILE *fd;
			fd = fopen("/tmp/wpa_supplicant.conf", "w+");
			if (fd != NULL)
			{
				sprintf(cCmd,"network={\nssid=\"%s\"\nproto=%s\nkey_mgmt=WPA-PSK\npairwise=CCMP TKIP\ngroup=CCMP TKIP WEP104 WEP40\npsk=\"%s\"\npriority=2\n}\n",
						stNetParam.wifiSSID,(stNetParam.wifiMode==2)?"WPA":"RSN WPA",stNetParam.wifiCode);
				fwrite(cCmd, 1, strlen(cCmd), fd);
				fclose(fd);
				if (!memcmp(stDevFile.wifiEth, "wlan", 4))
					sprintf(cCmd,"wpa_supplicant -i %s -D wext -c /tmp/wpa_supplicant.conf -B", stDevFile.wifiEth);
				else
					sprintf(cCmd,"wpa_supplicant -B -i%s -c/tmp/wpa_supplicant.conf -Dzydas &", stDevFile.wifiEth);
			}
			else
				cCmd[0] = 0;
		}
	}
	if (strlen(cCmd) != 0)
	{
	    if (MySystemCmd(cCmd) == -1)
            MySystemCmd(cCmd);
	}
	else
        printf("start_wifi:执行命令为空\n");
	g_iWifiTimeout= 12;
	g_iWifiEnable = 1;
    sleep(2);
}
/**********************************************************/
//名称:					diagnose_wifi
//功能:诊断wifi网卡，获取wifi网卡的ip
//参数:	input	网络参数
//		output
//作者:	季增光
//时间:	2011-05-08
/**********************************************************/
void diagnose_wifi()
{
	int sock;
	struct ifreq ifr;
	struct sockaddr_in addr;

    if (strlen(stDevFile.wifiEth) == 0)
	{
	    stNetStatus.wifi.iIP = -1;
	    stNetStatus.wifi.sDevice = PHY_STATUS_NOCARD;
	    return ;
	}
    sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0)
	{
		perror("diagnose_wifi:创建sock失败");
		return;
	}

    strcpy(ifr.ifr_name,stDevFile.wifiEth);
    if (ioctl(sock,SIOCGIFFLAGS,(char *)&ifr) >= 0)
    {
        if (ifr.ifr_flags&IFF_UP)
            stNetStatus.wifi.sDevice = PHY_STATUS_LINK100M;
        else
            stNetStatus.wifi.sDevice = PHY_STATUS_NOCARD;
    }
    else
        stNetStatus.wifi.sDevice = PHY_STATUS_IOCTRL_ERR;

    if (stNetStatus.wifi.sDevice >= PHY_STATUS_LINK10M)
    {
        if (ioctl(sock,SIOCGIFADDR,(char *)&ifr) < 0)
        {
            stNetStatus.wifi.iIP = -1;

            stNetStatus.wifi.sDevice = PHY_STATUS_IOCTRL_ERR;
            if (g_iWifiEnable == 1)
            {
            	printf("WIFI连接断开，获取wifi_IP失败！！！！\n");
            }
        }
        else
        {
            struct in_addr in=((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr;
        	inet_aton(inet_ntoa(in),&addr.sin_addr);
            stNetStatus.wifi.iIP = addr.sin_addr.s_addr;
        }
    }
    else
        stNetStatus.wifi.iIP = -1;

    close(sock);
}
void stop_Wifi(void)
{
	char cCmd[128];
    sprintf(cCmd, "killall iwpriv");
	if (MySystemCmd(cCmd) == -1)
        MySystemCmd(cCmd);
    sprintf(cCmd, "killall iwconfig");
	if (MySystemCmd(cCmd) == -1)
        MySystemCmd(cCmd);
    sprintf(cCmd, "killall wpa_supplicant");
	if (MySystemCmd(cCmd) == -1)
        MySystemCmd(cCmd);
    sprintf(cCmd, "killall udhcpc");
	if (MySystemCmd(cCmd) == -1)
        MySystemCmd(cCmd);
	sprintf(cCmd,"ifconfig %s %d.%d.%d.%d up",stDevFile.wifiEth,0,0,0,0);
	if (MySystemCmd(cCmd) == -1)
        MySystemCmd(cCmd);
    g_iWifi_Running = 0;
}

void HostAP_manage(void)
{
	char cCmd[128];
	if (stNetParam.wifiEnable == 0)
	{
		if (g_iHostAPEnable == 0)
		{
			FILE * apConfig = fopen("/etc/hostapd.conf","r");
			if (apConfig != NULL)
			{
				char ap_conf[4096];
				fread(ap_conf,1,4096,apConfig);
				fclose(apConfig);
				char *p=strstr(ap_conf,"ssid=");
				if (p != NULL)
				{
					sprintf(p+5,"%s%s\n",stDevFile.OEM_Type,&stNetParam.macAddress[4]);
					apConfig = fopen("/tmp/hostapd.conf","w+");
					if (apConfig != NULL)
					{
						fwrite(ap_conf,1,strlen(ap_conf)+1,apConfig);
						fclose(apConfig);
						sprintf(cCmd,"hostapd /tmp/hostapd.conf -B");
						if (MySystemCmd(cCmd) == -1)
					        MySystemCmd(cCmd);
						g_iHostAPEnable = 1;
					}
				}
			}
		}
	}
	else
	{
		if (g_iHostAPEnable == 1)
		{
			if (stNetStatus.wifi.iIP != -1)
			{
				sprintf(cCmd,"killall hostapd");
				if (MySystemCmd(cCmd) == -1)
			        MySystemCmd(cCmd);
				g_iHostAPEnable = 0;
			}
		}
	}
}

int net_status=0;
pthread_t th_net_diagnose;
void * net_diagnose(void * net)
{
	st_eth0_t stEth0;
	memcpy(&stEth0,stNetParam.localIP,sizeof(st_eth0_t));
	stEth0.dhcpEnable = stNetParam.dhcpEnable;
	start_eth0();
	GPIO_Set_Dir(stDevFile.LampPort,stDevFile.LampBit,1);
	GPIO_Control(stDevFile.LampPort,stDevFile.LampBit,GPIO_TYPE_CYCLE,1000,1);
    st_wlan_t st_wlan;
	memcpy(&st_wlan,&stNetParam.wifiEnable,sizeof(st_wlan_t));
	st_wlan.wifiDhcp = stNetParam.wifiDhcp;
    if (stNetParam.wifiEnable == 1)
	{
		stNetParam.wifiSSID[32] = 0;
		stNetParam.wifiCode[32] = 0;
		char file[64];
		sprintf(file,"ifconfig %s %d.%d.%d.%d up",stDevFile.wifiEth,0,0,0,0);
		if (MySystemCmd(file) == -1)
			MySystemCmd(file);
	}
	
	st_pppoe_t st_pppoe;
	memcpy(&st_pppoe,&stNetParam.pppoeEnable,sizeof(st_pppoe_t));
	
	int webServicePort = stNetParam.webServicePort;
	
    int iEth0Link=1;
    int iReadParam=10;
    while(net_status == 1)
	{
		iReadParam++;
		if (iReadParam >= 10)
		{
			iReadParam = 0;
			netFileGet(&stNetParam);
		
		    check_eth0_link();
		    if (stNetStatus.lan.sDevice >= PHY_STATUS_LINK10M)
		    {													//插着网线走有线
		        if (iEth0Link == 0)
		        {
		            iEth0Link = 1;
		            if (stNetParam.wifiEnable == 1)
		            {
		                printf("启动wifi后插上网线需要断开wifi\n");
	                    stop_Wifi();
	                    memset(stEth0.localIP,0,4);				//目的是为了重新设置eth0的ip和路由
	                }
		        }
		        if ((memcmp(stEth0.localIP,stNetParam.localIP,4)) || (memcmp(stEth0.netMask,stNetParam.netMask,4)) || (memcmp(stEth0.gateIP,stNetParam.gateIP,4))
	             || ((stEth0.dhcpEnable!=stNetParam.dhcpEnable)&&(stNetParam.dhcpEnable==0)))
		        {
		        	memcpy(&stEth0,stNetParam.localIP,sizeof(st_eth0_t));
					stEth0.dhcpEnable = stNetParam.dhcpEnable;
		            start_eth0();
		        }
		        if (stNetParam.dhcpEnable == 1)
		        {
		        	if ((g_iDhcpEnable==0) && (g_iDhcpResume==0))
		                start_dhcp(0);
	                else
	                {
	                    if (g_iDhcpResume != 0)
	                        g_iDhcpResume--;
	                    if (stNetStatus.lan.iIP == -1)
	                    {
	                        if (g_iDhcpTimeout != 0)
	                        {
	                            g_iDhcpTimeout--;
	                            if (g_iDhcpTimeout == 0)
	                            {
	                                start_eth0();
	                                memcpy(&stEth0,stNetParam.localIP,sizeof(st_eth0_t)-4);
	                                g_iDhcpEnable = 0;
	                                g_iDhcpResume = 12;
	                            }
	                        }
	                    }
	                    else
	                    { 
	                    	g_iDhcpTimeout = 1;
	                    	if (g_iDhcpResume == 0)
	                    	{
	                    		if (net_get_RouteIP() == 0)                      //dhcp获取到ip但是获取不到默认路由则重新启动dhcp
			                    {
			                        g_iDhcpEnable = 0;
			                        g_iDhcpResume = 0;
			                    }
			                }
		                }
					}
		        }
				if (stNetParam.pppoeEnable == 1)
		        {
	                if (g_iAdslEnable == 0)
	                {
	                    start_adsl();
	                    g_iAdsl_Running = 1;
	                }
	                else
	                {
	                    if (stNetStatus.adsl.iIP == -1)
	                    {
	                        if (g_iAdslTimeout != 0)
	                        {
	                            g_iAdslTimeout--;
	                            if (g_iAdslTimeout == 0)
	                                g_iAdslEnable = 0;
	                        }
	                    }
	                    else
	                    	g_iAdslTimeout = 1;
	                }
		        }
		        else
		        {
		            if (g_iAdsl_Running==1)
		            {
		            	g_iAdslEnable = 0;
		                g_iAdsl_Running=0;
		                memset(stEth0.localIP,0,4);				//目的是为了重新设置eth0的ip和路由
		            }
		        }
		        if (stNetParam.wifiEnable == 0)
		        	g_iWifiLinkStatus = 0;						//插着网线同时wifi未设置
		        else
		        	g_iWifiLinkStatus = 1;						//wifi参数已经设置但插着网线
			}
			else
			{
		    	if (iEth0Link == 1)
	            {
	                iEth0Link = 0;
	                g_iAdslEnable = 0;
	                g_iDhcpEnable = 0;
	                g_iDhcpResume = 0;
	                if (stNetParam.wifiEnable == 1)
	                {
	                	if (MySystemCmd("ifconfig eth0 0.0.0.0") == -1)
	                        MySystemCmd("ifconfig eth0 0.0.0.0");
	                    if (MySystemCmd("ifconfig eth0:1 down") == -1)
	                    	MySystemCmd("ifconfig eth0:1 down");
	                }
	            }
	            if (stNetParam.wifiEnable == 1)
	            {
	                if (g_iWifiEnable == 0)                 	//启动wifi功能
	                {
	                	GPIO_Control(stDevFile.LampPort,stDevFile.LampBit,GPIO_TYPE_CYCLE,4000,1);
	                    start_wifi();                       	//启动wifi的DHCP功能
	                    if (stNetParam.wifiDhcp == 1)
	                       start_dhcp(1);
	                    g_iWifiTimeout = 12;
	                    g_iWifiEnable = 1;
	                    g_iWifiLinkStatus = 3;					//开始连接或者正在连接
	                }
	                else if (stNetStatus.wifi.iIP == -1) 		//wifi的ip如果没有获取到则超时处理重新连接，超时60秒
	                {
	                    if (g_iWifiTimeout != 0)
	                    {
	                        g_iWifiTimeout--;
	                        if (g_iWifiTimeout == 0)
	                        {
	                        	stop_Wifi();
	                        	g_iAdslEnable = 0;
	                        	g_iWifiEnable = 0;
	                        }
	                    }
	                }
					else
					{
						GPIO_Control(stDevFile.LampPort,stDevFile.LampBit,GPIO_TYPE_ONCE,300,1);
						g_iWifiLinkStatus = 4;					//连接成功
						g_iWifiTimeout = 1;
						if (stNetParam.pppoeEnable == 0)
						{
						    if (net_get_RouteIP() == 0)         //dhcp获取到ip但是获取不到默认路由则重新启动dhcp
						    {
						    	stop_Wifi();
						    	g_iAdslEnable = 0;
						    	g_iWifiEnable = 0;
						    }
						}
					}
	                                                        	//开了wifi和adsl走家庭热点
	                if ((stNetParam.wifiEnable==1) && (stNetParam.pppoeEnable==1))
	                {
	                	if (stNetStatus.wifi.iIP != -1)
	                	{
		                    if (g_iAdslEnable == 0)
		                    {
		                        start_adsl();
		                        g_iAdsl_Running = 1;
		                    }
		                    else
		                    {
		                        if (stNetStatus.adsl.iIP == -1)
		                        {
		                            if (g_iAdslTimeout != 0)
		                            {
		                                g_iAdslTimeout--;
		                                if (g_iAdslTimeout == 0)
		                                    g_iAdslEnable = 0;
		                            }
		                        }
		                        else
		                        {
		                            g_iAdslTimeout = 1;
		                            if (net_get_RouteIP() == 0)
			                        {
			                        	printf("wifi adsl路由丢失重新连接\n");
RELINK_WIFI:		                        	
			                        	stop_Wifi();
								    	g_iAdslEnable = 0;
								    	g_iWifiEnable = 0;
								    	iEth0Link = 1;
			                        }
		                        }
		                    }                		
	                	}
	                	else
	                		printf("Waiting wifi link sucess to dial PPPOE");
	                }
	                else
	                {
	                    if (stNetParam.pppoeEnable == 0)
	                    {
	                        if (g_iAdsl_Running==1)
	                        {
	                            g_iAdsl_Running = 0;
	                            goto RELINK_WIFI;
	                        }
	                    }
	                }
	            }
	            else
	            	g_iWifiLinkStatus = 2;						//网线拔掉了但wifi未设置
	        }
			
	        if (webServicePort != stNetParam.webServicePort)
	        {
	            webServicePort = stNetParam.webServicePort;
	            char webPort[8]={0};
				FILE *webFIle =NULL;
				if ((webServicePort==0) || (webServicePort==0xffff))
					webServicePort = 80;
				sprintf(webPort,"%d\r\n",webServicePort);
				webFIle = fopen("/dvs/webPort.bin","w+");
				if (webFIle != NULL)
				{
					fwrite(webPort,1,strlen(webPort),webFIle);
					fclose(webFIle);
				}
				if (MySystemCmd("killall thttpd") == -1)
	 	 			MySystemCmd("killall thttpd");
	        }
	
	        if (memcmp(&stNetParam.pppoeEnable,&st_pppoe,sizeof(st_pppoe_t)))
	        {
	            memcpy(&st_pppoe,&stNetParam.pppoeEnable,sizeof(st_pppoe_t));
	            if (stNetStatus.lan.sDevice >= PHY_STATUS_LINK10M)
	            {
	            	if (g_iAdsl_Running==1)
		            {
		                g_iAdslEnable = 0;
		                g_iAdsl_Running=0;
		                memset(stEth0.localIP,0,4);				//目的是为了重新设置eth0的ip和路由
		            }
	            }
	            else
	            {
	            	if (g_iAdsl_Running==1)
	            		goto RELINK_WIFI;
	            }
	        }
	        
	        if ((memcmp(&stNetParam.wifiNum,&st_wlan.wifiNum,sizeof(st_wlan_t)-2)) || (stNetParam.wifiDhcp!=st_wlan.wifiDhcp))
	        {
	        	if (stNetStatus.lan.sDevice < PHY_STATUS_LINK10M)
	            {
		            if (g_iAdsl_Running==1)
		            {
		            	g_iAdsl_Running=0;
		            	stop_Wifi();
				    	g_iAdslEnable = 0;
				    	g_iWifiEnable = 0;
				    	iEth0Link = 1;
		            }
		            else
		            {	
		            	stop_Wifi();
				    	g_iWifiEnable = 0;
		            }
		            memcpy(&st_wlan,&stNetParam.wifiEnable,sizeof(st_wlan_t));
					st_wlan.wifiDhcp = stNetParam.wifiDhcp;
		        }
	        }
	
			diagnose_eth0();
			diagnose_adsl();
			diagnose_wifi();
			HostAP_manage();
		}
		usleep(500000);
	}
	if (MySystemCmd("pppoe-stop") == -1)
		MySystemCmd("pppoe-stop");
	stop_Wifi();
	if (MySystemCmd("killall hostapd") == -1)
        MySystemCmd("killall hostapd");
	return net;
}

void NET_Manage_Open(void)
{
	devFileGet(&stDevFile);
	netFileGet(&stNetParam);
	pthread_mutex_init(&mutex_command,NULL);
	stNetStatus.lan.iIP  = -1;
	stNetStatus.adsl.iIP = -1;
	stNetStatus.wifi.iIP = -1;
	net_status = 1;
	pthread_create(&th_net_diagnose,NULL,net_diagnose,NULL);
	usleep(100000);
	ddns_create();
}

void NET_Manage_Close(void)
{
	ddns_destroy();
	net_status = 0;
	pthread_join(th_net_diagnose,NULL);
	pthread_mutex_destroy(&mutex_command);
}
