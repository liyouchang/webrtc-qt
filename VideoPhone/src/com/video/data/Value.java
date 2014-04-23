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
	public static String TerminalDealerName = "1234567";
	
	/**
	 * 服务器ID和Port
	 */
	public final static String BackstageIPPort = "tcp://192.168.40.191:5555";
//	public final static String BackstageIPPort = "tcp://218.56.11.182:5555";
	
	/**
	 * 超时时间
	 */
	public final static int requestTimeout = 6000;
	
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
	
	public static String SetDeviceBgActivityImagePath = null;
	
	/**
	 * 初始化应用程序的全局变量
	 */
	public static void resetValues() {
		isLoginSuccess = false;
		isNeedReqTermListFlag = true;
		isNeedReqAlarmListFlag = true;
	}
}
