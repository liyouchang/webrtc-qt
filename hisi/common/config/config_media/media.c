#include "media.h"

struct MEDIAPARAM mediaParam;
CONFIG_CALLBACK mediaCallBack=0;

static void setDefaultMedia(struct MEDIAPARAM *this,struct DEVHARDPARAM * dev)
{
    memset(this,0,sizeof(struct MEDIAPARAM));
    this->aFormat = 0;
    this->vFormat = 0;

	int i = 0;
	for(i=0; i<dev->VideoChanNum; i++)
	{
		this->main[i].enable = 1;
		this->main[i].resolution = 7;
		this->main[i].frame_rate = 15;
		this->main[i].idr_interval = 30;
		this->main[i].rate_ctrl_mode = 0;
		this->main[i].bitrate = 512;
		this->main[i].piclevel = 0;
		this->main[i].qp_i = 24;
		this->main[i].qp_p = 32;
		this->main[i].rate_statistic = 1;
		this->main[i].wm_enable = 0;
		this->minor[i].enable = 0;
		this->minor[i].resolution = 4;
		this->minor[i].frame_rate = 15;
		this->minor[i].idr_interval = 30;
		this->minor[i].rate_ctrl_mode = 0;
		this->minor[i].bitrate = 384;
		this->minor[i].piclevel = 0;
		this->minor[i].qp_i = 24;
		this->minor[i].qp_p = 32;
		this->minor[i].rate_statistic = 1;
		this->minor[i].wm_enable = 0;
		printf("setDefaultMedia %d %d\n",this->main[i].resolution,this->minor[i].resolution);
	}
}

int mediaFileRead(struct DEVHARDPARAM * dev)
{
	FILE *fp = NULL;

	if((fp=fopen("/dvs/media.bin","r")) == NULL)
	{
		perror("打开网络参数配置文件 读 失败\n");
		setDefaultMedia(&mediaParam,dev);
	}
	else
	{
		fread(&mediaParam,1,sizeof(struct MEDIAPARAM),fp);
		fclose(fp);
	}
	int i;
	for (i=0; i<dev->VideoChanNum; i++)
	{
	    if (mediaParam.minor[i].enable == 0)
	    {
	        if (dev->VideoAdType < 10)
                mediaParam.minor[i].resolution = 2;
            else
                mediaParam.minor[i].resolution = 6;
            mediaParam.minor[i].frame_rate = 25;
            mediaParam.minor[i].idr_interval = 75;
            mediaParam.minor[i].rate_ctrl_mode = 0;
            mediaParam.minor[i].bitrate = 1024;
            mediaParam.minor[i].piclevel = 1;
            mediaParam.minor[i].qp_i = 32;
            mediaParam.minor[i].qp_p = 40;
	    }
	    if (dev->VideoAdType < 10)
	    {
	        if (mediaParam.main[i].resolution==0)
                mediaParam.main[i].resolution = 2;
            if (mediaParam.minor[i].resolution==0)
                mediaParam.minor[i].resolution = 2;
	    }
	    else if (dev->VideoAdType < 20)
	    {
            if (mediaParam.main[i].resolution==0)
                mediaParam.main[i].resolution = 7;
            if (mediaParam.minor[i].resolution==0)
                mediaParam.minor[i].resolution = 6;
	    }
	    else if (dev->VideoAdType < 30)
	    {
            if (mediaParam.main[i].resolution==0)
                mediaParam.main[i].resolution = 8;
            if (mediaParam.minor[i].resolution==0)
                mediaParam.minor[i].resolution = 6;
	    }
	    if (!strcmp(dev->OEM_Type,"CAM21D-040"))
	    {
		    if (mediaParam.main[i].resolution > 6)
		    	mediaParam.main[i].resolution = 4;
		    if (mediaParam.minor[i].resolution > 6)
		    	mediaParam.minor[i].resolution = 4;
		}
		if (mediaParam.main[i].piclevel == 0)
        {
        	mediaParam.main[i].qp_i = 24;
        	mediaParam.main[i].qp_p = 32;
        }
        else
        {
            mediaParam.main[i].qp_i = 28+mediaParam.main[i].piclevel*4;
			mediaParam.main[i].qp_p = 36+mediaParam.main[i].piclevel*4;
		}
		if (mediaParam.main[i].qp_p > 50)
			mediaParam.main[i].qp_p = 50;
		if (mediaParam.main[i].qp_i > mediaParam.main[i].qp_p)
			mediaParam.main[i].qp_i = mediaParam.main[i].qp_p-2;
			
        if (mediaParam.minor[i].piclevel == 0)
        {
        	mediaParam.minor[i].qp_i = 24;
        	mediaParam.minor[i].qp_p = 32;
        }
        else
        {
            mediaParam.minor[i].qp_i = 28+mediaParam.minor[i].piclevel*4;
			mediaParam.minor[i].qp_p = 36+mediaParam.minor[i].piclevel*4;
		}
		if (mediaParam.minor[i].qp_p > 50)
			mediaParam.minor[i].qp_p = 50;
		if (mediaParam.minor[i].qp_i > mediaParam.minor[i].qp_p)
			mediaParam.minor[i].qp_i = mediaParam.minor[i].qp_p-2;
	}
	return 0;
}

int mediaFileGet(struct MEDIAPARAM *media)
{
	memcpy(media,&mediaParam,sizeof(struct MEDIAPARAM));
	return 0;
}

int mediaFileSet(struct MEDIAPARAM *media)
{
	if (memcmp(media,&mediaParam,sizeof(struct MEDIAPARAM)))
	{
		memcpy(&mediaParam,media,sizeof(struct MEDIAPARAM));
		
		FILE *fp = NULL;
		if ((fp=fopen("/dvs/media.bin","w+")) == NULL)
			perror("打开网络参数配置文件 写 失败\n");
		else
		{
		    printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
		    fwrite((char *)&mediaParam,1,sizeof(struct MEDIAPARAM),fp);
			fclose(fp);
		}
		
		if (mediaCallBack != 0)
			mediaCallBack((void *)&mediaParam);
	}
	return 0;
}
