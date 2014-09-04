package com.video.service;

import org.json.JSONException;
import org.json.JSONObject;

import android.annotation.SuppressLint;
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
import com.video.socket.ZmqCtrl;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

@SuppressLint("HandlerLeak")
public class BackstageService extends Service {

	public static boolean isRun = false;
	private static BackstageThread thread = null;
	private int timeTick = 0;
	private PreferData preferData = null;
	private String userName = null;
	
	public static final String TUNNEL_REQUEST_ACTION = "BackstageService.TunnelRequest"; // 通道是否被打开
	public static final String BACKSTAGE_MESSAGE_ACTION = "BackstageService.backstage_message"; // 接收报警消息
	public static final String SEARCH_LOCAL_DEVICE_ACTION = "BackstageService.search_local_device"; // 搜索本地设备
	public static final String CHANGE_DEVICE_LIST_ACTION = "BackstageService.change_device_list"; // 更新设备列表状态
	public static final String TERM_ONLINE_STATE_ACTION = "BackstageService.term_online_state"; // 终端上下线状态
	
	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
		// 初始化通道
		TunnelCommunication.getInstance().tunnelInitialize(MainApplication.getInstance().generateStunJson(Value.stun));
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// TODO Auto-generated method stub
		//初始化数据
		preferData = new PreferData(BackstageService.this);
		
		//注册广播
		IntentFilter mFilter = new IntentFilter();
		mFilter.addAction(TUNNEL_REQUEST_ACTION);
		mFilter.addAction(TERM_ONLINE_STATE_ACTION);
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
		Utils.log("【停止服务】");
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
		if (ZmqThread.zmqThreadHandler != null) {
			ZmqThread.zmqThreadHandler.sendMessage(msg);
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
		if (ZmqThread.zmqThreadHandler != null) {
			ZmqThread.zmqThreadHandler.sendMessage(msg);
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
					// 100ms轮询ZMQ是否收到数据
					sendHandlerMsg(R.id.zmq_recv_data_id);
					
					if (Value.isLoginSuccess) {
						timeTick ++;
						
						// 联机操作
						if ((timeTick % 10) == 0) {
							linkDevice();
						}
						
						// 每分钟与服务器1次心跳
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
	 * 联机操作
	 */
	private void linkDevice() {
		if (MainApplication.getInstance().deviceList == null) {
			return ;
		}
		
		int size = MainApplication.getInstance().deviceList.size();
		if (size > 0) {
			for (int i=0; i<size; i++) {
				if (MainApplication.getInstance().deviceList.get(i).get("isOnline").equals("true")) {
					String peerId = MainApplication.getInstance().deviceList.get(i).get("dealerName");
					String linkState = MainApplication.getInstance().deviceList.get(i).get("LinkState");
					
					if (linkState.equals("notlink")) {
						if (TunnelCommunication.getInstance().IsTunnelOpened(peerId)) {
							// 已联机
							MainApplication.getInstance().deviceList.get(i).put("LinkState", "linked");
						} else {
							if (TunnelCommunication.getInstance().openTunnel(peerId) == 0) {
								// 正在联机...
								MainApplication.getInstance().deviceList.get(i).put("LinkState", "linking");
							} else {
								MainApplication.getInstance().deviceList.get(i).put("LinkState", "notlink");
							}
						}
					}
					else if (linkState.equals("timeout")) {
						if (TunnelCommunication.getInstance().IsTunnelOpened(peerId)) {
							// 已联机
							MainApplication.getInstance().deviceList.get(i).put("LinkState", "linked");
						} else {
							// 发送超时正在联机延时消息
							if (!handler.hasMessages(LINK_TIMEOUT)) {
								MainApplication.getInstance().sendHandlerMsg(handler, LINK_TIMEOUT, i, 0, peerId, 3000);
							}
						}
					}
					// 发送更新设备列表状态的广播
					sendChangeDeviceListBroadcast(false);
				} else {
					continue;
				}
			}
		}
	}
	
	/**
	 * 发送更新设备列表状态的广播
	 */
	public void sendChangeDeviceListBroadcast(boolean isTermActive) {
		Intent actionIntent = new Intent();
		actionIntent.putExtra("isTermActive", isTermActive);
		actionIntent.setAction(BackstageService.CHANGE_DEVICE_LIST_ACTION);
		sendBroadcast(actionIntent);
	}
	
	private int loginTimes = 0;
	private final static int LOGIN_TIMEOUT = 1;
	private final int LOGIN_AGAIN = 2;
	private final int LINK_TIMEOUT = 3;
	
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
						ZmqCtrl.getInstance().exit();
				    	stopService(new Intent(BackstageService.this, BackstageService.class));
						Value.resetValues();
						sendHandlerMsg(LOGIN_AGAIN, 3000);
					}
					break;
				//重新登录
				case LOGIN_AGAIN:
					ZmqCtrl.getInstance().init();
					if (loginTimes >= 4) {
						loginTimes = 0;
					} else {
						Value.beatHeartFailFlag = true;
						Handler sendHandler = ZmqThread.zmqThreadHandler;
						String data = MainApplication.getInstance().generateLoginJson(MainApplication.getInstance().userName, MainApplication.getInstance().userPwd);
						sendHandlerMsg(LOGIN_TIMEOUT, Value.REQ_TIME_6S);
						sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
					}
					break;
				// 超时正在联机...
				case LINK_TIMEOUT:
					if (TunnelCommunication.getInstance().openTunnel((String) msg.obj) == 0) {
						MainApplication.getInstance().deviceList.get(msg.arg1).put("LinkState", "linking");
					} else {
						MainApplication.getInstance().deviceList.get(msg.arg1).put("LinkState", "notlink");
					}
					if (handler.hasMessages(LINK_TIMEOUT)) {
						handler.removeMessages(LINK_TIMEOUT);
					}
					sendChangeDeviceListBroadcast(false);
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
				// 网络变化后，处理登录操作
				connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
				info = connectivityManager.getActiveNetworkInfo();
				
				if ((info != null) && (info.isAvailable()) && networkChangeFlag) {
					ZmqCtrl.getInstance().init();
					networkChangeFlag = false;
					Utils.log("【网络已连接】");
					
					// 重新登录
					Value.beatHeartFailFlag = true;
					Handler sendHandler = ZmqThread.zmqThreadHandler;
					String data = MainApplication.getInstance().generateLoginJson(MainApplication.getInstance().userName, MainApplication.getInstance().userPwd);
					sendHandlerMsg(LOGIN_TIMEOUT, Value.REQ_TIME_6S);
					sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
				} 
				else if (Value.isLoginSuccess) {
					Utils.log("【没有可用网络】");
					networkChangeFlag = true;
					Value.resetValues();
					closeAPPAndService();
					ZmqCtrl.getInstance().exit();
				}
			}
			else if (action.equals(TUNNEL_REQUEST_ACTION)) {
				// 联机的4种状态：linked:已联机 notlink:无法联机 linking:正在联机... timeout:联机超时
				int TunnelEvent = intent.getIntExtra("TunnelEvent", 1);
				String peerId = intent.getStringExtra("peerId");
				int position = MainApplication.getInstance().getDeviceListPositionByDealerName(peerId);
				if (position == -1) {
					return ;
				}
				
				switch (TunnelEvent) {
					// 通道被打开
					case 0:
						// 已联机
						MainApplication.getInstance().deviceList.get(position).put("LinkState", "linked");
						break;
					// 通道被关闭
					case 1:
						// 联机超时
						MainApplication.getInstance().deviceList.get(position).put("LinkState", "timeout");
						break;
				}
				sendChangeDeviceListBroadcast(false);
			}
			else if (action.equals(TERM_ONLINE_STATE_ACTION)) {
				// 终端上下线
				String mac = intent.getStringExtra("deviceID");
				int position = MainApplication.getInstance().getDeviceListPositionByDeviceID(mac);
				if (position == -1) {
					return ;
				}
				String dealerName = intent.getStringExtra("dealerName");
				String isOnline = intent.getStringExtra("isOnline");
				
				MainApplication.getInstance().deviceList.get(position).put("dealerName", dealerName);
				MainApplication.getInstance().deviceList.get(position).put("isOnline", isOnline);
				if (!isOnline.equals("true")) {
					MainApplication.getInstance().deviceList.get(position).put("LinkState", "notlink");
				}
				MainApplication.getInstance().xmlDevice.updateItemState(mac, isOnline, dealerName);
				sendChangeDeviceListBroadcast(true);
			}
		}
	};
}
