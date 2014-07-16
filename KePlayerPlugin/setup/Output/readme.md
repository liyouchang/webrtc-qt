###版本说明:
V0.51 

V0.35 
1. 增加DestroyAll()函数,在js中的window.onbeforeunload,解决IE浏览器刷新无法销毁对象的问题

V0.33
1. 对ActiveX插件加入安全标记,此插件为标记为安全的activex插件
2. 问题:IE浏览器第一次刷新界面插件无法销毁,如果正在观看视频时刷新,会导致下一次观看视频时异常.
3. 问题:chrome浏览器如果需要授权插件运行,在javasricpt onload()函数中的插件初始化操作会失败,需要手动调用initialize或者重新刷新一次.

V0.32 
1. 修改Initialize函数接口
2. 增加日志功能

###接口说明:

1. Initialize( routerUrl, jstrIceServers)

	参数说明:	
	routerUrl:服务器url,如"tcp://222.174.213.185:5555"	
	jstrIceServers: ice服务器的地址列表,这是一个json字符串,如 "[{"uri":"stun:222.174.213.185:5389"},{"uri":"turn:222.174.213.185:5766"}]"
	
    返回值说明:	
	KE_FAILED - 失败	
	KE_SUCCESS - 成功

2. OpenTunnel( peer_id)

	打开指定通道.
    
	参数说明:
	peer_id: 通道id
    
	返回值说明:	
	KE_FAILED - 失败	
	KE_SUCCESS - 成功

3. CloseTunnel(peer_id)
	
    关闭指定通道.
	
    参数说明:	
	peer_id: 通道id

	返回值说明:	
	KE_FAILED - 失败	
	KE_SUCCESS - 成功

4. IsTunnelOpened( peer_id)
	
    判断指定通道是否打开,
	
    参数说明:
	peer_id: 通道id

	返回值说明:
	true - 已打开, false -  未打开

5. StartVideo( peer_id, video);
	开始指定通道视频
	
    参数说明:	
	peer_id: 通道id	
	video: 1-高清播放,2-均衡播放,3-流畅播放
    
6. StopVideo(peerId);

	停止指定通道视频
	
    参数说明: 
    peer_id: 通道id

7. GetVersion();
	返回版本号
	
	返回值说明:	
	返回一个版本号数值,除以100为主版本号,余100为子版本号,如返回35则版本号应为0.35,返回101版本号为1.01

8. SearchLocalDevice();
	
	UDP广播查找本地终端设备
	
###回调说明:
1. TunnelOpened(peerId);

2. TunnelClosed(peerId);

3. RecvPeerMsg(peerId, msg)

4. LocalDeviceInfo(devInfo);

	SearchLocalDevice查找设备返回,有一个设备回应函数回调一次
	
	
###枚举类型说明:

    KE_SUCCESS = 0,
    KE_FAILED = 10000,
    KE_PARAM_ERROR = 10001,
    KE_TUNNEL_NOT_OPEN = 10002,
    KE_VIDEO_CUT_FAILED = 10003,
    KE_VIDEO_START_FAILED = 10004,
    KE_CAPTURE_FAILED = 10005,
    KE_OTHER_ERROR
