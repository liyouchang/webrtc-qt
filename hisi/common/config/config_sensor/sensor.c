#include "sensor.h"

int sensorNum=MAXVIDEOCHNS;
static struct SENSORPARAM sensorParam[MAXVIDEOCHNS];
CONFIG_CALLBACK sensorCallBack=0;

static void setDefaultSensor(struct SENSORPARAM *this,struct DEVHARDPARAM * dev)
{
	int i = 0;
	struct SENSORPARAM * sensor = this;
	for(i=0; i<dev->VideoChanNum; i++)
	{
	    memset(sensor,0,sizeof(struct SENSORPARAM));
	    sensor->resolution = 7;
	    sensor->whiteBalance = 0;
	    sensor->redGain = 3;
	    sensor->blueGain = 3;
	    sensor->focusAuto = 0;
	    sensor->focusChange = 3;
	    sensor->zoomChange = 3;
	    sensor->exposure = 0;
	    sensor->shutterSpeed = 3;
	    sensor->IRISGain = 3;
	    sensor->exposureGain = 3;
	    sensor->exposureComp = 1;
	    sensor->exposureCompChange = 3;
	    sensor->turn_h = 0;
	    sensor->turn_v = 0;
	    sensor->dayMode = 1;
	    sensor->dayOrNight = 3;
	    sensor->apertureChange = 3;
	    sensor->cameraMode = 3;
	    sensor->cameraNumber = 0xff;
	    sensor->zoomSpeed = 0xff;

	    sensor->luma = 128;
        sensor->contrast = 192;
        sensor->saturation = 128;
        sensor->chroma = 128;
        sensor->sharpen = 128;
        sensor->resetValue = 0;
        sensor->drcEnable = 0;
        sensor->drcStrength = 128;
        sensor->denoise2D = 0x15;
        sensor->dn3DEnable = 1;
        sensor->sfStrength = 32;
        sensor->tfStrength = 8;
        sensor->antifogEnable = 0;
        sensor->antifogStrength = 128;
        sensor->rGain = 373;
        sensor->gGain = 266;
        sensor->bGain = 409;
        sensor->exposureTime = 2;
        sensor->zoomPosition = -1;
        sensor++;
	}
}

int sensorFileRead(struct DEVHARDPARAM * dev)
{
	int i = 0;
	sensorNum = dev->VideoChanNum;
	
	FILE *fp = NULL;	
	if((fp=fopen("/dvs/Sensor.bin","r")) == NULL)
	{
		perror("打开模组参数配置文件 读 失败\n");
		setDefaultSensor(sensorParam,dev);
	}
	else
	{
		for(i=0; i<sensorNum; i++)
			fread(&sensorParam[i],1,sizeof(struct SENSORPARAM),fp);
		fclose(fp);
	}
	return 0;
}

int sensorFileGet(struct SENSORPARAM *sensor)
{
	int i;
	for (i=0; i<sensorNum; i++,sensor++)
		memcpy(sensor,&sensorParam[i],sizeof(struct SENSORPARAM));
	
	return 0;
}

int sensor_adjust(struct SENSORPARAM *sensor)									//sensor参数调整
{
    if (sensor->resetValue == 1)
    {
        sensor->resolution = 7;
	    sensor->whiteBalance = 0;
	    sensor->redGain = 3;
	    sensor->blueGain = 3;
	    sensor->focusAuto = 0;
	    sensor->focusChange = 3;
	    sensor->zoomChange = 3;
	    sensor->exposure = 0;
	    sensor->shutterSpeed = 3;
	    sensor->IRISGain = 3;
	    sensor->exposureGain = 3;
	    sensor->exposureComp = 1;
	    sensor->exposureCompChange = 3;
//	    sensor->turn_h = 0;
//	    sensor->turn_v = 0;
//	    sensor->dayMode = 1;
//	    sensor->dayOrNight = 3;
	    sensor->apertureChange = 3;
	    sensor->cameraMode = 3;
	    sensor->cameraNumber = 0xff;
	    sensor->zoomSpeed = 0xff;
        sensor->luma = EXPOSURE_COMP;
        sensor->contrast = 192;
        sensor->saturation = 128;//100;//
        sensor->chroma = 128;
        sensor->sharpen = 128;
        sensor->resetValue = 0;
        sensor->drcEnable = 0;
        sensor->drcStrength = 128;
        sensor->denoise2D = 0x15;
        sensor->dn3DEnable = 1;
        sensor->sfStrength = 32;
        sensor->tfStrength = 8;
        sensor->antifogEnable = 0;
        sensor->antifogStrength = 128;
        sensor->rGain = 373;
        sensor->gGain = 266;
        sensor->bGain = 409;
        sensor->exposureTime = 2;
        sensor->zoomPosition = -1;
    }
    else
    {
        if (sensor->redGain == 0)
            sensor->rGain = 373;
        else if (sensor->redGain == 1)
        {
            if (sensor->rGain != 0)
                sensor->rGain--;
        }
        else if (sensor->redGain == 2)
        {
            if (sensor->rGain != 0xff)
                sensor->rGain++;
        }
        sensor->redGain = 3;

        if (sensor->blueGain == 0)
            sensor->bGain = 409;
        else if (sensor->blueGain == 1)
        {
            if (sensor->bGain != 0)
                sensor->bGain--;
        }
        else if (sensor->blueGain == 2)
        {
            if (sensor->bGain != 0xff)
                sensor->bGain++;
        }
        sensor->blueGain = 3;

        if (sensor->shutterSpeed == 0)
            sensor->exposureTime = 2;
        else if (sensor->shutterSpeed == 1)
        {
            if (sensor->exposureTime != 0)
                sensor->exposureTime--;
        }
        else if (sensor->shutterSpeed == 2)
        {
            if (sensor->exposureTime != 0xff)
                sensor->exposureTime++;
        }
        sensor->shutterSpeed = 3;

        if (sensor->exposureCompChange == 0)
            sensor->luma = EXPOSURE_COMP;
        else if (sensor->exposureCompChange == 1)
        {
            if (sensor->luma != 0)
                sensor->luma--;
        }
        else if (sensor->exposureCompChange == 2)
        {
            if (sensor->luma != 0xff)
                sensor->luma++;
        }
        sensor->exposureCompChange = 3;

        if (sensor->apertureChange == 0)
            sensor->sharpen = 250;
        else if (sensor->apertureChange == 1)
        {
            if (sensor->sharpen != 0)
                sensor->sharpen--;
        }
        else if (sensor->apertureChange == 2)
        {
            if (sensor->sharpen != 0xff)
                sensor->sharpen++;
        }
        sensor->apertureChange = 3;
    }

    return 0;
}

int sensorFileSet(struct SENSORPARAM *sensor)
{
	int i;
	int isWrite = 0;
	
	sensor_adjust(sensor);
	for (i=0; i<sensorNum; i++,sensor++)
	{
		if (memcmp(sensor,&sensorParam[i],sizeof(struct SENSORPARAM)))
		{	
			memcpy(&sensorParam[i],sensor,sizeof(struct SENSORPARAM));
			isWrite = 1;
		}
	}
	
	if (isWrite == 1)
	{
		FILE *fp = NULL;
		if((fp=fopen("/dvs/Sensor.bin","w+")) == NULL)
			perror("打开模组参数配置文件 写 失败\n");
		else
		{
			printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			for(i=0; i<sensorNum; i++)
				fwrite(&sensorParam[i],1,sizeof(struct SENSORPARAM),fp);
			fclose(fp);
		}
		
		if (sensorCallBack != 0)
			sensorCallBack((void *)sensorParam);		
	}
	return 0;
}
