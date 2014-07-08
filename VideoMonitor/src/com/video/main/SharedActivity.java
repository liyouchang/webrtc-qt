package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.BitmapDrawable;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.view.ViewPager.LayoutParams;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.play.PlayerActivity;
import com.video.play.TunnelCommunication;
import com.video.service.BackstageService;
import com.video.service.MainApplication;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.user.LoginActivity;
import com.video.utils.DeviceItemAdapter;
import com.video.utils.OkCancelDialog;
import com.video.utils.OkOnlyDialog;
import com.video.utils.PopupWindowAdapter;
import com.video.utils.Utils;

public class SharedActivity extends Activity implements OnClickListener {
	
	private Context mContext;
	//终端列表项
	private static String mDeviceName = null;
	private static String mDeviceId = null;
	private static int listPosition = 0;
	private static int listSize = 0;
	private RelativeLayout noDeviceLayout = null;
	
	private PopupWindow mPopupWindow;
	private Dialog mDialog = null;
	
	private static ArrayList<HashMap<String, String>> sharedList = null;
	private static DeviceItemAdapter deviceAdapter = null;
	private ListView lv_list;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	private final int LINK_TIMEOUT = 3;
	private final int REFRESH_DEVICE_LIST = 4;
	
	private LinkDeviceThread linkDeviceThread = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shared);
		initView();
		initData();
	}
	
	private void initView () {
		ImageButton back = (ImageButton) this.findViewById(R.id.btn_shared_back);
		back.setOnClickListener(this);
		
		noDeviceLayout = (RelativeLayout) this.findViewById(R.id.rl_no_shared_list);
		
		lv_list = (ListView) this.findViewById(R.id.shared_list);
		lv_list.setOnItemClickListener(new OnItemClickListenerImpl());
		lv_list.setOnItemLongClickListener(new OnItemLongClickListenerImpl());
	}
	
	private void initData() {
		//初始化Activity要使用的参数
		mContext = SharedActivity.this;
		ZmqHandler.mHandler = handler;
		
		Value.isSharedUser = true;
		
		//注册广播
		IntentFilter mFilter = new IntentFilter();
		mFilter.addAction(BackstageService.TUNNEL_REQUEST_ACTION);
		mFilter.addAction(BackstageService.TERM_ONLINE_STATE_ACTION);
        mFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        registerReceiver(serviceReceiver, mFilter);
		
		//初始化终端列表的显示
		reqTermListEvent();
	}
	
	/**
	 * 生成JSON的请求分享列表字符串
	 */
	private String generateReqTermListJson() {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ReqShareList");
			jsonObj.put("UserName", MainApplication.getInstance().userName);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的删除终端分享字符串
	 */
	private String generateDelShareTermItemJson(String mac) {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelShareTerm");
			jsonObj.put("UserName", MainApplication.getInstance().userName);
			jsonObj.put("MAC", mac);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	private Handler handler = new Handler() {

		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUESTING:
					if (mDialog == null) {
						mDialog = Utils.createLoadingDialog(mContext, (String) msg.obj);
						mDialog.show();
					}
					break;
				case REQUEST_TIMEOUT:
					if ((mDialog != null) && (mDialog.isShowing())) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					Toast.makeText(mContext, ""+msg.obj, Toast.LENGTH_SHORT).show();
					listSize = 0;
					break;
				//请求终端列表
				case R.id.request_device_share_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							ArrayList<HashMap<String, String>> listObj = (ArrayList<HashMap<String, String>>) msg.obj;
							if (listObj != null) {
								sharedList = listObj;
								deviceAdapter = new DeviceItemAdapter(mContext, sharedList);
								lv_list.setAdapter(deviceAdapter);
								listSize = sharedList.size();
								linkDeviceThread = new LinkDeviceThread(true);
								linkDeviceThread.start();
							} else {
								listSize = 0;
								if (sharedList != null) {
									sharedList.clear();
									deviceAdapter.notifyDataSetChanged();
								}
							}
						} else {
							Toast.makeText(mContext, msg.obj+"，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.request_device_share_id);
					}
					break;
				//删除终端分享
				case R.id.delete_device_share_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							sharedList.remove(listPosition);
							deviceAdapter.notifyDataSetChanged();
							listSize = sharedList.size();
							Toast.makeText(mContext, "删除终端分享成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "删除终端分享失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_device_share_id);
					}
					break;
				// 联机超时3秒后，重新联机
				case LINK_TIMEOUT:
					if (TunnelCommunication.getInstance().openTunnel((String) msg.obj) == 0) {
						sharedList.get(msg.arg1).put("LinkState", "linking");
					} else {
						sharedList.get(msg.arg1).put("LinkState", "notlink");
					}
					if (handler.hasMessages(LINK_TIMEOUT)) {
						handler.removeMessages(LINK_TIMEOUT);
					}
					break;
				// 刷新终端列表图片
				case REFRESH_DEVICE_LIST:
					if (deviceAdapter != null) {
						if (msg.obj != null) {
							deviceAdapter.notifyDataSetChanged();
						} else {
							deviceAdapter.notifyDataSetChanged();
						}
					}
					break;
			}
			if (listSize == 0) {
				noDeviceLayout.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.INVISIBLE);
			}
		}
	};
	
	/**
	 * 联机操作后台线程
	 */
	public class LinkDeviceThread extends Thread {
		private boolean isRun = false;
		
		public LinkDeviceThread(boolean isRun) {
			this.isRun = isRun;
		}
		
		public void stopThread() {
			isRun = false;
			if (linkDeviceThread != null) {
				try {
					linkDeviceThread.join();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				linkDeviceThread = null;
			}
		}
		
		@Override
		public void run() {
			while (isRun) {
				try {
					sleep(1000);
					if (listSize > 0) {
						linkDevice();
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
		if ((sharedList == null) || (sharedList.size() <= 0)) {
			return ;
		}
		
		int size = sharedList.size();
		if (size > 0) {
			for (int i=0; i<size; i++) {
				if (sharedList.get(i).get("isOnline").equals("true")) {
					String peerId = sharedList.get(i).get("dealerName");
					String linkState = sharedList.get(i).get("LinkState");
					
					if (linkState.equals("notlink")) {
						if (TunnelCommunication.getInstance().IsTunnelOpened(peerId)) {
							// 已联机
							sharedList.get(i).put("LinkState", "linked");
						} else {
							if (TunnelCommunication.getInstance().openTunnel(peerId) == 0) {
								// 正在联机...
								sharedList.get(i).put("LinkState", "linking");
							} else {
								sharedList.get(i).put("LinkState", "notlink");
							}
						}
					}
					else if (linkState.equals("timeout")) {
						if (TunnelCommunication.getInstance().IsTunnelOpened(peerId)) {
							// 已联机
							sharedList.get(i).put("LinkState", "linked");
						} else {
							// 发送超时正在联机延时消息
							if (!handler.hasMessages(LINK_TIMEOUT)) {
								MainApplication.getInstance().sendHandlerMsg(handler, LINK_TIMEOUT, i, 0, peerId, 3000);
							}
						}
					}
					// 发送更新设备列表状态的广播
					sendHandlerMsg(REFRESH_DEVICE_LIST, null);
				} else {
					continue;
				}
			}
		}
	}
	
	/**
	 * 发送Handler消息
	 */
	private void sendHandlerMsg(int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	private void sendHandlerMsg(int what, String obj, int timeout) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessageDelayed(msg, timeout);
	}
	private void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		if (handler != null) {
			handler.sendMessage(msg);
		}
	}
	
	/**
	 * 请求终端分享列表的网络操作
	 */
	public void reqTermListEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			String data = generateReqTermListJson();
			sendHandlerMsg(IS_REQUESTING, "正在请求分享列表...");
			sendHandlerMsg(REQUEST_TIMEOUT, "请求分享列表失败，网络超时！", Value.REQ_TIME_10S);
			sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	/**
	 * 删除终端分享的网络操作
	 */
	public void delTermItemEvent(String id) {
		if (Utils.isNetworkAvailable(mContext)) {
			String data = generateDelShareTermItemJson(id);
			sendHandlerMsg(IS_REQUESTING, "正在删除终端分享...");
			sendHandlerMsg(REQUEST_TIMEOUT, "删除终端分享失败，网络超时！", Value.REQ_TIME_10S);
			sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_shared_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.up_out);
				break;
		}
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		unregisterReceiver(serviceReceiver);
		Value.isSharedUser = false;
		if (linkDeviceThread != null) {
			linkDeviceThread.stopThread();
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			Value.isSharedUser = false;
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.up_out);
		}
		return super.onKeyDown(keyCode, event);
	}
	
	/**
	 * 设备项ListView的点击事件
	 */
	private class OnItemClickListenerImpl implements OnItemClickListener {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, final int position, long id) {
			// TODO Auto-generated method stub
			listPosition = position;
			HashMap<String, String> item = sharedList.get(listPosition);
			mDeviceName = item.get("deviceName");
			mDeviceId = item.get("deviceID");
			
			if (Utils.isNetworkAvailable(mContext)) {
				if (Utils.getOnlineState(sharedList.get(position).get("isOnline"))) {
					
					if (Value.isLoginSuccess) {
						//读取流量保护开关设置
						boolean isProtectTraffic = true;
						PreferData preferData = new PreferData(mContext);
						if (preferData.isExist("ProtectTraffic")) {
							isProtectTraffic = preferData.readBoolean("ProtectTraffic");
						}
						
						if (item.get("LinkState").equals("linked")) {
							if (!isProtectTraffic) {
								//实时视频
								Intent intent = new Intent(mContext, PlayerActivity.class);
								intent.putExtra("deviceName", mDeviceName);
								intent.putExtra("dealerName", sharedList.get(position).get("dealerName"));
								mContext.startActivity(intent);
							} else {
								if (Utils.isWiFiNetwork(mContext)) {
									//实时视频
									Intent intent = new Intent(mContext, PlayerActivity.class);
									intent.putExtra("deviceName", mDeviceName);
									intent.putExtra("dealerName", sharedList.get(position).get("dealerName"));
									mContext.startActivity(intent);
								} else {
									final OkCancelDialog myDialog=new OkCancelDialog(mContext);
									myDialog.setTitle("温馨提示");
									myDialog.setMessage("当前网络不是WiFi，继续观看视频？");
									myDialog.setPositiveButton("确认", new OnClickListener() {
										@Override
										public void onClick(View v) {
											myDialog.dismiss();
											//实时视频
											Intent intent = new Intent(mContext, PlayerActivity.class);
											intent.putExtra("deviceName", mDeviceName);
											intent.putExtra("dealerName", sharedList.get(position).get("dealerName"));
											mContext.startActivity(intent);
										}
									});
									myDialog.setNegativeButton("取消", new OnClickListener() {
										@Override
										public void onClick(View v) {
											myDialog.dismiss();
										}
									});
								}
							}
						} else {
							Toast.makeText(mContext, "未联机，无法请求视频！", Toast.LENGTH_SHORT).show();
						}
					} else {
						final OkOnlyDialog myDialog=new OkOnlyDialog(mContext);
						myDialog.setTitle("温馨提示");
						myDialog.setMessage("网络不稳定，请重新登录！");
						myDialog.setPositiveButton("确认", new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog.dismiss();
								if (Value.beatHeartFailFlag) {
									Value.beatHeartFailFlag = false;
								}
								//登录界面
								mContext.startActivity(new Intent(mContext, LoginActivity.class));
							}
						});
					}
				} else {
					Toast.makeText(mContext, "【"+mDeviceName+"】终端设备不在线！", Toast.LENGTH_SHORT).show();
				}
			} else {
				Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
			}
		}
	}
	
	/**
	 * 设备项ListView的长点击事件
	 */
	private class OnItemLongClickListenerImpl implements OnItemLongClickListener {
		@Override
		public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
			// TODO Auto-generated method stub
			listPosition = position;
			showPopupWindow(lv_list);
			return false;
		}
	}
	
	/**
	 * 通过dealerName从设备列表获得指定设备的position
	 */
	public int getDeviceListPositionByDealerName(String dealerName) {
		String[] sArray = dealerName.split("-");
		int size = sharedList.size();
		for (int i=0; i<size; i++) {
			if (sharedList.get(i).get("deviceID").equalsIgnoreCase(sArray[0].trim())) {
				return i;
			}
		}
		return -1;
	}
	
	/**
	 * 通过deviceID从设备列表获得指定设备的position
	 */
	public int getDeviceListPositionByDeviceID(String deviceID) {
		int size = sharedList.size();
		for (int i=0; i<size; i++) {
			if (sharedList.get(i).get("deviceID").equals(deviceID)) {
				return i;
			}
		}
		return -1;
	}
	
	private BroadcastReceiver serviceReceiver = new BroadcastReceiver() {
		
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (action.equals(BackstageService.TUNNEL_REQUEST_ACTION)) {
				// 联机的4种状态：linked:已联机 notlink:无法联机 linking:正在联机... timeout:联机超时
				int TunnelEvent = intent.getIntExtra("TunnelEvent", 1);
				String peerId = intent.getStringExtra("PeerId");
				int position = getDeviceListPositionByDealerName(peerId);
				if (position == -1) {
					return ;
				}
				switch (TunnelEvent) {
					// 通道被打开
					case 0:
						// 已联机
						sharedList.get(position).put("LinkState", "linked");
						break;
					// 通道被关闭
					case 1:
						// 联机超时
						sharedList.get(position).put("LinkState", "timeout");
						break;
				}
				sendHandlerMsg(REFRESH_DEVICE_LIST, null);
			}
			else if (action.equals(BackstageService.TERM_ONLINE_STATE_ACTION)) {
				// 终端上下线
				String mac = intent.getStringExtra("deviceID");
				int position = getDeviceListPositionByDeviceID(mac);
				if (position == -1) {
					return ;
				}
				String dealerName = intent.getStringExtra("dealerName");
				String isOnline = intent.getStringExtra("isOnline");
				
				sharedList.get(position).put("dealerName", dealerName);
				sharedList.get(position).put("isOnline", isOnline);
				if (!isOnline.equals("true")) {
					sharedList.get(position).put("LinkState", "notlink");
				}
				sendHandlerMsg(REFRESH_DEVICE_LIST, null);
			}
		}
	};

	/**
	 * 设备项ListView的长按键的PopupWindow选项
	 */
	public void showPopupWindow(View view) {
		LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View pop_view = inflater.inflate(R.layout.pop_event_main, null);
		ListView pop_listView = (ListView)pop_view.findViewById(R.id.pop_list);
		
		List<String> item_list = new ArrayList<String>();
		item_list.add("删除终端分享");
		PopupWindowAdapter popAdapter = new PopupWindowAdapter(mContext, item_list);
		pop_listView.setAdapter(popAdapter);
		
		mPopupWindow = new PopupWindow(pop_view, Utils.screenWidth, 200, true);
		mPopupWindow.setHeight(LayoutParams.WRAP_CONTENT); 
		mPopupWindow.setBackgroundDrawable(new BitmapDrawable());
		mPopupWindow.setOutsideTouchable(true);
		
		mPopupWindow.setAnimationStyle(R.style.PopupAnimationBottom);
		mPopupWindow.showAtLocation(view, Gravity.BOTTOM, 0, 0);
		mPopupWindow.update();

		pop_listView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				HashMap<String, String> item = sharedList.get(listPosition);
				mDeviceId = item.get("deviceID");
				switch (position) {
					case 0:
						delTermItemEvent(mDeviceId);
						break;
				}
				if (mPopupWindow.isShowing()) {
					mPopupWindow.dismiss();
				}
			}
		});
	}
}
