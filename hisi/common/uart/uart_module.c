#include "uart_module.h"
#include "../gpio/gpio_module.h"

static int g_iUartInit=0;
struct UART_MODULE uart[3];

const int TableUart[9] = {B1200,B2400,B4800,B9600,B19200,B38400,B57600,B115200,B4000000};
/*********************************************************/
//串口处理模块
/*********************************************************/
void * uart_receive(void * rev)
{
	int i = (int)uart;
	int iRlen;
	char cRecv[1024];
	while(uart[i].iRcvThread == 1)
	{
		if (uart[i].receive != 0)
		{
			iRlen = uart[i].receive(uart[i].iFd,cRecv,1024);
			if (iRlen > 0)
			{
				if (uart[i].protocol != 0)
					uart[i].protocol(uart[i].iFd,cRecv,iRlen);
			}
			else
				usleep(100000);
		}
		else
			sleep(1);
	}
	return rev;
}

int UART_Open(char * pName,UART_RECEIVE_CALLBACK receive,UART_PROTOCOL_CALLBACK protocol)
{
	int i;
	if (g_iUartInit == 0)
	{
		g_iUartInit = 1;
		for (i=0; i<3; i++)
			uart[i].iFd = -1;
	}
	for (i=0; i<3; i++)
	{	
		if (uart[i].iFd == -1)
			break;
	}
	if (i >= 3)
		return -1;
	
    if (uart[i].iFd < 0)
        uart[i].iFd = open(pName,O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (uart[i].iFd < 0)
    {
        perror("open com port error\n");
        return  -2;
    }
    uart[i].attr.ubaud = 3;
    uart[i].attr.udata = 8;
    uart[i].attr.ucheck = 0;
    uart[i].attr.ustop = 1;
    uart[i].ctrl.enable = 0;
    uart[i].receive = 0;
    uart[i].protocol = 0;
    uart[i].iRcvThread = 1;
    pthread_create(&uart[i].thread_recv,NULL,uart_receive,(void *)i);
    usleep(100000);
    
    return i;
}
/*********************************************************/
//设置波特率\校验方式
/*********************************************************/
int UART_Set_Attr(int handle,st_uart_attr_t * attr,st_485_ctrl_t *ctrl)
{
    struct termios Opt;
    if (tcgetattr(uart[handle].iFd,&Opt) != 0)
    {
        printf("uartFd %d uart addr %d\n",uart[handle].iFd,(int)uart[handle].attr.ubaud);
        perror("set baud get com port param error\n");
UART_FAIL:
        close(uart[handle].iFd);
        uart[handle].iFd = -1;
        return -1;
    }
    tcflush(uart[handle].iFd,TCIOFLUSH);
    cfsetispeed(&Opt,TableUart[(int)uart[handle].attr.ubaud]);
    cfsetospeed(&Opt,TableUart[(int)uart[handle].attr.ubaud]);
    if  (tcsetattr(uart[handle].iFd,TCSANOW,&Opt) != 0)
    {
        perror("set com baud rate error\n");
        goto UART_FAIL;
    }
    tcflush(uart[handle].iFd,TCIOFLUSH);

    if (tcgetattr(uart[handle].iFd,&Opt) != 0)
    {
        perror("set parity get com port param error\n");
        goto UART_FAIL;
    }

    Opt.c_cflag &= ~CSIZE;
    Opt.c_iflag&=~(IGNBRK|IGNCR|INLCR|ICRNL|IUCLC|IXANY|IXON|IXOFF|INPCK|ISTRIP);
    Opt.c_iflag|=(BRKINT|IGNPAR);
    Opt.c_oflag &= ~OPOST;
    Opt.c_lflag &= ~(ECHO|ECHONL|ISIG|IEXTEN|ICANON);
    switch(uart[handle].attr.udata)
    {
        case 7:
            Opt.c_cflag |= CS7;
            break;
        default:
            Opt.c_cflag |= CS8;
            break;
    }
    switch(uart[handle].attr.ucheck)
    {
        case 1:
            Opt.c_cflag |= (PARODD|PARENB);     //设置为奇效验
            Opt.c_iflag |= INPCK;
            break;
        case 2:
            Opt.c_cflag |= PARENB;              //设置为偶效验
            Opt.c_cflag &= ~PARODD;
            Opt.c_iflag |= INPCK;
            break;
        default:
            Opt.c_cflag &= ~PARENB;             //无校验
            Opt.c_iflag &= ~INPCK;              /* Enable parity checking */
            break;
    }
    switch (uart[handle].attr.ustop)
    {
        case 2:
            Opt.c_cflag |= CSTOPB;
            break;
        default:
            Opt.c_cflag &= ~CSTOPB;
            break;
	}

//    switch(uart->property.uartIXON)
//    {
//        case 2:
//            Opt.c_cflag |= IXON|IXOFF|IXANY;
//            break;
//        case 1:
//            Opt.c_cflag |= CRTSCTS;
//            break;
//        case 0:
//        default:
//            Opt.c_cflag &= ~CRTSCTS;
//            break;
//    }

    if (uart[handle].attr.ucheck != 0)
  		Opt.c_iflag |= INPCK;
    Opt.c_cc[VTIME] = 150;
    Opt.c_cc[VMIN] = 2;
    tcflush(uart[handle].iFd,TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(uart[handle].iFd,TCSANOW,&Opt) != 0)
  	{
  		perror("set parity set com port parity error\n");
		goto UART_FAIL;
	}
	memcpy(&uart[handle].ctrl,ctrl,sizeof(st_485_ctrl_t));
	if (uart[handle].ctrl.enable == 1)
	{
		GPIO_Set_Dir(uart[handle].ctrl.gpio_group,uart[handle].ctrl.gpio_bit,1);
		GPIO_Set_Value(uart[handle].ctrl.gpio_group,uart[handle].ctrl.gpio_bit,0);
	}
    return 0;
}

int UART_Send(int handle,char * pData,int iLen)
{
	int i;
	for (i=0; i<3; i++)
	{
		if (uart[i].iFd == handle)
			break;
	}
	if (i >= 3)
		return -1;
	
	if (uart[i].ctrl.enable == 1)
		GPIO_Set_Value(uart[i].ctrl.gpio_group,uart[i].ctrl.gpio_bit,1);
	if (write(uart[i].iFd,pData,iLen) != iLen)
    {
    	if (uart[i].ctrl.enable == 1)
			GPIO_Set_Value(uart[i].ctrl.gpio_group,uart[i].ctrl.gpio_bit,0);
		perror("UART_Send failed!!\n");
        return -2;
    }
    if (uart[i].ctrl.enable == 1)
	{
		if (uart[i].attr.ubaud==0)
			usleep(10000*iLen);
        else if (uart[i].attr.ubaud==1)
        	usleep(5000*iLen);
        else if (uart[i].attr.ubaud==2)
        	usleep(2500*iLen);
        else
        	usleep(1200*iLen);
		GPIO_Set_Value(uart[i].ctrl.gpio_group,uart[i].ctrl.gpio_bit,0);
	}
	return iLen;
}

int UART_Close(int handle)
{
	if (uart[handle].iRcvThread != 0)
	{
		uart[handle].iRcvThread = 0;
		pthread_join(uart[handle].thread_recv,NULL);
	}
	if (uart[handle].iFd != -1)
		close(uart[handle].iFd);
	uart[handle].iFd = -1;
	return 0;
}
