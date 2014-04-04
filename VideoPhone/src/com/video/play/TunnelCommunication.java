package com.video.play;

public class TunnelCommunication {

	//声明本地接口
	private static native int naInitialize(String classPath);
	private static native int naTerminate();
	private static native int naOpenTunnel(String peerId);
	private static native int naCloseTunnel(String peerId);
	private static native int naAskMediaData(String peerId);

	static {
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("VideoPhone");
	}

	/**
	 * 初始化通道
	 */
	public static int tunnelInitialize(String classPath) {
		return naInitialize(classPath);
	}

	/**
	 * 终止使用通道
	 */
	public static int tunnelTerminate() {
		return naTerminate();
	}

	/**
	 * 打开通道
	 */
	public static int openTunnel(String peerId) {
		return naOpenTunnel(peerId);
	}

	/**
	 * 关闭通道
	 */
	public static int closeTunnel(String peerId) {
		return naCloseTunnel(peerId);
	}

	/**
	 * 请求多媒体数据
	 */
	public static int askMediaData(String peerId) {
		return naAskMediaData(peerId);
	}

	public static void SendToPeer(String arg1, String arg2) {
		System.out.println("MyDebug: 1、SendToPeer()");
	}

	public static void RecvVideoData(String arg1) {
		System.out.println("MyDebug: 2、RecvVideoData()");
	}
	
	public static void RecvAudioData(String arg1) {
		System.out.println("MyDebug: 3、RecvAudioData()");
	}
}
