#include "osd_title.h"

int osdTitleNum=MAXVIDEOCHNS;
static struct TITLEOSD osdTitleParam[MAXVIDEOCHNS];
CONFIG_CALLBACK osdTitleCallBack=0;

static void setDefaultOsdTitle(struct TITLEOSD *this,struct DEVHARDPARAM * dev)
{
	int i = 0;
	struct TITLEOSD * tmp_title=this;
	for(i=0; i<dev->VideoChanNum; i++)
	{
		memset(tmp_title,0,sizeof(struct TITLEOSD));
		tmp_title->Enable = 1;
		tmp_title->X = 290;
		tmp_title->Y = 240;
		tmp_title->Width = 16;
		tmp_title->Height = 16;
		tmp_title->Trans = 50;
		tmp_title->Layer = 1;
		sprintf((char *)tmp_title->Contert,"channel_%d",i);
		tmp_title->Len = strlen((char *)tmp_title->Contert);
		tmp_title++;
	}
}

int osdTitleFileRead(struct DEVHARDPARAM * dev)
{
	int i = 0;
	FILE *fp = NULL;
	osdTitleNum = dev->VideoChanNum;

	if((fp=fopen("/dvs/OSDTitle.bin","r")) == NULL)
	{
		perror("打开OSD标题配置文件 读 失败\n");
		setDefaultOsdTitle(osdTitleParam,dev);
	}
	else
	{
		for(i=0; i<osdTitleNum; i++)
			fread(&osdTitleParam[i],1,sizeof(struct TITLEOSD),fp);
		fclose(fp);
	}
	return 0;
}

int osdTitleFileGet(struct TITLEOSD *osdTitle)
{
	int i;
	for (i=0; i<osdTitleNum; i++,osdTitle++)
		memcpy(osdTitle,&osdTitleParam[i],sizeof(struct TITLEOSD));
	return 0;
}

int osdTitleFileSet(struct TITLEOSD *osdTitle)
{
	int i;
	int isWrite = 0;
	for (i=0; i<osdTitleNum; i++,osdTitle++)
	{
		if (memcmp(osdTitle,&osdTitleParam[i],sizeof(struct TITLEOSD)))
		{
			memcpy(&osdTitleParam[i],osdTitle,sizeof(struct TITLEOSD));
			isWrite = 1;
		}
	}
	if (isWrite == 1)
	{
		FILE *fp = NULL;
		if((fp=fopen("/dvs/OSDTitle.bin","w+")) == NULL)
			perror("打开OSD标题配置文件 写 失败\n");
		else
		{
			printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
            for(i=0; i<osdTitleNum; i++)
                fwrite(&osdTitleParam[i],1,sizeof(struct TITLEOSD),fp);
			fclose(fp);
		}

		if (osdTitleCallBack != 0)
			osdTitleCallBack((void *)osdTitleParam);
	}
	return 0;
}
