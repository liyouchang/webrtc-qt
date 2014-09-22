#include "gpio_module.h"
#include "../common_api.h"
#include "../config/config_net/net.h"

static int g_iGpioFd = -1;
st_gpio_def stGpio[MAX_CONTROL_GPIO];

static int gpio_thread_ctrl=0;
pthread_t  gpio_control;
pthread_mutex_t gpio_mutex;

int GPIO_Set_Dir(int gpio_group,int gpio_bit,int value)
{
	if ((g_iGpioFd >=0) && (gpio_group<12))
    {
    	gpio_groupbit_info io;
    	io.groupnumber = gpio_group;
    	io.bitnumber = gpio_bit;
    	io.value = value;
    	ioctl(g_iGpioFd,GPIO_SET_DIR,&io);
    }
    else
        return -1;
    return 0;
}

int GPIO_Set_Value(int gpio_group,int gpio_bit,int value)
{
    if ((g_iGpioFd >=0) && (gpio_group<12))
    {
    	gpio_groupbit_info io;
    	io.groupnumber = gpio_group;
    	io.bitnumber = gpio_bit;
    	io.value = value;
    	ioctl(g_iGpioFd,GPIO_WRITE_BIT,&io);
    }
    else
        return -1;
    return 0;
}

int GPIO_Get_Value(int gpio_group,int gpio_bit)
{
    if ((g_iGpioFd >=0) && (gpio_group<12))
    {
    	gpio_groupbit_info io;
    	io.groupnumber = gpio_group;
    	io.bitnumber = gpio_bit;
    	ioctl(g_iGpioFd,GPIO_READ_BIT,&io);
    	return io.value;
    }
    else
        return -1;
}

int GPIO_Control(int gpio_group,int gpio_bit,e_gpio_control enCtrl,int ms_value,int isHigh)
{
	if ((g_iGpioFd >=0) && (gpio_group<12))
	{
	    ms_value = ms_value/100;
		int i;
		for (i=0; i<MAX_CONTROL_GPIO; i++)
		{
			if ((gpio_group==stGpio[i].port)&&(gpio_bit==stGpio[i].bit))
			{
				break;
			}
		}
		if (i < MAX_CONTROL_GPIO)
		{
			if (enCtrl == GPIO_TYPE_ONCE)
			{	
				if (ms_value == 0)
				{	
					stGpio[i].enCtrl = GPIO_TYPE_COUNT;
					stGpio[i].port = 0x7f;
					stGpio[i].bit = 0x7f;
					stGpio[i].ms_current = 0;
					if (isHigh == stGpio[i].value)
						printf("when stop,we can't set the same state!!!!!!!\n");
					GPIO_Set_Value(stGpio[i].port,stGpio[i].bit,isHigh);
				}
				stGpio[i].ms_value = ms_value;			
			}
			else
			{
				if (ms_value == 0)
				{
					pthread_mutex_lock(&gpio_mutex);
					stGpio[i].enCtrl = GPIO_TYPE_COUNT;
					stGpio[i].port = 0x7f;
					stGpio[i].bit = 0x7f;
					stGpio[i].ms_current = 0;
					GPIO_Set_Value(stGpio[i].port,stGpio[i].bit,isHigh);
					pthread_mutex_unlock(&gpio_mutex);
				}
				else
					stGpio[i].ms_value = ms_value;
			}
		}
		else
		{
			for (i=0; i<MAX_CONTROL_GPIO; i++)
			{
				if (stGpio[i].enCtrl == GPIO_TYPE_COUNT)
				{
					break;
				}
			}
			if (i < MAX_CONTROL_GPIO)
			{
				pthread_mutex_lock(&gpio_mutex);
				stGpio[i].port = gpio_group;
				stGpio[i].bit = gpio_bit;
				stGpio[i].value = isHigh;
				stGpio[i].ms_current = 0;
				if (ms_value == 0)
					stGpio[i].enCtrl = GPIO_TYPE_COUNT;
				else
				{
					stGpio[i].enCtrl = enCtrl;
					stGpio[i].ms_value = ms_value;
				}
				GPIO_Set_Value(stGpio[i].port,stGpio[i].bit,isHigh);
				pthread_mutex_unlock(&gpio_mutex);	
			}
		}
	}
	return 0;
}

void *gpio_thread_control(void *ctrl)
{
	int i;
	int iResetKeyCnt=0;
#ifdef SAFE_MODE
	iResetKeyCnt=50;
#endif	
	struct DEVHARDPARAM stDevFile;
	CONFIG_Get(CONFIG_TYPE_DEV,(void *)&stDevFile);
	GPIO_Set_Dir(stDevFile.ResetKeyPort,stDevFile.ResetKeyBit,0);
	while(gpio_thread_ctrl == 1)
	{
		pthread_mutex_lock(&gpio_mutex);
		for (i=0; i<MAX_CONTROL_GPIO; i++)
		{
			if (stGpio[i].enCtrl != GPIO_TYPE_COUNT)
			{
				if (stGpio[i].enCtrl == GPIO_TYPE_ONCE)
				{
					stGpio[i].ms_current++;
					if (stGpio[i].ms_current >= stGpio[i].ms_value)
					{
						stGpio[i].enCtrl = GPIO_TYPE_COUNT;
						stGpio[i].port = 0x7f;
						stGpio[i].bit = 0x7f;
						stGpio[i].ms_current = 0;
						if (stGpio[i].value==0)
							GPIO_Set_Value(stGpio[i].port,stGpio[i].bit,1);
						else
							GPIO_Set_Value(stGpio[i].port,stGpio[i].bit,0);
					}
				}
				else
				{
					stGpio[i].ms_current++;
					if (stGpio[i].ms_current >= stGpio[i].ms_value)
					{
						stGpio[i].ms_current = 0;
						if (stGpio[i].value==0)
						{	
							stGpio[i].value = 1;
							GPIO_Set_Value(stGpio[i].port,stGpio[i].bit,1);
						}
						else
						{	
							stGpio[i].value = 0;
							GPIO_Set_Value(stGpio[i].port,stGpio[i].bit,0);
						}
					}
				}
			}
		}	
		pthread_mutex_unlock(&gpio_mutex);
		if (GPIO_Get_Value(stDevFile.ResetKeyPort,stDevFile.ResetKeyBit) == 0)
		{
			iResetKeyCnt ++;
			if (iResetKeyCnt == 50)
			{
				system("cp /dvs/confbak/*.bin /dvs/. -rf");
				struct NETPARAM stNetParam;
				netFileGet(&stNetParam);
				stNetParam.wifiEnable=0;
				netFileSet(&stNetParam);
				SYSTEM_Set_RestartApp();
			}
			else if (iResetKeyCnt >= 50)
				iResetKeyCnt = 50;
		}
		else
			iResetKeyCnt = 0;
		
		usleep(100000);
	}
	return ctrl;
}

int GPIO_Open(char *devName)
{
	printf("启动GPIO 接口模块\n");
    g_iGpioFd = open(devName, O_RDWR);
    if (g_iGpioFd < 0)
    	return -1;
	int i;
	for (i=0; i<MAX_CONTROL_GPIO; i++)
	{
		stGpio[i].enCtrl = GPIO_TYPE_COUNT;
		stGpio[i].port = 0x7f;
		stGpio[i].bit = 0x7f;
	}
    pthread_mutex_init(&gpio_mutex,NULL);
    gpio_thread_ctrl = 1;
    pthread_create(&gpio_control,NULL,gpio_thread_control,NULL);
    
    return 0;
}

int GPIO_Close(int handle)
{
    gpio_thread_ctrl = 0;
    pthread_join(gpio_control,NULL);
	pthread_mutex_destroy(&gpio_mutex);
	if (g_iGpioFd >= 0)
		close(g_iGpioFd);
	g_iGpioFd = -1;
	return 0;
}

