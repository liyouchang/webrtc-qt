#include "output.h"

int outputNum=MAX_ALARMOUTPUTS;
struct ALMOUTPUT outputParam[MAX_ALARMOUTPUTS];
CONFIG_CALLBACK outputCallBack=0;

static void setDefaultOutput(struct ALMOUTPUT *this,struct DEVHARDPARAM * dev)
{
    int i = 0;
	for(i=0; i<outputNum; i++)
	{
		memset(&this[i], 0, sizeof(struct ALMOUTPUT));
		(&this[i])->AlarmLong = 20;
		(&this[i])->Enable = 1;
	}
}

int outputFileRead(struct DEVHARDPARAM * dev)
{
	int i=0;
	FILE *fp = NULL;
	outputNum = dev->OutputAlarmNum;
	
	if((fp=fopen("/dvs/AlmOut.bin","r")) == NULL)
	{
		perror("打开网络参数配置文件 读 失败\n");
		setDefaultOutput(outputParam,dev);
	}
	else
	{
		for(i=0; i<outputNum; i++)
			fread(&outputParam[i],1,sizeof(struct ALMOUTPUT),fp);
		fclose(fp);
	}
	return 0;
}

int outputFileGet(struct ALMOUTPUT *output)
{
	int i;
	for (i=0; i<outputNum; i++,output++)
		memcpy(output,&outputParam[i],sizeof(struct ALMOUTPUT));
		
	return 0;
}

int outputFileSet(struct ALMOUTPUT *output)
{
	int i;
	for (i=0; i<outputNum; i++,output++)
	{
		if (memcmp(output,&outputParam[i],sizeof(struct ALMOUTPUT)))
			memcpy(&outputParam[i],output,sizeof(struct ALMOUTPUT));
	}
	
	FILE *fp = NULL;
	if ((fp=fopen("/dvs/AlmOut.bin","w+")) == NULL)
		perror("打开网络参数配置文件 写 失败\n");
	else
	{
	    printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
	    fwrite((char *)outputParam,1,sizeof(struct ALMOUTPUT)*outputNum,fp);
		fclose(fp);
	}
	
	if (outputCallBack != 0)
		outputCallBack((void *)outputParam);
	return 0;
}

