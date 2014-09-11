#include "ddns.h"
#include <signal.h>
#include <unistd.h>
#include <ctype.h>

/**********************************************************/
//名称:					gethostbyname_my
/**********************************************************/
int isWanLinked = 0;
int gethostbyname_my(char * host,char * ip)
{
    if ((host==NULL) || (ip==NULL) || (strlen(host)==0))
        return -1;

    int iRet;
    struct addrinfo hint;
    struct addrinfo *answer=NULL;

    bzero(&hint, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    if(getaddrinfo(host,NULL,&hint,&answer) == 0)
    {
    	if (answer != NULL)
        {
            memcpy(ip,&(((struct sockaddr_in *)(answer->ai_addr))->sin_addr),4);
//            printf("%s IP is %d.%d.%d.%d\n",host,ip[0],ip[1],ip[2],ip[3]);
            iRet = 0;
        }
        else
            iRet = -1;
    }
    else
        iRet = -1;
    if (answer != NULL)
        freeaddrinfo(answer);
    return iRet;
}

/**********************************************************/
//名称:				gethostbyname_my
/**********************************************************/
static pthread_t	th_ddns;			//每步、花生壳
static int 			reg_time=0;
static int 			gop_time=0;
static int			ddns_step=0;		//判断是否登陆成功
static int			ddns_status=0;		//ddns状态标志
static struct NETPARAM stNetParam;
static st_dns_server_t dns_server;
static char cDdnsIP[32]={0};			//当前IP
int ddns_Receive(int iIp,short sPort,int iSock,char *pData,int iLen)
{
    char * p = pData;																		//接收到的数据
    char * q = strstr(p,"&&&&&");
    char szTime[4]= {0};
    char szNetip[32]= {0};
    
    if (strstr(p,"Successful landing"))
    {
        ddns_step  = 1;
        printf("登陆成功\n");
    }
    else if (strstr(p,"Unsuccessful landing"))
        printf("登陆失败\n");
    else if (q != NULL)
    {
        memcpy(szTime,p,2);
        szTime[2] = 0;
        sscanf(p+2,"%[^&]",szNetip);
        printf("%s:%d当前IP: %s\n", __FUNCTION__,__LINE__,szNetip);
        if (q != NULL)
        {
            * q = 0;
            if (strcmp(cDdnsIP,szNetip))
            {
                ddns_step  = 1;
                printf("IP已经改变\n");
                strcpy(cDdnsIP,szNetip);
            }
        }
    }
    else
    {
        int i;
        for (i=0; i<iLen; i++)
            printf("%02x ",pData[i]);
        printf("\n");
    }

    return 0;
}
/**********************************************************/
//	发送获取外网IP 协议
/**********************************************************/
int ddns_MeiBuGetIP(void)
{
    char szbuf[16]    = "test&&&&&";														//获取ip协议
    char szServer[32] = "data002.cngame.org";												//服务器域名
    if (gethostbyname_my(szServer,(char *)&dns_server.serverIP) != 0)
    {
        herror("ddns_MeiBuGetIP:hostbyname");
        return 20;
    }
    UDP_Server_Send(dns_server.iSocket,dns_server.serverIP,htons(28585),szbuf,sizeof(szbuf));
    return 30;
}

/**********************************************************/
//请求登陆			ddns_MeiBuRegister
/**********************************************************/
int ddns_MeiBuRegister(char *user,char *pass,char *server,short s_port)
{
    char reg[128] = {0};		                        //消息协议
    char szServer[64] = {0};	                        //域名服务器
	printf("meibu_register.....\n");

    char * p;
    memcpy(szServer,user,1);
    if (szServer[1]!=0)
    {
        printf("szServer[1] %d\n\n\n",szServer[1]);
        szServer[1] = 0;
    }                                                   /*组包发送登陆协议*/
    sprintf(reg,"375614338%s&&&%s&&&iap&&&29611&&&",user,pass);
    p = strstr(server,".");
    if (p != NULL)
    {
        strcat(szServer,"main");
        strcat(szServer,p);
    }
    if (gethostbyname_my(szServer,(char *)&dns_server.serverIP) != 0)
    {
        printf("server: %s\n",szServer);
    	herror("ddns_MeiBuRegister:hostbyname");
        return 2;
    }

    dns_server.usPort = htons(s_port);
    UDP_Server_Send(dns_server.iSocket,dns_server.serverIP,dns_server.usPort,reg,strlen(reg)+1);
    return 3;
}
/**********************************************************/
//状态更新回调
/**********************************************************/
PHGlobal globalt;
static void myOnStatusChanged(int status, long data)
{
    printf("myOnStatusChanged %s", convert_status_code(status));
    if (status == okKeepAliveRecved)
    {
        printf(", IP: %ld", data);
    }
    if (status == okDomainsRegistered)
    {
        printf(", UserType: %ld", data);
    }
    printf("\n");
}
/**********************************************************/
//域名注册回调
/**********************************************************/
static void myOnDomainRegistered(char *domain)
{
    printf("myOnDomainRegistered %s\n", domain);
}
/**********************************************************/
//用户信息XML数据回调
/**********************************************************/
static void myOnUserInfo(char *userInfo, int len)
{
    printf("myOnUserInfo %s\n", userInfo);
}
/**********************************************************/
//域名信息XML数据回调
/**********************************************************/
static void myOnAccountDomainInfo(char *domainInfo, int len)
{
    printf("myOnAccountDomainInfo %s\n", domainInfo);
}

/**********************************************************/
//名称:					ddns_thread
/**********************************************************/
static int dns__status = 0;
void * dns__thread(void * dns)
{
	int dns_ip=-1;
	int tmp_ip=-1;
	int dns_cnt=30;
	char route_ip[32];
	FILE * dnsFile=NULL;
	while(dns__status == 1)
	{
		dns_cnt ++;
		if (dns_cnt > 30)
		{
			dns_cnt = 0;
			NET_Get_RouteIP(route_ip);
			if (!memcmp(route_ip,stNetParam.dnsIP,2))
			{
				stNetParam.dnsIP[0] = 8;
				stNetParam.dnsIP[1] = 8;
				stNetParam.dnsIP[2] = 8;
				stNetParam.dnsIP[3] = 8;
			}
			memcpy(&dns_ip,stNetParam.dnsIP,4);
			if (dns_ip != tmp_ip)
			{
				tmp_ip = dns_ip;
				char cDdnsContent1[128];
				char cDdnsContent2[128];
	        	dnsFile = fopen("/etc/resolv.conf","r");
	            if (dnsFile != NULL)					//以前创建过resolv.conf文件，判断是否要重新创建
	            {
	                fread(cDdnsContent1,1,128,dnsFile);
	                cDdnsContent1[127] = 0;
	                fclose(dnsFile);
DNS_WRITE:
	                sprintf((char *)cDdnsContent2,"nameserver %d.%d.%d.%d\n",stNetParam.dnsIP[0],stNetParam.dnsIP[1],stNetParam.dnsIP[2],stNetParam.dnsIP[3]);
	                if (strcmp((char *)cDdnsContent1,(char *)cDdnsContent2))
	                {
	                	remove("/etc/resolv.conf");
	                	remove("/etc/ppp/resolv.conf");
	                	dnsFile = fopen("/etc/resolv.conf","w+");
	                	if (dnsFile != NULL)
	                    {
	                        printf("配置DNS服务器IP %d.%d.%d.%d\n",stNetParam.dnsIP[0],stNetParam.dnsIP[1],stNetParam.dnsIP[2],stNetParam.dnsIP[3]);
	                        fprintf(dnsFile,"nameserver %d.%d.%d.%d\n",stNetParam.dnsIP[0],stNetParam.dnsIP[1],stNetParam.dnsIP[2],stNetParam.dnsIP[3]);
	                        fclose(dnsFile);
	                    }
	                    system("cp /etc/resolv.conf /etc/ppp/resolv.conf -rf");
	                }
	            }
	            else
	            {
	            	memset(cDdnsContent1,0,128);
	            	goto DNS_WRITE;
	            }				
			}
			sleep(1);
	    	char testIP[32];
	    	if (gethostbyname_my("kaer.cn",testIP) >= 0)
	    		isWanLinked = 1;
	    	else
	    		isWanLinked = 0;
		}
		else
			sleep(1);
	}
	return dns;
}

/**********************************************************/
//名称:					ddns_thread
/**********************************************************/
#include "../../../config/config_net/net.h"
void * ddns_thread(void * ddns)
{
	int iIP;
	netFileGet(&stNetParam);
	dns_server.iSocket = -1;
    printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
    printf("user %s\n",stNetParam.ddns.user);
    printf("pass %s\n",stNetParam.ddns.pass);
    printf("server %s\n",stNetParam.ddns.server);
    printf("port %d\n",(unsigned short)stNetParam.ddns.port);
    printf("serverType %d\n",stNetParam.ddns.serverType);
    printf("dns ip is %d.%d.%d.%d\n",stNetParam.dnsIP[0],stNetParam.dnsIP[1],stNetParam.dnsIP[2],stNetParam.dnsIP[3]);
    printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	memcpy(&iIP,stNetParam.dnsIP,4);
	if ((iIP==-1) || (iIP==0))
	{
		stNetParam.dnsIP[0] = 8;
		stNetParam.dnsIP[1] = 8;
		stNetParam.dnsIP[2] = 8;
		stNetParam.dnsIP[3] = 8;
	}
	pthread_t th_dns;
	dns__status = 1;
    pthread_create(&th_dns,NULL,dns__thread,NULL);
    
    int iSleepCnt =0;
    int oray_flag =0;
    int meibu_flag=0;
    int iMeiBuTOut=0;
    
    int next = 1;
    int nextCnt=0;
    
    while(ddns_status == 1)
    {
        iSleepCnt = 0;
        if (stNetParam.ddns.enable == 1)
        {
            if (stNetParam.ddns.serverType == 1)    	//花生壳
            {
            	if (nextCnt == 0)
            	{
	                if(oray_flag==0)
	                {
	                    oray_flag=1;
	                    init_global(&globalt);
	                    globalt.cbOnStatusChanged = myOnStatusChanged;
	                    globalt.cbOnDomainRegistered = myOnDomainRegistered;
	                    globalt.cbOnUserInfo = myOnUserInfo;
	                    globalt.cbOnAccountDomainInfo = myOnAccountDomainInfo;
	                    set_default_callback(&globalt);
	                    strcpy(globalt.szHost,stNetParam.ddns.server);						//你所拿到的服务器地址
	                    strcpy(globalt.szUserID,stNetParam.ddns.user);						//Oray账号
	                    strcpy(globalt.szUserPWD,stNetParam.ddns.pass);						//以下两个值用户您临时测试（Oray可能随时删除或修改），正式发布前请填写您的实际分配值
	                    globalt.clientinfo   = 0x266D974B;									//这里填写刚才第二步算出的值
	                    globalt.challengekey = 0x1E08AF59;									//这里填写嵌入式认证码
	                }
	                if(meibu_flag==1)
	                {
	                    meibu_flag=0;
	                    if (dns_server.iSocket > 0)
	                    	UDP_Server_Destory(dns_server.iSocket);
	                    dns_server.iSocket = -1;
	                }
	                next = phddns_step(&globalt);
	                if (next == 10001)
	                {
	                    globalt.bStatue = 2;
	                    printf("[%s]: 登录失败！%d\n",__FUNCTION__,globalt.bStatue);
	                    nextCnt = 30;
	                }
	                else
	                	nextCnt = next;
	            }
            }
            else if (stNetParam.ddns.serverType == 0)   									//每步
            {
            	if(meibu_flag==0)
                {
                    meibu_flag=1;
                    dns_server.iSocket=UDP_Server_Create(25555,1460,(UDP_PROTOCOL_CALLBACK)ddns_Receive);
                    if (dns_server.iSocket >= 0)
                    {
	                    ddns_step   = 0;
	                    memset(cDdnsIP,0,sizeof(cDdnsIP));
	                }
                    else
						perror("MeiBu udp server");
                }
                if(oray_flag==1)
                {
                    oray_flag=0;
                    phddns_stop(&globalt);
                    globalt.bTcpUpdateSuccessed = FALSE;									//注销后,状态为0;为未登录                    
                    globalt.bStatue = 0;
                    globalt.bNeed_connect = TRUE;
                }
				
				gop_time++;
				if ((dns_server.iSocket>=0) && (((ddns_step==0)&&(reg_time==0))||(gop_time>=1209600)))
				{
					gop_time = 0;
					reg_time = ddns_MeiBuRegister(stNetParam.ddns.user,stNetParam.ddns.pass,stNetParam.ddns.server,stNetParam.ddns.port);
				}
				if (reg_time != 0)															//三秒钟没有收到回应则重新发送注册
                    reg_time --;
                
                if ((iMeiBuTOut==0) && (dns_server.iSocket>0))
                	iMeiBuTOut = ddns_MeiBuGetIP();
                if (iMeiBuTOut > 0)
					iMeiBuTOut--;
            }
            if (nextCnt != 0)
            	nextCnt--;
        }
    	sleep(1);
    }
    dns__status = 0;
    pthread_join(th_dns,NULL);
    
    if ((stNetParam.ddns.serverType==1) && (oray_flag==1))							//花生壳
    {
        oray_flag=0;
        phddns_stop(&globalt);
        globalt.bTcpUpdateSuccessed = FALSE;										//注销后，状态为0；为未登录
        globalt.bStatue = 0;
        globalt.bNeed_connect = TRUE;
    }
    else if ((stNetParam.ddns.serverType==0) && (meibu_flag==1))					//每步
    {
        meibu_flag=0;
        if (dns_server.iSocket > 0)
        	UDP_Server_Destory(dns_server.iSocket);
        dns_server.iSocket = -1;
    }
    return ddns;
}
/**********************************************************/
//名称:					ddns_create
/**********************************************************/
void ddns_create(void)
{
    printf("%s:%d\n",__FUNCTION__,__LINE__);
    ddns_status = 1;
    pthread_create(&th_ddns,NULL,ddns_thread,NULL);
    usleep(100000);
}

/**********************************************************/
/**					direct_destroy                                            */
/**********************************************************/
void ddns_destroy(void)
{
    ddns_status = 0;
    pthread_join(th_ddns,NULL);
    printf("%s:%d\n",__FUNCTION__,__LINE__);
}

