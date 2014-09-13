#ifndef __MY_GPIO_MODULE_H__
#define __MY_GPIO_MODULE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../common_define.h"
#include "gpio/hi_gpio.h"

//同时最多控制gpio输出的管脚个数
#define MAX_CONTROL_GPIO	18

typedef struct ST_GPIO
{
    e_gpio_control enCtrl;
    unsigned int port;
    unsigned int bit;
    unsigned int value;
    unsigned int ms_value;
    unsigned int ms_current;
}	st_gpio_def;

extern int GPIO_Set_Dir(int gpio_group,int gpio_bit,int value);
extern int GPIO_Set_Value(int gpio_group,int gpio_bit,int value);

#endif	//__MY_GPIO_MODULE_H__

