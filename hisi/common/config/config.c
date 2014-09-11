#include "config.h"
#include "../common_api.h"

struct DEVHARDPARAM * pDevParam;
//=============================================================================//

//=============================================================================//
int CONFIG_Initialize(void)
{
	devFileRead(&pDevParam);
	netFileRead(pDevParam);
	mediaFileRead(pDevParam);
	inputFileRead(pDevParam);
	outputFileRead(pDevParam);
	jpegFileRead(pDevParam);
	nameFileRead(pDevParam);
	serialFileRead(pDevParam);
	storeFileRead(pDevParam);
	osdTimeFileRead(pDevParam);
	osdTitleFileRead(pDevParam);
	osdPrivacyFileRead(pDevParam);
	sensorFileRead(pDevParam);
	senseFileRead(pDevParam);
	overlayFileRead(pDevParam);
	singleLostFileRead(pDevParam);
	voFileRead(pDevParam);
	wirlessFileRead(pDevParam);
	
	return CONFIG_SUCESS;
}

int CONFIG_Cleanup(void)
{
	return CONFIG_SUCESS;
}

int CONFIG_Get(e_config_type enAttrId,void * pData)
{
	switch(enAttrId)
	{
		case CONFIG_TYPE_DEV:
			devFileGet((struct DEVHARDPARAM *)pData);
			break;
		case CONFIG_TYPE_NET:
			netFileGet((struct NETPARAM *)pData);
			break;
		case CONFIG_TYPE_MEDIA:
			mediaFileGet((struct MEDIAPARAM *)pData);
			break;
		case CONFIG_TYPE_INPUT:
			inputFileGet((struct ALMINPUT *)pData);
			break;
		case CONFIG_TYPE_OUTPUT:
			outputFileGet((struct ALMOUTPUT *)pData);
			break;
		case CONFIG_TYPE_JPEG:
			jpegFileGet((struct JPEGPARAM *)pData);
			break;
		case CONFIG_TYPE_NAME:
			nameFileGet((struct NAMEPARAM *)pData);
			break;
		case CONFIG_TYPE_SERIAL:
			serialFileGet((struct SERIAL *)pData);
			break;
		case CONFIG_TYPE_STORE:
			storeFileGet((struct STORPARAM *)pData);
			break;
		case CONFIG_TYPE_OSDTIME:
			osdTimeFileGet((struct TIMEOSD *)pData);
			break;
		case CONFIG_TYPE_OSDTITLE:
			osdTitleFileGet((struct TITLEOSD *)pData);
			break;
		case CONFIG_TYPE_OSDPRIVACY:
			osdPrivacyFileGet((struct PRITOSD *)pData);
			break;
		case CONFIG_TYPE_SENSOR:
			sensorFileGet((struct SENSORPARAM *)pData);
			break;
		case CONFIG_TYPE_SENSE:
			senseFileGet((struct SENSE *)pData);
			break;
		case CONFIG_TYPE_OVERLAY:
			overlayFileGet((struct OVALM *)pData);
			break;
		case CONFIG_TYPE_LOSTSINGLE:
			singleLostFileGet((struct SINGLELOST *)pData);
			break;
		case CONFIG_TYPE_VO:
			voFileGet((struct VOPARAM *)pData);
			break;
		case CONFIG_TYPE_WIRLESS_OLD:
			wirlessFileGet((char *)pData);
			break;
		default:
			break;
	}
	return CONFIG_SUCESS;
}

int CONFIG_Set(e_config_type enAttrId,void * pData)
{
	switch(enAttrId)
	{
		case CONFIG_TYPE_NET:
			netFileSet((struct NETPARAM *)pData);
			break;
		case CONFIG_TYPE_MEDIA:
			mediaFileSet((struct MEDIAPARAM *)pData);
			break;
		case CONFIG_TYPE_INPUT:
			inputFileSet((struct ALMINPUT *)pData);
			break;
		case CONFIG_TYPE_OUTPUT:
			outputFileSet((struct ALMOUTPUT *)pData);
			break;
		case CONFIG_TYPE_JPEG:
			jpegFileSet((struct JPEGPARAM *)pData);
			break;
		case CONFIG_TYPE_NAME:
			nameFileSet((struct NAMEPARAM *)pData);
			break;
		case CONFIG_TYPE_SERIAL:
			serialFileSet((struct SERIAL *)pData);
			break;
		case CONFIG_TYPE_STORE:
			storeFileSet((struct STORPARAM *)pData);
			break;
		case CONFIG_TYPE_OSDTIME:
			osdTimeFileSet((struct TIMEOSD *)pData);
			break;
		case CONFIG_TYPE_OSDTITLE:
			osdTitleFileSet((struct TITLEOSD *)pData);
			break;
		case CONFIG_TYPE_OSDPRIVACY:
			osdPrivacyFileSet((struct PRITOSD *)pData);
			break;
		case CONFIG_TYPE_SENSOR:
			sensorFileSet((struct SENSORPARAM *)pData);
			break;
		case CONFIG_TYPE_SENSE:
			senseFileSet((struct SENSE *)pData);
			break;
		case CONFIG_TYPE_OVERLAY:
			overlayFileSet((struct OVALM *)pData);
			break;
		case CONFIG_TYPE_LOSTSINGLE:
			singleLostFileSet((struct SINGLELOST *)pData);
			break;
		case CONFIG_TYPE_VO:
			voFileSet((struct VOPARAM *)pData);
			break;
		case CONFIG_TYPE_WIRLESS_OLD:
			wirlessFileSet((char *)pData);
			break;
		default:
			break;
	}
	return CONFIG_SUCESS;
}

int CONFIG_Register_Callback(e_config_type enAttrId,CONFIG_CALLBACK config_callback)
{
	switch(enAttrId)
	{
		case CONFIG_TYPE_NET:
			netCallBack = config_callback;
			break;
		case CONFIG_TYPE_MEDIA:
			mediaCallBack = config_callback;
			break;
		case CONFIG_TYPE_INPUT:
			inputCallBack = config_callback;
			break;
		case CONFIG_TYPE_OUTPUT:
			outputCallBack = config_callback;
			break;
		case CONFIG_TYPE_JPEG:
			jpegCallBack = config_callback;
			break;
		case CONFIG_TYPE_NAME:
			nameCallBack = config_callback;
			break;
		case CONFIG_TYPE_SERIAL:
			serialCallBack = config_callback;
			break;
		case CONFIG_TYPE_STORE:
			storeCallBack = config_callback;
			break;
		case CONFIG_TYPE_OSDTIME:
			osdTimeCallBack = config_callback;
			break;
		case CONFIG_TYPE_OSDTITLE:
			osdTitleCallBack = config_callback;
			break;
		case CONFIG_TYPE_OSDPRIVACY:
			osdPrivacyCallBack = config_callback;
			break;
		case CONFIG_TYPE_SENSOR:
			sensorCallBack = config_callback;
			break;
		case CONFIG_TYPE_SENSE:
			if (senseCallBack[0] == 0)
				senseCallBack[0] = config_callback;
			else
				senseCallBack[1] = config_callback;
			break;
		case CONFIG_TYPE_OVERLAY:
			if (overlayCallBack[0] == 0)
				overlayCallBack[0] = config_callback;
			else
				overlayCallBack[1] = config_callback;
			break;
		case CONFIG_TYPE_LOSTSINGLE:
			singleLostCallBack = config_callback;
			break;
		case CONFIG_TYPE_VO:
			voCallBack = config_callback;
			break;
		case CONFIG_TYPE_WIRLESS_OLD:
			wirlessCallBack = config_callback;
			break;
		default:
			break;
	}
	return CONFIG_SUCESS;
}
