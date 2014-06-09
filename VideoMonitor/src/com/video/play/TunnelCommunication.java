package com.video.play;

import java.util.HashMap;

import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.video.R;
import com.video.data.Value;
import com.video.socket.HandlerApplication;
import com.video.terminal.player.TerminalPlayerActivity;
import com.video.utils.Tools;

public class TunnelCommunication {

	private static TunnelCommunication tunnel = null; //
	
	public static int width = 1280; 
	public static int height = 720;
	
	//视频
	public static byte videoFrameType; 
	public static VideoCache videoDataCache = null;
	public static byte[] naluData = new byte[width*height*3];
	public static int naluDataLen = 4;
	
	//音频
	public static byte audioFrameType; 
	public static AudioCache audioDataCache = null;
	
	//P2P动态库接口
	private native int naInitialize(String classPath);
	private native int naTerminate();
	private native int naOpenTunnel(String peerId);
	private native int naCloseTunnel(String peerId);
	private native int naMessageFromPeer(String peerId, String msg);
	private native int naAskMediaData(String peerId);
	private native int naSendTalkData(byte[] ulawData, int ulawDataLen);
	private native int naStartPeerVideoCut(String peerId, String filepath);
	private native int naStopPeerVideoCut(String peerId);
	private native int naDownloadRemoteFile(String peerId, String remoteFileName, String saveFilePath, int playSize);
	private native int naSearchLocalDevice();
	private native int naConnectLocalDevice(String peerAddr);
	private native int naDisconnectLocalDevice(String peerAddr);
	private native int naStartLocalVideo(String peerAddr);
	private native int naStopLocalVideo(String peerAddr);
	
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
	 * 初始化通道
	 */
	public int tunnelInitialize(String classPath) {
		Log.i("tag","tunnelInitialize start");
		naInitialize(classPath);
		Log.i("tag","tunnelInitialize end");
		return 0;
	}

	/**
	 * 结束通道
	 */
	public int tunnelTerminate() {
		return naTerminate();
	}
	
	/**
	 * 通道被打开(回调)
	 */
	public void TunnelOpened(String peerId) {
		System.out.println("MyDebug: 【"+peerId+"】通道被打开");
		PlayerActivity.requestPlayerTimes = 0;
		
		Intent intent = new Intent();
		intent.putExtra("TunnelEvent", 0);
		intent.putExtra("PeerId", peerId);
		if (Value.playTerminalVideoFileFlag) {
			intent.setAction(PlayerActivity.TUNNEL_REQUEST_ACTION);
			HandlerApplication.getInstance().sendBroadcast(intent);
		} else {
			intent.setAction(PlayerActivity.TUNNEL_REQUEST_ACTION);
			HandlerApplication.getInstance().sendBroadcast(intent);
		}
	}
	
	/**
	 * 通道被关闭(回调)
	 */
	public void TunnelClosed(String peerId) {
		System.out.println("MyDebug: 【"+peerId+"】通道被关闭");
		PlayerActivity.requestPlayerTimes = 0;
		
		Intent intent = new Intent();
		intent.putExtra("TunnelEvent", 1);
		intent.putExtra("PeerId", peerId);
		if (Value.playTerminalVideoFileFlag) {
			intent.setAction(PlayerActivity.TUNNEL_REQUEST_ACTION);
			HandlerApplication.getInstance().sendBroadcast(intent);
		} else {
			intent.setAction(PlayerActivity.TUNNEL_REQUEST_ACTION);
			HandlerApplication.getInstance().sendBroadcast(intent);
		}
	}

	/**
	 * 打开通道
	 */
	public int openTunnel(String peerId) {
		if (videoDataCache == null) {
			videoDataCache = new VideoCache(1024*1024*3);
		}
		if (audioDataCache == null) {
			audioDataCache = new AudioCache(1024*1024);
		}
		return naOpenTunnel(peerId);
	}
	
	/**
	 * 关闭通道
	 */
	public int closeTunnel(String peerId) {
		if (videoDataCache != null) {
			videoDataCache.clearBuffer();
		}
		if (audioDataCache != null) {
			audioDataCache.clearBuffer();
		} 
		return naCloseTunnel(peerId);
	}
	
	/**
	 * 请求视频数据
	 */
	public int askMediaData(String peerId) {
		return naAskMediaData(peerId);
	}
	
	/**
	 * 开始录视频
	 */
	public int startRecordVideo(String peerId, String filepath) {
		return naStartPeerVideoCut(peerId, filepath);
	}
	
	/**
	 * 停止录视频
	 */
	public int stopRecordVideo(String peerId) {
		return naStopPeerVideoCut(peerId);
	}
	
	/**
	 * 发送对讲数据
	 */
	public int sendTalkData(byte[] ulawData, int ulawDataLen) {
		return naSendTalkData(ulawData, ulawDataLen);
	}
	
	/**
	 * 下载终端录像文件
	 */
	public int downloadRemoteFile(String peerId, String remoteFileName, String saveFilePath, int playSize) {
		return naDownloadRemoteFile(peerId, remoteFileName, saveFilePath, playSize);
	}
	
	/**
	 * 下载终端录像文件(回调)
	 * status的值如下:
	 * 0:请求录像成功
	 * 1:保存录像失败
	 * 2:请求文件错误
	 * 3:下载录像完成
	 * 4:达到播放阀值
	 * 5:请求消息错误
	 */
	public void RecordStatus(String peerId, int status) {
		System.out.println("MyDebug: 【"+peerId+"】下载终端录像文件回调值："+status);
		
		Intent intent = new Intent();
		intent.putExtra("PeerId", peerId);
		intent.putExtra("RecordStatus", status);
		intent.setAction(TerminalPlayerActivity.REQUEST_TERMINAL_VIDEO_ACTION);
		HandlerApplication.getInstance().sendBroadcast(intent);
	}
	
	public void SearchedDevice(String devInfo) {
		System.out.println("MyDebug: 【搜索设备信息】"+devInfo);
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
	private static void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	/**
	 * 向P2P发送消息
	 */
	public void SendToPeer(String peerId, String data) {
		HashMap<String, String> map = new HashMap<String, String>();
		map.put("peerId", peerId);
		map.put("peerData", data);
		Handler handler = HandlerApplication.getInstance().getMyHandler();
		sendHandlerMsg(handler, R.id.send_to_peer_id, map); 
	}
	
	/**
	 * 向终端发送消息
	 */
	public int messageFromPeer(String peerId, String msg) {
		return naMessageFromPeer(peerId, msg);
	}

	/**
	 * 接收的视频数据(回调)
	 */
	public void RecvVideoData(String peerID, byte[] data) {
		int dataLen = data.length;
		videoFrameType = (byte)(data[9]);
		
		int pushPosition = 10;
		int frameLen = Tools.getWordValue(data, pushPosition);
		pushPosition += 2;
		if (frameLen == (dataLen - pushPosition)) {
			if ((byte)(videoFrameType & 0x80) != 0) {
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
	
	/**
	 * 接收的音频数据(回调)
	 */
	public void RecvAudioData(String peerID,byte [] data) {
		int dataLen = data.length;
		audioFrameType = (byte)(data[9]);
		
		int pushPosition = 10;
		int frameLen = Tools.getWordValue(data, pushPosition);
		pushPosition += 2;
		if (frameLen == (dataLen - pushPosition)) {
			pushPosition += 4;
			audioDataCache.push(data, pushPosition, dataLen - pushPosition);
		}
	}
}
