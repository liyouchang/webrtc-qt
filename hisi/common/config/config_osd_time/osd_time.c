#include "osd_time.h"

int osdTimeNum=MAXVIDEOCHNS;
static struct TIMEOSD osdTimeParam[MAXVIDEOCHNS];
CONFIG_CALLBACK osdTimeCallBack=0;

static void setDefaultosdTime(struct TIMEOSD *this,struct DEVHARDPARAM * dev)
{
	struct TIMEOSD *osdTime=this;
	int i = 0;
	for(i=0; i<dev->VideoChanNum; i++)
	{
		memset(osdTime, 0, sizeof(struct TIMEOSD));
		osdTime->X = 10;
		osdTime->Y = 10;
		osdTime->Switch = 1;
		osdTime->Trans = 50;
		osdTime->Layer = 0;
		osdTime->Format = 1;
		osdTime->Reso = 2;
		osdTime++;
	}
}

int osdTimeFileRead(struct DEVHARDPARAM * dev)
{
	int i = 0;
	FILE *fp = NULL;
	osdTimeNum = dev->VideoChanNum;
	
	if((fp=fopen("/dvs/OSDTime.bin","r")) == NULL)
	{
		perror("打开OSD时间配置文件 读 失败\n");
		setDefaultosdTime(osdTimeParam,dev);
	}
	else
	{
		for(i=0; i<dev->VideoChanNum; i++)
			fread(&osdTimeParam[i],1,sizeof(struct TIMEOSD),fp);
		fclose(fp);
	}
	return 0;
}

int osdTimeFileGet(struct TIMEOSD *osdTime)
{
	int i;
	for (i=0; i<osdTimeNum; i++,osdTime++)
		memcpy(osdTime,&osdTimeParam[i],sizeof(struct TIMEOSD));
		
	return 0;
}

int osdTimeFileSet(struct TIMEOSD *osdTime)
{
	int i;
	int isWrite = 0;
	for (i=0; i<osdTimeNum; i++,osdTime++)
	{
		if (memcmp(osdTime,&osdTimeParam[i],sizeof(struct TIMEOSD)))
		{	
			memcpy(&osdTimeParam[i],osdTime,sizeof(struct TIMEOSD));
			isWrite = 1;
		}
	}
	if (isWrite == 1)
	{			
		FILE *fp = NULL;
		if((fp=fopen("/dvs/OSDTime.bin","w+")) == NULL)
			perror("打开OSD时间配置文件 写 失败\n");
		else
		{
			printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			fwrite((char *)osdTimeParam,1,sizeof(struct TIMEOSD)*osdTimeNum,fp);
			fclose(fp);
		}
		
		if (osdTimeCallBack != 0)
			osdTimeCallBack((void *)osdTimeParam);
	}
	return 0;
}
