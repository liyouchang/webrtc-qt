package com.video.data;

import com.video.main.MainActivity;


public class Value {
	
	/**
	 * 服务器IP和Port
	 */
//	public final static String BackstageIPPort = "tcp://192.168.40.179:5555";
//	public final static String BackstageIPPort = "tcp://222.174.213.181:5555";
//	public final static String BackstageIPPort = "tcp://192.168.60.198:5555";
//	public final static String BackstageIPPort = "tcp://192.168.0.185:5555";
//	public final static String BackstageIPPort = "tcp://222.174.213.185:5555";
//	public final static String BackstageIPPort = "tcp://192.168.40.192:5555";
//	public final static String BackstageIPPort = "tcp://218.56.11.180:5555";
//	public final static String BackstageIPPort = "tcp://192.168.40.180:5555";
	public final static String BackstageIPPort = "tcp://218.56.11.181:5555";

	/**
	 * 设备服务器名称
	 */
	public final static String DeviceBackstageName = "Backstage";
	
	/**
	 * 报警服务器名称
	 */
	public final static String AlarmBackstageName = "Alarmstage";
	
	/**
	 * 终端服务初始化stun,turn的IP和Port
	 */
//	public final static String stun = "stun:222.174.213.185:5389";
//	public final static String turn = "turn:222.174.213.185:5766";
	
//	public final static String stun = "stun:222.174.213.181:5389";
//	public final static String turn = "turn:222.174.213.181:5766";
	
	public final static String stun = "stun:218.56.11.181:5389";
	public final static String turn = "turn:218.56.11.181:5766";
	
//	public final static String stun = "stun:192.168.40.179:5389";
//	public final static String turn = "turn:192.168.40.179:5766";
	
//	public final static String stun = "stun:218.56.11.180:5389";
//	public final static String turn = "turn:218.56.11.180:5766";
	
//	public final static String stun = "stun:192.168.40.180:5389";
//	public final static String turn = "turn:192.168.40.180:5766";
	
	/**
	 * 服务器上比较版本的xml文件地址
	 */
//	public final static String ApkXmlPath = "http://222.174.213.185:8088/plugin/VideoMonitorXml.xml";
//	public final static String ApkXmlPath = "http://222.174.213.181:8088/plugin/VideoMonitorXml.xml";
	public final static String ApkXmlPath = "http://218.56.11.181:8088/plugin/VideoMonitorXml.xml";
	
	/**
	 * realm
	 */
	public static String realm = "kaer";
	
	/**
	 * 服务的包名类名
	 */
	public final static String BackstageServicePackage = "com.video.service.BackstageService";
	
	/**
	 * 超时时间
	 */
	public final static int REQ_TIME_6S = 6000;
	public final static int REQ_TIME_10S = 10000;
	public final static int REQ_TIME_15S = 15000;
	public final static int REQ_TIME_30S = 30000;
	public final static int REQ_TIME_60S = 60000;
	
	/**
	 * 是否需要请求报警记录的标志
	 */
	public static boolean isLoginSuccess = false;
	
	/**
	 * 是否需要请求终端列表的标志
	 */
	public static boolean isNeedReqTermListFlag = true;
	
	/**
	 * 是否需要请求报警记录的标志
	 */
	public static boolean isNeedReqAlarmListFlag = true;
	
	/**
	 * 请求报警记录的数据
	 */
	public static int newAlarmMessageCount = 0;
	
	/**
	 * 报警语音是否播放标志
	 */
	public static boolean isPlayMp3 = false;
	
	/**
	 * 是否手动退出软件
	 */
	public static boolean isManulLogout = false;
	
	/**
	 * 心跳失败标志
	 */
	public static boolean beatHeartFailFlag = false;
	public static int reloginTimes = 0;
	
	/**
	 * 通道是否打开的标志
	 */
	public static boolean isTunnelOpened = false;
	
	/**
	 * 是否是共享的用户
	 */
	public static boolean isSharedUser = false;
	
	/**
	 * 实时界面请求报警消息
	 */
	public static boolean ownFragmentRequestAlarmFlag = false;
	
	/**
	 * 初始化应用程序的全局变量
	 */
	public static void resetValues() {
		isLoginSuccess = false;
		isNeedReqTermListFlag = true;
		isNeedReqAlarmListFlag = true;
		newAlarmMessageCount = 0;
		isPlayMp3 = false;
		isManulLogout = true;
		beatHeartFailFlag = false;
		reloginTimes = 0;
		isTunnelOpened = false;
		isSharedUser = false;
		ownFragmentRequestAlarmFlag = false;
		MainActivity.lastFragment = 0;
	}
}
