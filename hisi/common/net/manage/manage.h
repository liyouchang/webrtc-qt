#ifndef __MY_NET_MANAGE_H__
#define __MY_NET_MANAGE_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
//#ifndef __KERNEL__
//#define __KERNEL__
//#include <asm/types.h>
//#undef __KERNEL__
//#endif
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include "ddns/ddns.h"
#include "../../common_define.h"

#define PHY_STATUS_UNLINK           0x00    //未插网线
#define PHY_STATUS_NOCARD           0x01    //未找到网卡设备
#define PHY_STATUS_IOCTRL_ERR       0x02    //执行ioctrl失败
#define PHY_STATUS_IOCTRL_ILLEGAL   0x03    //执行ioctrl获取非法值
#define PHY_STATUS_LINK10M          0x10    //未找到网卡设备
#define PHY_STATUS_LINK100M         0x11    //未找到网卡设备

typedef struct net_eht0
{
    unsigned char   localIP[4];             			//IP地址
    unsigned char   gateIP[4];							//网关
    unsigned char   netMask[4];             			//子网掩码
    int dhcpEnable;
}	st_eth0_t;

typedef struct net_pppoe
{
	unsigned char   pppoeEnable;
    unsigned char   pppoeIPType;
    unsigned char   pppoeUser[30];
    unsigned char   pppoePass[30];

}	st_pppoe_t;

typedef struct net_wlan
{
	unsigned char   wifiEnable;
    unsigned char   wifiNum;
    unsigned char   wifiMode;
    unsigned char   wifiASC;                			//十六进制(0)ASCII(1) TKIP(0) AES(1)
    unsigned char   wifiIP[4];
    unsigned char   wifimask[4];
    unsigned char   wifigateway[4];
    unsigned char   wifiSSID[33];
    unsigned char   wifiCode[33];
    unsigned char   wifiDhcp;

}	st_wlan_t;

extern int g_iWifiLinkStatus;
extern void NET_Manage_Open(void);
extern int NET_Get_WanStatus(void);
extern int NET_Get_RouteIP(char * cip);
extern int NET_Get_Status(st_net_status_t * status);
extern void NET_Manage_Close(void);

#endif	//__MY_NET_MANAGE_H__
