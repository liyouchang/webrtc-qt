#include "sense.h"

int senseNum=MAXVIDEOCHNS;
static struct SENSE senseParam[MAXVIDEOCHNS][4];
CONFIG_CALLBACK senseCallBack[2]={0};

static void setDefaultSense(struct SENSE *this,struct DEVHARDPARAM * dev)
{
    struct SENSE * psense=this;
	int i = 0;
	int j = 0;
	for(i=0; i<dev->VideoChanNum; i++)
	{
	    for (j=0; j<4; j++)
        {
            memset(psense,0,sizeof(struct SENSE));
            psense++;
        }
    }
}

int senseFileRead(struct DEVHARDPARAM * dev)
{
	senseNum = dev->VideoChanNum;
	FILE *fp = NULL;
	if((fp=fopen("/dvs/Sense.bin","r")) == NULL)
	{
		perror("打开移动侦测配置文件 读 失败\n");
		setDefaultSense((struct SENSE *)senseParam,dev);
	}
	else
	{
		int i,j;
		for(i=0; i<dev->VideoChanNum; i++)
		{
		    for (j=0; j<4; j++)
                fread(&senseParam[i][j],1,sizeof(struct SENSE),fp);
		}
		fclose(fp);
	}
	return 0;
}

int senseFileGet(struct SENSE *sense)
{
	int i = 0;
	int j = 0;
	for (i=0; i<senseNum; i++)
	{	
		for (j=0; j<4; j++,sense++)
			memcpy(sense,&senseParam[i][j],sizeof(struct SENSE));
	}
	return 0;
}

int senseFileSet(struct SENSE *sense)
{
	int i;
	int j;
	int isWrite = 0;
	for (i=0; i<senseNum; i++)
	{
		for (j=0; j<4; j++,sense++)
		{
			if (memcmp(sense,&senseParam[i][j],sizeof(struct SENSE)))
			{	
				memcpy(&senseParam[i][j],sense,sizeof(struct SENSE));
				isWrite = 1;
			}
		}
	}
	
	if (isWrite == 1)
	{
		FILE *fp = NULL;	
		if((fp=fopen("/dvs/Sense.bin","w+")) == NULL)
			perror("打开移动侦测配置文件 写 失败\n");
		else
		{
			printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			for(i=0; i<senseNum; i++)
			{
			    for (j=0; j<4; j++)
	                fwrite(&senseParam[i][j],1,sizeof(struct SENSE),fp);
			}
			fclose(fp);
		}
		
		if (senseCallBack[0] != 0)
			senseCallBack[0]((void *)senseParam);
		if (senseCallBack[1] != 0)
			senseCallBack[1]((void *)senseParam);
	}
	return 0;
}
