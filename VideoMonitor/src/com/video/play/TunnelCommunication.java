package com.video.play;

import java.util.HashMap;

import org.json.JSONException;
import org.json.JSONObject;

import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.video.R;
import com.video.service.BackstageService;
import com.video.service.MainApplication;
import com.video.socket.ZmqThread;
import com.video.terminal.player.TerminalPlayerActivity;
import com.video.utils.Tools;
import com.video.utils.Utils;

public class TunnelCommunication {

	private static TunnelCommunication tunnel = null; 
	
	public static int width = 1280; 
	public static int height = 720;
	
	//视频
	public static byte videoFrameType; 
	public static byte oldVideoFrameType; 

	public static VideoCache videoDataCache = null;
	public static byte[] naluData = new byte[width*height*3];
	public static int naluDataLen = 4;
	
	//音频
	public static byte audioFrameType; 
	public static AudioCache audioDataCache = null;
	
	//P2P动态库接口
	private native int naInitialize(String iceServersValue);
	private native int naTerminate();
	private native int naOpenTunnel(String peerId);
	private native int naCloseTunnel(String peerId);
	private native int naMessageFromPeer(String peerId, String msg);
	private native int naStartMediaData(String peerId, int level);
	private native int naStopMediaData(String peerId);
	private native int naSendTalkData(byte[] ulawData, int ulawDataLen);
	private native int naStartPeerVideoCut(String peerId, String filepath);
	private native int naStopPeerVideoCut(String peerId);
	private native int naDownloadRemoteFile(String peerId, String remoteFileName, String saveFilePath, int playSize);
	private native int naSearchLocalDevice();
	private native int naConnectLocalDevice(String peerAddr);
	private native int naDisconnectLocalDevice(String peerAddr);
	private native int naStartLocalVideo(String peerAddr);
	private native int naStopLocalVideo(String peerAddr);
	private native boolean naIsTunnelOpened(String peerId);

	synchronized public static TunnelCommunication getInstance() {
		if (tunnel == null) {
			tunnel = new TunnelCommunication();
		}
		return tunnel;
	}

	/**
	 * 初始化通道
	 */
	public int tunnelInitialize(String iceServersValue) {
		if (videoDataCache == null) {
			videoDataCache = new VideoCache(1024*1024);
		}
		if (audioDataCache == null) {
			audioDataCache = new AudioCache(1024*1024);
		}
		return naInitialize(iceServersValue);
	}

	/**
	 * 结束通道
	 */
	public int tunnelTerminate() {
		if (videoDataCache != null) {
			videoDataCache.clearBuffer();
		}
		if (audioDataCache != null) {
			audioDataCache.clearBuffer();
		}
		return naTerminate();
	}
	
	/**
	 * 判断通道是否打开
	 */
	public boolean IsTunnelOpened(String peerId) {
		return naIsTunnelOpened(peerId);
	}
	
	/**
	 * 通道被打开(回调)
	 */
	public void TunnelOpened(String peerId) {
		Utils.log("【"+peerId+"】通道被打开");
		
		Intent intent = new Intent();
		intent.putExtra("TunnelEvent", 0);
		intent.putExtra("PeerId", peerId);
		intent.setAction(BackstageService.TUNNEL_REQUEST_ACTION);
		MainApplication.getInstance().sendBroadcast(intent);
	}
	
	/**
	 * 通道被关闭(回调)
	 */
	public void TunnelClosed(String peerId) {
		Utils.log("【"+peerId+"】通道被关闭");
		
		Intent intent = new Intent();
		intent.putExtra("TunnelEvent", 1);
		intent.putExtra("PeerId", peerId);
		intent.setAction(BackstageService.TUNNEL_REQUEST_ACTION);
		MainApplication.getInstance().sendBroadcast(intent);
	}

	/**
	 * 打开通道
	 */
	public int openTunnel(String peerId) {
		return naOpenTunnel(peerId);
	}
	
	/**
	 * 关闭通道
	 */
	public int closeTunnel(String peerId) {
		return naCloseTunnel(peerId);
	}
	
	/**
	 * 请求视频数据
	 */
	public int startMediaData(String peerId, int level) {
		return naStartMediaData(peerId, level);
	}
	
	/**
	 * 关闭视频数据
	 */
	public int stopMediaData(String peerId) {
		return naStopMediaData(peerId);
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
		Log.i("tunnel","MyDebug: 【"+peerId+"】下载终端录像文件回调值："+status);
		
		Intent intent = new Intent();
		intent.putExtra("PeerId", peerId);
		intent.putExtra("RecordStatus", status);
		intent.setAction(TerminalPlayerActivity.REQUEST_TERMINAL_VIDEO_ACTION);
		MainApplication.getInstance().sendBroadcast(intent);
	}
	
	/**
	 * 搜索本地设备(回调)
	 */
	public void SearchedDevice(String devInfo) {
		try {
			JSONObject jsonObj = new JSONObject(devInfo);
			String mac = jsonObj.getString("sn");
			String ip = jsonObj.getString("ip");
			int port = jsonObj.getInt("port");
			String gateway = jsonObj.getString("gateway");
			String mask = jsonObj.getString("mask");
			
			Intent intent = new Intent();
			intent.putExtra("MAC", mac);
			intent.putExtra("IP", ip);
			intent.putExtra("Port", port);
			intent.putExtra("Gateway", gateway);
			intent.putExtra("Mask", mask);
			intent.setAction(BackstageService.SEARCH_LOCAL_DEVICE_ACTION);
			MainApplication.getInstance().sendBroadcast(intent);
			
			Log.i("tunnel","MyDebug: 【本地设备信息】 mac:"+mac+" ip:"+ip+" port:"+port+" gateway: "+gateway+" mask:"+mask);
		} catch (JSONException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * 搜索本地设备
	 */
	public void searchLocalDevice() {
		naSearchLocalDevice();
		System.out.println("MyDebug: 正在搜索本地设备...");
	}

	/**
	 * 连接本地设备
	 */
	public void connectLocalDevice(String peerAddr) {
		naConnectLocalDevice(peerAddr);
	}
	
	/**
	 * 断开本地设备连接
	 */
	public void disconnectLocalDevice(String peerAddr) {
		naDisconnectLocalDevice(peerAddr);
	}
	
	/**
	 * 打开本地设备视频
	 */
	public void startLocalVideo(String peerAddr) {
		naStartLocalVideo(peerAddr);
	}
	
	/**
	 * 关闭本地设备视频
	 */
	public void stopLocalVideo(String peerAddr) {
		naStopLocalVideo(peerAddr);
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
		sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.send_to_peer_id, map); 
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
			if(oldVideoFrameType != videoFrameType){
				Log.i("tunnel","frame type change old is "+oldVideoFrameType+" now is "+videoFrameType);
				oldVideoFrameType = videoFrameType;
			}
			if ((byte)(videoFrameType & 0x80) != 0) {//子frame
				pushPosition += 4;
			} else {//
				if(naluDataLen > 4 ){//收到新一帧数据，将上一帧数据放入播放缓冲区
					Tools.setIntValue(naluData, 0, naluDataLen-4);//前四个字节为帧长度
					if (videoDataCache.push(naluData, naluDataLen) != 0) {
						videoDataCache.clearBuffer();
					}
					//保留前四个字节为帧长度
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
