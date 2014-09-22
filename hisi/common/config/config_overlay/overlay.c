#include "overlay.h"

int overlayNum=MAXVIDEOCHNS;
static struct OVALM overlayParam[MAXVIDEOCHNS];
CONFIG_CALLBACK overlayCallBack[2]={0};

static void setDefaultOverlay(struct OVALM *this,struct DEVHARDPARAM * dev)
{
	int i = 0;
	for(i=0; i<dev->VideoChanNum; i++)
		memset(&this[i],0,sizeof(struct OVALM));
}

int overlayFileRead(struct DEVHARDPARAM * dev)
{
	overlayNum = dev->VideoChanNum;

	FILE *fp = NULL;
	if((fp=fopen("/dvs/Overlay.bin","r")) == NULL)
	{
		perror("打开遮挡报警配置文件 读 失败\n");
		setDefaultOverlay(overlayParam,dev);
	}
	else
	{
		int i;
		for(i=0; i<overlayNum; i++)
			fread(&overlayParam[i],1,sizeof(struct OVALM),fp);
		fclose(fp);
	}
	return 0;
}

int overlayFileGet(struct OVALM *overlay)
{
	int i;
	for (i=0; i<overlayNum; i++,overlay++)
		memcpy(overlay,&overlayParam[i],sizeof(struct OVALM));
	return 0;
}

int overlayFileSet(struct OVALM *overlay)
{
	int i;
	int isWrite = 0;
	for (i=0; i<overlayNum; i++,overlay++)
	{
		if (memcmp(overlay,&overlayParam[i],sizeof(struct OVALM)))
		{	
			memcpy(&overlayParam[i],overlay,sizeof(struct OVALM));
			isWrite = 1;
		}
	}
	
	if (isWrite == 1)
	{
		FILE *fp = NULL;
		if((fp=fopen("/dvs/Overlay.bin","w+")) == NULL)
			perror("打开遮挡报警配置文件 写 失败\n");
		else
		{
			printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			for(i=0; i<overlayNum; i++)
				fwrite(&overlayParam[i],1,sizeof(struct OVALM),fp);
			fclose(fp);
		}
		
		if (overlayCallBack[0] != 0)
			overlayCallBack[0]((void *)overlayParam);
		if (overlayCallBack[1] != 0)
			overlayCallBack[1]((void *)overlayParam);
	}
	return 0;
}
