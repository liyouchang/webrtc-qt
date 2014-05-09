package com.video.data;


public class Value {

	/**
	 * 设备服务器名称
	 */
	public final static String DeviceBackstageName = "Backstage";
	
	/**
	 * 报警服务器名称
	 */
	public final static String AlarmBackstageName = "Alarmstage";
	
	/**
	 * 终端的Dealer名称
	 */
	public static String TerminalDealerName = null;
	
	/**
	 * 服务器ID和Port
	 */
//	public final static String BackstageIPPort = "tcp://192.168.0.185:5555";
	public final static String BackstageIPPort = "tcp://222.174.213.185:5555";
//	public final static String BackstageIPPort = "tcp://192.168.40.191:5555";
//	public final static String BackstageIPPort = "tcp://218.56.11.182:5555";
	
	/**
	 * 服务器上比较版本的xml文件地址
	 */
	public final static String ApkXmlPath = "http://222.174.213.185:8088/plugin/VideoMonitorXml.xml";
	
	/**
	 * 超时时间
	 */
	public final static int requestTimeout = 10000;
	
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
	public static int requstAlarmCount = 5;
	
	/**
	 * 报警语音是否播放标志
	 */
	public static boolean isPlayMp3 = false;
	
	/**
	 * 是否手动退出软件
	 */
	public static boolean isManulLogout = false;
	
	
	/**
	 * 初始化应用程序的全局变量
	 */
	public static void resetValues() {
		TerminalDealerName = null;
		isLoginSuccess = false;
		isNeedReqTermListFlag = true;
		isNeedReqAlarmListFlag = true;
		requstAlarmCount = 5;
		isPlayMp3 = false;
		isManulLogout = true;
	}
}
