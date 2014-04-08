package com.video.play;

import java.util.HashMap;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.video.R;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqCtrl;

public class TunnelCommunication {

	//�������ؽӿ�
	private  native int naInitialize(String classPath);
	private  native int naTerminate();
	private  native int naOpenTunnel(String peerId);
	private  native int naCloseTunnel(String peerId);
	private  native int naMessageFromPeer(String peerId, String msg);
	private  native int naAskMediaData(String peerId);
	public TunnelCommunication(){
		Log.i("construction","");
	}
	static {
		new TunnelCommunication();
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("VideoPhone");
	}
	
	private static TunnelCommunication tunnel = null;

	synchronized public static TunnelCommunication Instance() {
		if (tunnel == null) {
			tunnel = new TunnelCommunication();
		}
		return tunnel;
	}

	/**
	 * ��ʼ��ͨ��
	 */
	public  int tunnelInitialize(String classPath) {
		return naInitialize(classPath);
	}

	/**
	 * ��ֹʹ��ͨ��
	 */
	public  int tunnelTerminate() {
		return naTerminate();
	}

	/**
	 * ��ͨ��
	 */
	public  int openTunnel(String peerId) {
		return naOpenTunnel(peerId);
	}

	/**
	 * �ر�ͨ��
	 */
	public  int closeTunnel(String peerId) {
		return naCloseTunnel(peerId);
	}
	
	/**
	 * ���ԶԵȶ˵����
	 */
	public  int messageFromPeer(String peerId, String msg) {
		return naMessageFromPeer(peerId, msg);
	}

	/**
	 * �����ý�����
	 */
	public  int askMediaData(String peerId) {
		return naAskMediaData(peerId);
	}
	
	/**
	 * ����Handler��Ϣ
	 */
	private static void sendHandlerMsg(Handler handler, int what, HashMap<String, String> obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	/**
	 * ��Եȶ˷������
	 */
	public static void SendToPeer(String peerId, String data) {
		System.out.print("MyDebug: 1��SendToPeer(): "+data);
		
		HashMap<String, String> map = new HashMap<String, String>();
		map.put("peerId", peerId);
		map.put("peerData", data);
		Handler handler = HandlerApplication.getInstance().getMyHandler();
		sendHandlerMsg(handler, R.id.send_to_peer_id, map);
	}

	public static void RecvVideoData(String peerID,byte [] data) {
		System.out.println("MyDebug: 2��RecvVideoData()");
	}
	
	public static void RecvAudioData(String peerID,byte [] data) {
		System.out.println("MyDebug: 3��RecvAudioData()");
	}
	
	public static void fromJNI(int i)
	{
		Log.w("Java------>", ""+i);
	}
}
