#include "system.h"
#include "../common_api.h"

int g_iWtdgFd=-1;
int g_iThreadStatus_system = 0;
pthread_mutex_t mutex_clientcom;
SYSTEM_CALLBACK system_reboot_callback=0;

pthread_t g_thread_system;
void * system_thread(void * this)
{
    printf("system start pid %d thread_id=%d\n",getpid(),(int)pthread_self());
	int iCnt=0;
	int iWatchTime=50;

	g_iWtdgFd = open("/dev/watchdog", O_RDWR);
    if (g_iWtdgFd >= 0)
    	ioctl(g_iWtdgFd,WDIOC_SETTIMEOUT,&iWatchTime);

    while (g_iThreadStatus_system == 1)
    {
        iCnt++;
        if (iCnt >= 10)
        {
            if (g_iWtdgFd >= 0)
                ioctl(g_iWtdgFd,WDIOC_KEEPALIVE,0);
            iCnt = 0;
        }
        cal_mem_occupy();
        cal_cpu_occupy();
        sleep(1);
    }
    close(g_iWtdgFd);								//防止某些模块异常到时线程阻塞，以致设备不能重启
    g_iWtdgFd = -1;
    printf("%s:%d\n",__FUNCTION__,__LINE__);
    return this;
}

int SYSTEM_Initialize(void)
{
	pthread_mutex_init(&mutex_clientcom,NULL);
	g_iThreadStatus_system = 1;
	pthread_create(&g_thread_system,NULL,system_thread,NULL);
	return 0;
}

int SYSTEM_Cleanup(void)
{
	g_iThreadStatus_system = 0;
	pthread_join(g_thread_system,NULL);
	pthread_mutex_destroy(&mutex_clientcom);
	return 0;
}

int SYSTEM_Register_Callback(SYSTEM_CALLBACK system_reboot_callback)
{
	system_reboot_callback=system_reboot_callback;
	return 0;
}

/**********************************************************/
//名称:					system reboot
//功能:系统重新启动
//参数:	input	网络模块类
//		output	0-成功；-1-失败
//作者:	季增光
//时间:	2014-03-07
/**********************************************************/
int iSystemReboot = 0;
int SYSTEM_Set_Reboot(void)
{
	iSystemReboot = 1;
	if (system_reboot_callback != 0)
		system_reboot_callback();
	return 0;
}

/**********************************************************/
//名称:					system reboot app
//功能:应用程序重新启动
//参数:	input	网络模块类
//		output	0-成功；-1-失败
//作者:	季增光
//时间:	2014-03-07
/**********************************************************/
int SYSTEM_Set_RestartApp(void)
{
	iSystemReboot = 2;
	if (system_reboot_callback != 0)
		system_reboot_callback();
	return 0;
}

int SYSTEM_Get_Run(void)
{
    return iSystemReboot;
}

/**********************************************************/
//名称:					system state
//功能:查询系统状态
//作者:	季增光
//时间:	2014-03-7
/**********************************************************/
int SYSTEM_Get_Mem(int *total,int *free)
{
	* total = mem.total;
    * free  = mem.free;
    return memUsageRate;
}

int SYSTEM_Get_Cpu(void)
{
	return cpuUsageRate;
}

int SYSTEM_Command(char *command)
{
	int iRet;
	pthread_mutex_lock(&mutex_clientcom);
	iRet = system(command);
	pthread_mutex_unlock(&mutex_clientcom);
	if (iRet < 0)
        printf("执行%s失败\n",command);
    else
        printf("执行%s完毕\n",command);
	return iRet;
}


