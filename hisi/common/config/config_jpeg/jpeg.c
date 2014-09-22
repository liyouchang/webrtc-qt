#include "jpeg.h"

int jpegNum=MAXVIDEOCHNS;
static struct JPEGPARAM jpegParam[MAXVIDEOCHNS];
CONFIG_CALLBACK jpegCallBack=0;

static void setDefaultJpeg(struct JPEGPARAM *this)
{
	int i;
	for(i=0; i<jpegNum; i++)
	{
		memset(&this[i],0,sizeof(struct JPEGPARAM));
		(&this[i])->resolution = 2;
		(&this[i])->picLevel = 1;
	}
}

int jpegFileRead(struct DEVHARDPARAM * dev)
{
	int i = 0;
	FILE *fp = NULL;
	jpegNum = dev->VideoChanNum;
	
	if((fp=fopen("/dvs/jpge.bin","r")) == NULL)
	{
		perror("打开抓拍图片配置文件 读 失败\n");
		setDefaultJpeg(jpegParam);
	}
	else
	{
		for(i=0; i<jpegNum; i++)
			fread(&jpegParam[i],1,sizeof(struct JPEGPARAM),fp);
		fclose(fp);
	}
	
	return 0;
}

int jpegFileGet(struct JPEGPARAM *jpeg)
{
	int i;
	for (i=0; i<jpegNum; i++,jpeg++)
		memcpy(jpeg,&jpegParam[i],sizeof(struct JPEGPARAM));
	
	return 0;
}

int jpegFileSet(struct JPEGPARAM *jpeg)
{
	int i;
	int isWrite = 0;
	for (i=0; i<jpegNum; i++,jpeg++)
	{
		if (memcmp(jpeg,&jpegParam[i],sizeof(struct JPEGPARAM)))
		{	
			memcpy(&jpegParam[i],jpeg,sizeof(struct JPEGPARAM));
			isWrite = 1;
		}
	}
	
	if (isWrite == 1)
	{
		FILE *fp = NULL;
		if((fp=fopen("/dvs/jpge.bin","w+")) == NULL)
			perror("打开抓拍图片配置文件 写 失败\n");
		else
		{
		    printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			for (i=0; i<jpegNum; i++,jpeg++)
				fwrite(&jpegParam[i],1,sizeof(struct JPEGPARAM),fp);
			fclose(fp);
		}
		
		if (jpegCallBack != 0)
			jpegCallBack((void *)jpegParam);
	}
	return 0;
}
