#include "vo.h"

struct VOPARAM	voParam;
CONFIG_CALLBACK voCallBack=0;

static void setDefaultVO(struct VOPARAM *this)
{
	memset(this,0,sizeof(struct VOPARAM));
}

int voFileRead(struct DEVHARDPARAM * dev)
{
	FILE *fp = NULL;
	if((fp=fopen("/dvs/Vo.bin","r")) == NULL)
	{
		perror("打开视频输出配置文件 读 失败\n");
		setDefaultVO(&voParam);
	}
	else
	{
		fread(&voParam,1,sizeof(struct VOPARAM),fp);
		fclose(fp);
	}
	return 0;
}

int voFileGet(struct VOPARAM *vo)
{
	memcpy(vo,&voParam,sizeof(struct VOPARAM));
	return 0;
}

int voFileSet(struct VOPARAM *vo)
{
	if (memcmp(vo,&voParam,sizeof(struct VOPARAM)))
	{
		memcpy(&voParam,vo,sizeof(struct VOPARAM));
		
		FILE *fp = NULL;
		if((fp=fopen("/dvs/Vo.bin","w+")) == NULL)
			perror("打开视频输出配置文件 写 失败\n");
		else
		{
			printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			fwrite(&voParam,1,sizeof(struct VOPARAM),fp);
			fclose(fp);
		}
		
		if (voCallBack != 0)
			voCallBack((void *)&voParam);
	}
	return 0;
}
