package com.video.service;

import com.video.R;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqThread;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

public class BackstageService extends Service {

	private boolean isRun = false;
	private BackstageThread thread = null;
	
	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// TODO Auto-generated method stub
		HandlerApplication.getInstance().setMyHandler(ZmqThread.zmqThreadHandler);
		isRun = true;
		thread = new BackstageThread();
		thread.start();
		System.out.println("MyDebug: 【打开服务】");
		return super.onStartCommand(intent, flags, startId);
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		if (thread != null) {
			isRun = false;
			thread = null;
		}
		sendHandlerMsg(R.id.close_zmq_socket_id);
		System.out.println("MyDebug: 【关闭服务】");
	}
	
	/**
	 * 发送Handler消息
	 */
	public void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		Handler handler = HandlerApplication.getInstance().getMyHandler();
		if (handler != null) {
			handler.sendMessage(msg);
		} else {
			if (thread != null) {
				isRun = false;
				thread = null;
			}
		}
	}
	
	/**
	 * 后台线程
	 */
	class BackstageThread extends Thread {

		@Override
		public void run() {
			while (isRun) {
				sendHandlerMsg(R.id.zmq_recv_data_id);
				try {
					sleep(100);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
}
