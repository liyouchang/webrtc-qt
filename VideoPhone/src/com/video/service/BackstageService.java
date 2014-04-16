package com.video.service;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqThread;

public class BackstageService extends Service {

	private static boolean isRun = false;
	private static BackstageThread thread = null;
	private int timeTick = 0;
	private PreferData preferData = null;
	private String userName = null;
	
	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// TODO Auto-generated method stub
		preferData = new PreferData(BackstageService.this);
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		HandlerApplication.getInstance().setMyHandler(ZmqThread.zmqThreadHandler);
		isRun = true;
		thread = new BackstageThread();
		thread.start();
		return super.onStartCommand(intent, flags, startId);
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}
	
	/**
	 * 关闭应用程序
	 */
	public static void closeAPPAndService() {
		Value.isLoginSuccess = false;
		Value.isNeedReqTermListFlag = true;
		Value.isNeedReqAlarmListFlag = true;
		if (thread != null) {
			isRun = false;
			thread = null;
		}
		try {
			sendHandlerMsg(R.id.close_zmq_socket_id);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		closeAPPAndService();
	}
	
	/**
	 * 生成JSON的发送心跳字符串
	 */
	private String generateBeatHeartJson() {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_BeatHeart");
			jsonObj.put("UserName", userName);
		} catch (JSONException e) {
			e.printStackTrace();
			if (preferData.isExist("UserName")) {
				userName = preferData.readString("UserName");
			}
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 发送Handler消息
	 */
	public static void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		Handler handler = HandlerApplication.getInstance().getMyHandler();
		if (handler != null) {
			handler.sendMessage(msg);
		} else {
			closeAPPAndService();
		}
	}
	public void sendHandlerMsg(int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		Handler handler = HandlerApplication.getInstance().getMyHandler();
		if (handler != null) {
			handler.sendMessage(msg);
		} else {
			closeAPPAndService();
		}
	}
	
	/**
	 * 后台线程
	 */
	class BackstageThread extends Thread {

		@Override
		public void run() {
			while (isRun) {
				try {
					sleep(100);
					sendHandlerMsg(R.id.zmq_recv_data_id);
					if (Value.isLoginSuccess) {
						timeTick ++;
						if (timeTick > 600) {
							timeTick = 0;
							sendHandlerMsg(R.id.zmq_send_data_id, generateBeatHeartJson());
						}
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
}
