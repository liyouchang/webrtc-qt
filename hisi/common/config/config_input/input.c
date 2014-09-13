#include "input.h"

int inputNum=MAX_ALARMINPUTS;
struct ALMINPUT inputParam[MAX_ALARMINPUTS];
CONFIG_CALLBACK inputCallBack=0;

static void setDefaultAlmIn(struct ALMINPUT *this,struct DEVHARDPARAM * dev)
{
    int i = 0;
	for(i=0; i<dev->InputAlarmNum; i++)
	{
		memset(&this[i], 0, sizeof(struct ALMINPUT));
		(&this[i])->CallCenter = 1;
		(&this[i])->TypeOutBurst = 1;
		(&this[i])->JpegSheet = 1;
		(&this[i])->JpegIntv = 25;
		(&this[i])->Enable = 0;
		(&this[i])->PreRecord = 5;
		(&this[i])->RecDelay = 2;
	}
}

int inputFileRead(struct DEVHARDPARAM * dev)
{
	int i = 0;
	FILE *fp = NULL;
	inputNum = dev->InputAlarmNum;

	if((fp=fopen("/dvs/AlmIn.bin","r")) == NULL)
	{
		perror("打开报警输入配置文件 读 失败\n");
		setDefaultAlmIn(inputParam,dev);
	}
	else
	{
		for(i=0; i<dev->InputAlarmNum; i++)
			fread(&inputParam[i],1,sizeof(struct ALMINPUT),fp);
		fclose(fp);
	}
	return 0;
}

int inputFileGet(struct ALMINPUT *input)
{
	int i;
	for (i=0; i<inputNum; i++,input++)
		memcpy(input,&inputParam[i],sizeof(struct ALMINPUT));
		
	return 0;
}

int inputFileSet(struct ALMINPUT *input)
{
	int i;
	int isWrite = 0;
	for (i=0; i<inputNum; i++,input++)
	{
		if (memcmp(input,&inputParam[i],sizeof(struct ALMINPUT)))
		{	
			memcpy(&inputParam[i],input,sizeof(struct ALMINPUT));
			isWrite = 1;
		}
	}
	if (isWrite == 1)
	{			
		FILE *fp = NULL;
		if ((fp=fopen("/dvs/AlmIn.bin","w+")) == NULL)
			perror("打开网络参数配置文件 写 失败\n");
		else
		{
		    printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
		    fwrite((char *)&inputParam,1,sizeof(struct ALMINPUT)*inputNum,fp);
			fclose(fp);
		}
		
		if (inputCallBack != 0)
			inputCallBack((void *)inputParam);
	}
	return 0;
}
