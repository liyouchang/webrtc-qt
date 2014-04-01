package com.video.socket;

public class ZmqCtrl {
	
	private static ZmqCtrl zmqCtrlInstance = null;
	private ZmqSocket zmqSocketInstance = null;
	
	/**
	 * 实例化ZmqCtrl的单例对象
	 */
	synchronized public static ZmqCtrl getInstance() {
		if (zmqCtrlInstance == null) {
			zmqCtrlInstance = new ZmqCtrl();
			System.out.println("MyDebug: 【实例化ZmqCtrl对象】");
		}
		return zmqCtrlInstance;
	}
	
	/**
	 * 初始化ZMQ, 启动相关线程
	 */
	public void init() {
		if (zmqSocketInstance == null) {
			zmqSocketInstance = new ZmqSocket();
		}
	}
	
	/**
	 * 关闭ZMQ连接句柄
	 */
	public void exit() {
		if (zmqSocketInstance != null) {
			zmqSocketInstance.zmqExit();
			zmqSocketInstance = null;
		}
		if (zmqCtrlInstance != null) {
			zmqCtrlInstance = null;
		}
	}
}
