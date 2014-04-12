package com.video.play;

import java.io.File;
import java.util.HashMap;

import android.os.Handler;
import android.os.Message;

import com.video.R;
import com.video.data.ByteCache;
import com.video.socket.HandlerApplication;
import com.video.utils.Tools;

public class TunnelCommunication {

	private static TunnelCommunication tunnel = null; //该类单例对象
	
	public static int width = 1280; 
	public static int height = 720;
	
	public static byte frameType; //帧类型
	public static ByteCache videoDataCache = null; //缓存图像数据对象
	private static byte[] naluData = new byte[width*height*3]; //要缓存的数据
	private static int naluDataLen = 4; //要缓存的数据大小
	
	//P2P通讯库JNI接口
	private native int naInitialize(String classPath);
	private native int naTerminate();
	private native int naOpenTunnel(String peerId);
	private native int naCloseTunnel(String peerId);
	private native int naMessageFromPeer(String peerId, String msg);
	private native int naAskMediaData(String peerId);
	
	static 
	{
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("p2p");
	}

	synchronized public static TunnelCommunication getInstance() {
		if (tunnel == null) {
			tunnel = new TunnelCommunication();
		}
		return tunnel;
	}

	/**
	 * 通道初始化
	 */
	public int tunnelInitialize(String classPath) {
		return naInitialize(classPath);
	}

	/**
	 * 通道终止
	 */
	public int tunnelTerminate() {
		return naTerminate();
	}

	/**
	 * 打开通道
	 */
	public int openTunnel(String peerId) {
		return naOpenTunnel(peerId);
	}

	private static File myData = null;
	
	/**
	 * 关闭通道
	 */
	public int closeTunnel(String peerId) {
		if (videoDataCache != null) {
			videoDataCache.clearBuffer();
		}
		return naCloseTunnel(peerId);
	}
	
	public int messageFromPeer(String peerId, String msg) {
		return naMessageFromPeer(peerId, msg);
	}

	/**
	 * 请求视频数据
	 */
	public int askMediaData(String peerId) {
		if (videoDataCache == null) {
			videoDataCache = new ByteCache();
			videoDataCache.setBufferLen(1024*1024*3);
		}
		return naAskMediaData(peerId);
	}
	
	/**
	 * 发送handler消息
	 */
	private static void sendHandlerMsg(Handler handler, int what, HashMap<String, String> obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	/**
	 * 转发给终端数据
	 */
	public void SendToPeer(String peerId, String data) {
		System.out.print("MyDebug: 1、SendToPeer(): "+data);
		
		HashMap<String, String> map = new HashMap<String, String>();
		map.put("peerId", peerId);
		map.put("peerData", data);
		Handler handler = HandlerApplication.getInstance().getMyHandler();
		sendHandlerMsg(handler, R.id.send_to_peer_id, map); 
	}

	/**
	 * 收到视频数据
	 */
	public void RecvVideoData(String peerID, byte[] data) {
		int dataLen = data.length;
		frameType = (byte)(data[9]);
		
		if ((frameType & 0x5F) < 30) {
			int pushPosition = 10;
			int frameLen = Tools.getWordValue(data, pushPosition);
			pushPosition += 2;
			if (frameLen == (dataLen - pushPosition)) {
				if ((byte)(frameType & 0x80) != 0) {
					pushPosition += 4;
				} else {
					if(naluDataLen > 4 ){
						Tools.setIntValue(naluData, 0, naluDataLen-4);
						if (videoDataCache.push(naluData, naluDataLen) != 0) {
							videoDataCache.clearBuffer();
						}
						naluDataLen = 4;
					}
				}
				int naluLen = dataLen - pushPosition;
				Tools.CopyByteArray(naluData, naluDataLen, data, pushPosition, naluLen);
				naluDataLen += naluLen;
			}
		}
	}
	
	/**
	 * 收到音频数据
	 */
	public void RecvAudioData(String peerID,byte [] data) {
		
	}
}
