package com.video.play;

import java.util.HashMap;

import android.os.Handler;
import android.os.Message;

import com.video.R;
import com.video.socket.HandlerApplication;

public class TunnelCommunication {

	//声明本地接口
	private static native int naInitialize(String classPath);
	private static native int naTerminate();
	private static native int naOpenTunnel(String peerId);
	private static native int naCloseTunnel(String peerId);
	private static native int naMessageFromPeer(String peerId, String msg);
	private static native int naAskMediaData(String peerId);

	static {
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("VideoPhone");
	}
	
	/**
	 * 发送Handler消息
	 */
	private static void sendHandlerMsg(Handler handler, int what, HashMap<String, String> obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
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
	 * 来自对等端的数据
	 */
	public static int messageFromPeer(String peerId, String msg) {
		return naMessageFromPeer(peerId, msg);
	}

	/**
	 * 请求多媒体数据
	 */
	public static int askMediaData(String peerId) {
		return naAskMediaData(peerId);
	}

	/**
	 * 向对等端发送数据
	 */
	public static void SendToPeer(String peerId, String data) {
		System.out.print("MyDebug: 1、SendToPeer(): "+data);
		
		HashMap<String, String> map = new HashMap<String, String>();
		map.put("peerId", peerId);
		map.put("peerData", data);
		Handler handler = HandlerApplication.getInstance().getMyHandler();
		sendHandlerMsg(handler, R.id.send_to_peer_id, map);
	}

	public static void RecvVideoData(String arg1) {
		System.out.println("MyDebug: 2、RecvVideoData()");
	}
	
	public static void RecvAudioData(String arg1) {
		System.out.println("MyDebug: 3、RecvAudioData()");
	}
}
