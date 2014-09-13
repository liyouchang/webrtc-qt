#include "store.h"

static struct STORPARAM storeParam;
CONFIG_CALLBACK storeCallBack=0;

static void setDefaultStore(struct STORPARAM *this)
{
	memset(this,0,sizeof(struct STORPARAM));
	this->overwrite = 1;
	this->Enable = 1;
	this->packinterval = 15;
	this->snapeswitch = 1;
	this->alarmswitch = 1;
	this->senseswitch = 1;
	int i,j,l;
	for(i=0; i<16; i++)
		this->StoreCycle[i] = 7;
	for (l=0; l<MAXVIDEOCHNS; l++)
	{
	    for(i=0; i<7; i++)
	    {
	        this->strategy[l][i].begin[0]=0x00;
	        this->strategy[l][i].end[0]=0x1800;
	        for(j=1; j<3; j++)
	        {
	            this->strategy[l][i].begin[j]=0x00;
	            this->strategy[l][i].end[j]=0x00;
	        }
	    }
	}    
}

int storeFileRead(struct DEVHARDPARAM * dev)
{
	FILE *fp = NULL;

	if((fp=fopen("/dvs/Stor.bin","r")) == NULL)
	{
		perror("打开存储配置文件 读 失败\n");
		setDefaultStore(&storeParam);
	}
	else
	{
		fread(&storeParam,1,sizeof(struct STORPARAM),fp);
		fclose(fp);
	}
	return 0;
}

int storeFileGet(struct STORPARAM *store)
{
	memcpy(store,&storeParam,sizeof(struct STORPARAM));
	return 0;
}

int storeFileSet(struct STORPARAM *store)
{
	if (memcmp(store,&storeParam,sizeof(struct STORPARAM)))
	{
		memcpy(&storeParam,store,sizeof(struct STORPARAM));
		
		FILE *fp = NULL;	
		if((fp=fopen("/dvs/Stor.bin","w+")) == NULL)
			perror("打开485参数配置文件 写 失败\n");
		else
		{
			printf("%s:%d 写文件\n",__FUNCTION__,__LINE__);
			fwrite(&storeParam,1,sizeof(struct STORPARAM),fp);
			fclose(fp);
		}		
		
		if (storeCallBack != 0)
			storeCallBack((void *)&storeParam);
	}
	return 0;
}
