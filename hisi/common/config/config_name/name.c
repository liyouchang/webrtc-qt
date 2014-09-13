#include "name.h"

static struct NAMEPARAM nameParam;
CONFIG_CALLBACK nameCallBack=0;

static void setDefaultName(struct NAMEPARAM *this,struct DEVHARDPARAM * dev)
{
	int i;

	memset(this,0,sizeof(struct NAMEPARAM));
	memcpy(this->nameInfo.devName,dev->DeviceType+2,strlen(dev->DeviceType)-4);
	for (i=0; i<dev->VideoChanNum; i++)
		sprintf(this->nameInfo.chnName[i],"%s%d","channel_",i+1);
	for (i=0; i<dev->InputAlarmNum; i++)
		sprintf(this->nameInfo.alarmIn[i],"%s%d","alarmIn_",i+1);
	for (i=0; i<dev->OutputAlarmNum; i++)
		sprintf(this->nameInfo.alarmOut[i],"%s%d","alarmOut_",i+1);
}

int nameFileRead(struct DEVHARDPARAM * dev)
{
	FILE *fp = NULL;

	if((fp=fopen("/dvs/Name.bin","r")) == NULL)
	{
		perror("打开名称配置文件 读 失败\n");
		setDefaultName(&nameParam,dev);
	}
	else
	{
		fread(&nameParam,1,sizeof(struct NAMEPARAM),fp);
		fclose(fp);
	}
	return 0;
}

int nameFileGet(struct NAMEPARAM *name)
{
	memcpy(name,&nameParam,sizeof(struct NAMEPARAM));
	return 0;
}

int nameFileSet(struct NAMEPARAM *name)
{
	if (memcmp(name,&nameParam,sizeof(struct NAMEPARAM)))
	{
		memcpy(&nameParam,name,sizeof(struct NAMEPARAM));
		
		FILE *fp = NULL;
		if((fp=fopen("/dvs/Name.bin","w+")) == NULL)
			perror("打开名称配置文件 写 失败\n");
		else
		{
		    printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			fwrite(&nameParam,1,sizeof(struct NAMEPARAM),fp);
			fclose(fp);
		}
		
		if (nameCallBack != 0)
			nameCallBack((void *)&nameParam);
	}
	
	return 0;	
}
