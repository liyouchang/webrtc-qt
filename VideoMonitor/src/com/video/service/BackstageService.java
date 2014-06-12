package com.video.service;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.play.TunnelCommunication;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqCtrl;
import com.video.socket.ZmqThread;
import com.video.user.LoginActivity;
import com.video.utils.Utils;

public class BackstageService extends Service {

	public static boolean isRun = false;
	private static BackstageThread thread = null;
	private int timeTick = 0;
	private PreferData preferData = null;
	private String userName = null;
	
	static 
	{
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("p2p");
		System.loadLibrary("h264");
	}
	
	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
		// 初始化通道
		TunnelCommunication.getInstance().tunnelInitialize("com/video/play/TunnelCommunication");
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// TODO Auto-generated method stub
		//初始化数据
		preferData = new PreferData(BackstageService.this);
		HandlerApplication.getInstance().setMyHandler(ZmqThread.zmqThreadHandler);
		
		//注册广播
		IntentFilter mFilter = new IntentFilter();
        mFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        registerReceiver(serviceReceiver, mFilter);
        
        //启动线程
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
		if (thread != null) {
			isRun = false;
			thread = null;
		}
		System.out.println("MyDebug: 【停止服务】");
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		closeAPPAndService();
		try {
			sendHandlerMsg(R.id.close_zmq_socket_id);
		} catch (Exception e) {
			e.printStackTrace();
		}
		// 注销广播
		unregisterReceiver(serviceReceiver);
		// 注销通道
		TunnelCommunication.getInstance().tunnelTerminate();
	}
	
	/**
	 * 生成JSON的发送心跳字符串
	 */
	private String generateBeatHeartJson() {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_BeatHeart");
			if (preferData.isExist("UserName")) {
				userName = preferData.readString("UserName");
			}
			jsonObj.put("UserName", userName);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 发送Handler消息
	 */
	private static void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		Handler handler = HandlerApplication.getInstance().getMyHandler();
		if (handler != null) {
			handler.sendMessage(msg);
		} else {
			closeAPPAndService();
		}
	}
	private void sendHandlerMsg(int what, int timeout) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessageDelayed(msg, timeout);
	}
	private void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	private void sendHandlerMsg(int what, String obj) {
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
	
	/**
	 * 生成JSON的登录字符串
	 */
	private String generateLoginJson() {
		String userName = "";
		String userPwd = "";
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		if (preferData.isExist("UserPwd")) {
			userPwd = preferData.readString("UserPwd");
		}
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_Login");
			jsonObj.put("UserName", userName);
			jsonObj.put("Pwd", Utils.CreateMD5Pwd(userPwd));
			return jsonObj.toString();
		} catch (JSONException e) {
			System.out.println("MyDebug: generateLoginJson()异常！");
			e.printStackTrace();
		}
		return null;
	}
	
	private int loginTimes = 0;
	private final static int LOGIN_TIMEOUT = 1;
	private final int LOGIN_AGAIN = 2;
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				//登录超时
				case LOGIN_TIMEOUT:
					if (handler.hasMessages(LOGIN_TIMEOUT)) {
						handler.removeMessages(LOGIN_TIMEOUT);
					}
					if (!Value.isLoginSuccess) {
						loginTimes ++;
						
						//超时之后关闭服务，断开连接，再重启服务
						stopSelf();
						sendHandlerMsg(LOGIN_AGAIN, 2000);
					}
					break;
				//重新登录
				case LOGIN_AGAIN:
					ZmqCtrl.getInstance().init();
					if (loginTimes >= 3) {
						loginTimes = 0;
						if (Value.beatHeartFailFlag) {
							Intent intent = new Intent(BackstageService.this, LoginActivity.class);
							intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
							startActivity(intent);
						}
					} else {
						Handler sendHandler = ZmqThread.zmqThreadHandler;
						String data = generateLoginJson();
						sendHandlerMsg(LOGIN_TIMEOUT, Value.requestTimeout);
						sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
					}
					break;
			}
		}
	};	

	private BroadcastReceiver serviceReceiver = new BroadcastReceiver() {
		
		private ConnectivityManager connectivityManager;
		private NetworkInfo info;
		private boolean networkChangeFlag = false;
		
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
				
				System.out.println("MyDebug: 【网络状态已经改变】");
				connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
				info = connectivityManager.getActiveNetworkInfo();
				
				if ((info != null) && (info.isAvailable()) && networkChangeFlag) {
					ZmqCtrl.getInstance().init();
					networkChangeFlag = false;
					String name = info.getTypeName();
					System.out.println("MyDebug: 【当前网络名称】" + name);
					
					//重新登录
					Value.beatHeartFailFlag = true;
					Handler sendHandler = ZmqThread.zmqThreadHandler;
					String data = generateLoginJson();
					sendHandlerMsg(LOGIN_TIMEOUT, Value.requestTimeout);
					sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
				} 
				else if (Value.isLoginSuccess) {
					networkChangeFlag = true;
					Value.resetValues();
					closeAPPAndService();
					ZmqCtrl.getInstance().exit();
					System.out.println("MyDebug: 【没有可用网络】");
				}
			}
		}
	};
}
