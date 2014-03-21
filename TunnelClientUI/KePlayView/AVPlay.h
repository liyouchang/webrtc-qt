

#ifdef KAERPLAY_EXPORTS
#define AVPLAY_API __declspec(dllexport)
#else
#define AVPLAY_API __declspec(dllimport)
#endif

#define WM_FILEEND			(WM_USER + 112)		//文件播放完毕消息
#define Play_Buffer_SIZE	0x300000

#pragma   pack(1) 
//查找设备的数据结构（计数器）
typedef struct tagFINDDEVICEDATA
{
	int iCount;
} FINDDEVICEDATA, *PFINDDEVICEDATA;

typedef struct tagDRAWFRAME
{
	int ileft;
	int itop;
	int ibottom;
	int iright;
}DRAWFRAM;

typedef struct tagDRAWFRAMEGROUP
{
	DRAWFRAM drawframe[4];
}DRAWFRAMGROUP;

#pragma

//13.视频数据解码后回调
typedef void (WINAPI *PFVideoData)(int iTerminalId, int iChlId, long hHandle, const char* pDataBuf, int iLen, BOOL bIdr, int iTimeStamp, LPVOID pContext);
//14.音频数据解码后回调
typedef void (WINAPI *PFAudioData)(int iTerminalId, int iChlId, long hHandle, const char* pDataBuf, int iLen, int iTimeStamp, LPVOID pContext);


extern "C" AVPLAY_API HRESULT AV_SetDllName(char *pDllName);

extern "C" AVPLAY_API HRESULT AV_Initial(HWND hWnd);

extern "C" AVPLAY_API HRESULT AV_OpenStream_Ex(long hHandle);

extern "C" AVPLAY_API HRESULT AV_CloseStream_Ex(long hHandle);

extern "C" AVPLAY_API HRESULT AV_OpenFile_Ex(long hHandle, char *sFileName,DWORD dwFileSize = 0x00);

extern "C" AVPLAY_API HRESULT AV_CloseFile_Ex(long hHandle);

extern "C" AVPLAY_API HRESULT AV_SetGPSCallback(long lFn,long lUser);


extern "C" AVPLAY_API HRESULT AV_GetPort();

extern "C" AVPLAY_API HRESULT AV_Play(long hHandle, HWND hWnd);

extern "C" AVPLAY_API HRESULT AV_OpenFile(long *phHandle, LPCTSTR sFileName , DWORD dwFileSize = 0x00 );

extern "C" AVPLAY_API HRESULT AV_OpenStream(long *phHandle,long *pFileHeadBuf,long lSize);

extern "C" AVPLAY_API HRESULT AV_SetStreamOpenMode(long hHandle,long lMode);

extern "C" AVPLAY_API HRESULT AV_GetStreamOpenMode(long hHandle);

extern "C" AVPLAY_API HRESULT AV_CloseStream(long hHandle ,bool bCallBack);

extern "C" AVPLAY_API HRESULT AV_CloseFile(long hHandle, bool bCallBack);

extern "C" AVPLAY_API HRESULT AV_CutVdo(long hHandle, long lCut);

extern "C" AVPLAY_API HRESULT AV_RefreshSurface(long hHandle);

extern "C" AVPLAY_API HRESULT AV_RefreshByWnd(long lPlayWnd);

extern "C" AVPLAY_API HRESULT AV_Pause(long hHandle, long lPause);

extern "C" AVPLAY_API HRESULT AV_SetZoom(long hHandle, long lZoom);

extern "C" AVPLAY_API HRESULT AV_Fast(long hHandle);

extern "C" AVPLAY_API HRESULT AV_Slow(long hHandle);

extern "C" AVPLAY_API HRESULT AV_SetSpeed(long hHandle,long lSpeed);

extern "C" AVPLAY_API HRESULT AV_GetSpeed(long hHandle,long *lSpeed);

extern "C" AVPLAY_API HRESULT AV_SetPlayPos(long hHandle, long lPos);

extern "C" AVPLAY_API HRESULT AV_GetPlayPos(long hHandle, long * pPos);

extern "C" AVPLAY_API HRESULT AV_GetVersion(long * lVer);

extern "C" AVPLAY_API HRESULT AV_QueryFunction(LPCTSTR sFunctionName, long * lCanUse);

extern "C" AVPLAY_API HRESULT AV_SetFileEndMsgWnd(long hHandle, long hWnd, long lMsg);

extern "C" AVPLAY_API HRESULT AV_CapPic(long hHandle, LPCTSTR sFileName);

extern "C" AVPLAY_API HRESULT AV_CapPic_Ex(long hHandle, const char *sFileName);

extern "C" AVPLAY_API HRESULT AV_SetVolume(long hHandle, long lVolume);

extern "C" AVPLAY_API HRESULT AV_InitAudioTalk();

extern "C" AVPLAY_API HRESULT AV_SoundPlay(long hHandle);

extern "C" AVPLAY_API HRESULT AV_StopSound(long hHandle);

extern "C" AVPLAY_API HRESULT AV_GetAudioData(char* pBuf);

extern "C" AVPLAY_API HRESULT AV_GetBufferFrameCount(long hHandle);

extern "C" AVPLAY_API HRESULT AV_InputData(long hHandle, long* pBuf, long lSize);

extern "C" AVPLAY_API HRESULT AV_TalkOpen(long lEncFormat = 0x50);

extern "C" AVPLAY_API HRESULT AV_TalkClose();

extern "C" AVPLAY_API HRESULT AV_Free();

extern "C" AVPLAY_API DWORD AV_GetFileTotalFrames(LONG lHandle);

extern "C" AVPLAY_API DWORD AV_GetPlayedFrames(LONG lHandle);

extern "C" AVPLAY_API DWORD AV_GetFileTotalTimes(LONG lHandle);

extern "C" AVPLAY_API HRESULT AV_SetPlayTimes(LONG lHandle,LONG lTimes);

extern "C" AVPLAY_API DWORD AV_GetPlayedTimes(LONG lHandle);

extern "C" AVPLAY_API HRESULT AV_OneByOneBack(LONG lHandle);

extern "C" AVPLAY_API	HRESULT AV_OneByOne(LONG lHandle);

extern "C" AVPLAY_API HRESULT AV_RigisterDrawFun(LONG hHandle, long lfn, long pUser);

extern "C" AVPLAY_API HRESULT AV_GetVolume(long hHandle, long* lVolume);

extern "C" AVPLAY_API HRESULT AV_ScreenClear(long hHandle);

extern "C" AVPLAY_API HRESULT AV_BackPlayNormal(long hHandle);

extern "C" AVPLAY_API HRESULT AV_GetPlayFrameRate(long hHandle);

extern "C" AVPLAY_API double AV_GetPlayFluxRate(long hHandle);

extern "C" AVPLAY_API HRESULT AV_ReInitViewHandle(long hHandle, HWND hWnd);

extern "C" AVPLAY_API HRESULT AV_SetFileEndMessage(DWORD hThreadID, long lMessageID);

extern "C" AVPLAY_API HRESULT AV_SetCallbackAfterDecode(long hHandle, int iTerminalId, int iChlId, PFVideoData pfnVideoAfterDec, PFAudioData pfnAudioAfterDec, LPVOID Context);

/*
  中兴客户端专用
*/
extern "C" AVPLAY_API HRESULT AV_SetCallbackAfterDecode1(long hHandle, long lType, long lPara1, long lPara2);

extern "C" AVPLAY_API HRESULT AV_GetCallbackAfterDecode1(long hHandle, long lType, long *pPara1, long *pPara2);

extern "C" AVPLAY_API HRESULT AV_SetCallbackForTalk(long hHandle, long callbackptr);


/*
  华为客户端专用
*/
typedef struct
{
	long lWidth; //画面宽，单位像素。如果是音频数据则为0；
	long lHeight; //画面高。如果是音频数据则为0；
	long lStamp; //时标信息，单位毫秒。
	long lType; //数据类型，T_AUDIO16，T_RGB32， T_YV12，详见宏定义说明。
	long lFrameRate;//编码时产生的图像帧率。
}FRAME_INFO;

//#define T_AUDIO16	101
//#define T_AUDIO8	100
//#define T_UYVY		1
//#define T_YV12		3
//#define T_RGB32		7

typedef void (__stdcall *DrawCallBack2)(long, const char*, long, FRAME_INFO*, void*, long);
/*参数说明：
   lHandle:		    设置回调的播放句柄
   pBuf:			解码后的音视频数据
   lSize:			解码后的音视频数据pBuf的长度
   pFrameInfo:		图像和声音信息。详见下
   pUserData:		用户设置参数
   lReserved:		保留参数
功能描述：
设置回调函数，替换播放器中的显示部分，有用户自己控制显示，该函数在AV_OpenStream或AV_OpenFile之前调用，在Stop时自动失效，下次调用Start之前需要重新设置
*/
extern "C" AVPLAY_API HRESULT AV_SetDecodeCallBack2(long hHandle, DrawCallBack2 Pv, void* pContext);

/*
	功能描述：
	设置解码回调的流类型，在SetDecCallBack函数中生效
	参数说明：
		lHandle:		播放句柄
		lStream:		1视频流，2音频流，3复合流

*/
extern "C" AVPLAY_API HRESULT AV_SetDecCBStream(LONG lHandle, ULONG lStream);
/*
  华为客户端专用
*/

extern "C" AVPLAY_API HRESULT AV_GetEncodeFormat(long hHandle);

extern "C" AVPLAY_API HRESULT AV_SetDecodeStyle(long hHandle, int iDecodeStyle);

extern "C" AVPLAY_API HRESULT AV_AudioEncode(char *pInBuf, int iInlen, char *pOutBuf, int *pOutlen, int iType);



/**
  设置颜色参数。

  @param hHandle 解码器句柄。
  @param nBrightness 亮度，默认128，范围0-255。
  @param nContrast 对比度，默认128，范围0-255。
  @param nSaturation 饱和度，默认128，范围0-255。
  @param nHue 色度，默认128，范围0-255。
  @return 执行成功返回0，错误时返回错误码。
*/
extern "C" AVPLAY_API HRESULT AV_SetColor(long hHandle, long nBrightness, long nContrast, long nSaturation, long nHue);

/**
  获取当前设置的颜色参数。

  @param hHandle 解码器句柄。
  @param pBrightness 传出参数，亮度，默认128，范围0-255。
  @param pContrast 传出参数，对比度，默认128，范围0-255。
  @param pSaturation 传出参数，饱和度，默认128，范围0-255。
  @param pHue 传出参数，色度，默认128，范围0-255。
  @return 执行成功返回0，错误时返回错误码。
*/
extern "C" AVPLAY_API HRESULT AV_GetColor(long hHandle, long* pBrightness, long* pContrast, long* pSaturation, long* pHue);

/**
  设置视频图像翻转策略（开关量，翻转角度为180度）。

  @param hHandle 解码器句柄。
  @param lReverseImage 是否启用翻转策略，默认不启用，0 - 不启用，1 - 启用。
*/
extern "C" AVPLAY_API HRESULT AV_SetReverseImage(long hHandle, long lReverseImage);

extern "C" AVPLAY_API HRESULT AV_SetDrawframeArea(long hHandle, DRAWFRAMGROUP &DrawframeGroup);

extern "C" AVPLAY_API HRESULT AV_GetDrawframeArea(long hHandle, DRAWFRAMGROUP &PDrawframeGroup);

extern "C" AVPLAY_API HRESULT AV_SetWireFrames(long lFrames);

extern "C" AVPLAY_API HRESULT AV_SetWirelessFrames(long lFrames);

extern "C" AVPLAY_API HRESULT AV_GetWireFrames();

extern "C" AVPLAY_API HRESULT AV_GetWirelessFrames();

extern "C" AVPLAY_API HRESULT AV_GetLeftDataSize(long hHandle);

extern "C" AVPLAY_API HRESULT AV_ResetSourceBuffer(long hHandle);

extern "C" AVPLAY_API HRESULT AV_SetBackPlaySel(long hHandle, long lSel);

extern "C" AVPLAY_API HRESULT AV_GetBackPlaySel(long hHandle, long *lSel);

extern "C" AVPLAY_API HRESULT AV_SetRealPlaySel(long hHandle, long lSel);

extern "C" AVPLAY_API HRESULT AV_GetRealPlaySel(long hHandle, long *lSel);

extern "C" AVPLAY_API HRESULT AV_SetGDI(long lVlaue);

extern "C" AVPLAY_API HRESULT AV_GetGDI();

extern "C" AVPLAY_API HRESULT AV_SetLostFrame(long lVlaue);

extern "C" AVPLAY_API HRESULT AV_GetLostFrame();

extern "C" AVPLAY_API HRESULT AV_SetVideoSort(long lVlaue);

extern "C" AVPLAY_API HRESULT AV_GetVideoSort();

extern "C" AVPLAY_API HRESULT AV_SetEncoderFormatget(long lVlaue);

extern "C" AVPLAY_API HRESULT AV_GetEncoderFormatget();
 
extern "C" AVPLAY_API HRESULT AV_SetHigh(long lVlaue);

extern "C" AVPLAY_API HRESULT AV_SetAudioCallBack(long lCallback);

extern "C" AVPLAY_API HRESULT AV_GetHigh();

extern "C" AVPLAY_API HRESULT AV_SetAudioBroadcast(long lValue);

extern "C" AVPLAY_API HRESULT AV_InputPCMData(char* pPCMData, long nBufSize);

extern "C" AVPLAY_API HRESULT AV_SetPCMEncodeCallBack(long fPCMEncode, long lUser);

extern "C" AVPLAY_API HRESULT AV_SetFileSize(long hHandle, long lFileSize);

extern "C" AVPLAY_API HRESULT AV_SetPlayVideoInfo(long hHandle, long lVid, long lChlid);

extern "C" AVPLAY_API HRESULT AV_PlayWindowSelect(long hHandle, long lSelect);

extern "C" AVPLAY_API HRESULT AV_SetPowerView(long hHandle, long lPower, long lOpen);

