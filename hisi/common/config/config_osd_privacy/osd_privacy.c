#include "osd_privacy.h"

int osdPrivacyNum=MAXVIDEOCHNS;
static struct PRITOSD osdPrivacyParam[MAXVIDEOCHNS][4];
CONFIG_CALLBACK osdPrivacyCallBack=0;

static void setDefaultOsdPrivacy(struct PRITOSD *this,struct DEVHARDPARAM * dev)
{
    struct PRITOSD * privacy=this;
	int i = 0;
	int j = 0;
	for(i=0; i<dev->VideoChanNum; i++)
	{
	    for (j=0; j<4; j++)
        {
            memset(privacy,0,sizeof(struct PRITOSD));
            privacy++;
        }
    }
}

int osdPrivacyFileRead(struct DEVHARDPARAM * dev)
{
	int i = 0;
	int j = 0;
	osdPrivacyNum = dev->VideoChanNum;
	struct PRITOSD * privacy = (struct PRITOSD *)osdPrivacyParam;
	
	FILE *fp = NULL;
	if((fp=fopen("/dvs/OSDPrivacy.bin","r")) == NULL)
	{
		perror("打开OSD隐私保护配置文件 读 失败\n");
		setDefaultOsdPrivacy(privacy,dev);
	}
	else
	{
		for(i=0; i<osdPrivacyNum; i++)
		{
		    for (j=0; j<4; j++,privacy++)
                fread(privacy,1,sizeof(struct PRITOSD),fp);
		}
		fclose(fp);
	}
	return 0;
}

int osdPrivacyFileGet(struct PRITOSD *osdosdPrivacy)
{
	int i = 0;
	int j = 0;
	for (i=0; i<osdPrivacyNum; i++)
	{	
		for (j=0; j<4; j++,osdosdPrivacy++)
			memcpy(osdosdPrivacy,&osdPrivacyParam[i][j],sizeof(struct PRITOSD));
	}
	return 0;
}

int osdPrivacyFileSet(struct PRITOSD *osdosdPrivacy)
{
	int i;
	int j;
	int isWrite = 0;
	for (i=0; i<osdPrivacyNum; i++)
	{
		for (j=0; j<4; j++,osdosdPrivacy++)
		{
			if (memcmp(osdosdPrivacy,&osdPrivacyParam[i][j],sizeof(struct PRITOSD)))
			{	
				memcpy(&osdPrivacyParam[i][j],osdosdPrivacy,sizeof(struct PRITOSD));
				isWrite = 1;
			}
		}
	}
	
	if (isWrite == 1)
	{
		FILE *fp = NULL;
		if((fp=fopen("/dvs/OSDPrivacy.bin","w+")) == NULL)
			perror("打开OSD隐私保护配置文件 写 失败\n");
		else
		{
		    printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			for(i=0; i<osdPrivacyNum; i++)
			{
			    for (j=0; j<4; j++)
	                fwrite(&osdPrivacyParam[i][j],1,sizeof(struct PRITOSD),fp);
			}
			fclose(fp);
		}
		
		if (osdPrivacyCallBack != 0)
			osdPrivacyCallBack((void *)osdPrivacyParam);
	}
	return 0;
}
