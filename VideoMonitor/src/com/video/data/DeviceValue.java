package com.video.data;

public class DeviceValue {
	
	/**
	 * 【视频清晰度选择】 1:主通道高清  2:子通道均衡  3:子通道流畅
	 */
	public static int HIGH_CLARITY = 1;
	public static int NORMAL_CLARITY = 2;
	public static int LOW_CLARITY = 3;
	
	/*
	 * 终端设备: HashMap传输键
	 */
	public static String HASH_PLAYER_CLARITY = "playerClarity";
	
	/*
	 * 终端设备: XML保存键
	 */
	public static String XML_PLAYER_CLARITY = "clarity";
}
