package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.annotation.SuppressLint;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.BitmapDrawable;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager.LayoutParams;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
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
import com.video.data.XmlDevice;
import com.video.main.PullToRefreshHeaderView.OnHeaderRefreshListener;
import com.video.play.PlayerActivity;
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

@SuppressLint("HandlerLeak")
public class OwnFragment extends Fragment implements OnClickListener, OnHeaderRefreshListener {
	
	private View rootView;
	private static FragmentActivity mActivity;
	private PreferData preferData = null;
	private String userName = null;
	//终端列表项
	private static String mDeviceName = null;
	private static String mDeviceId = null;
	private static String mDeviceBg = null;
	private static int listPosition = 0;
	private static int listSize = 0;
	private RelativeLayout noDeviceLayout = null;
	
	private ImageButton button_add;
	private PopupWindow mPopupWindow;
	private Dialog mDialog = null;
	
	private static DeviceItemAdapter deviceAdapter = null;
	private static ListView lv_list;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	private final int REFRESH_DEVICE_LIST = 3;
	
	private OwnReceiver ownReceiver = null;
	
	private PullToRefreshHeaderView mPullToRefreshHeaderView;
	private String own_refresh_time = null;
	private String own_refresh_terminal = null;
	private Boolean isPullToRefresh = false;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		if (rootView == null) {
			rootView = inflater.inflate(R.layout.own, null);
		}
		container = (ViewGroup) rootView.getParent();
		if (container != null) {
			container.removeView(rootView);
		}
		return rootView;
	}
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		mActivity = getActivity();
		initView();
		// 注册广播
		ownReceiver = new OwnReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(BackstageService.CHANGE_DEVICE_LIST_ACTION);
		mActivity.registerReceiver(ownReceiver, filter);
	}
	
	@Override
	public void onStart() {
		// TODO Auto-generated method stub
		super.onStart();
		initData();
	}

	@Override
	public void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		if (MainActivity.isCurrentTab(MainActivity.TAB_ONE)) {
			ZmqHandler.mHandler = deviceHandler;
		}
	}
	
	private void initView () {
		button_add = (ImageButton)rootView.findViewById(R.id.btn_add_device);
		button_add.setOnClickListener(this);
		
		noDeviceLayout = (RelativeLayout) rootView.findViewById(R.id.rl_no_device_list);
		
		lv_list = (ListView) rootView.findViewById(R.id.device_list);
		lv_list.setOnItemClickListener(new OnItemClickListenerImpl());
		lv_list.setOnItemLongClickListener(new OnItemLongClickListenerImpl());
		
		mPullToRefreshHeaderView = (PullToRefreshHeaderView) rootView.findViewById(R.id.own_pull_refresh_view);
		mPullToRefreshHeaderView.setOnHeaderRefreshListener(this);
	}
	
	private void initData() {
		// 初始化Activity要使用的参数
		preferData = new PreferData(mActivity);
		ZmqHandler.mHandler = deviceHandler;
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		
		//初始化下拉刷新的显示
		if (preferData.isExist("ownRefreshTime")) {
			own_refresh_time = preferData.readString("ownRefreshTime");
		}
		if (preferData.isExist("ownRefreshTerminal")) {
			own_refresh_terminal = preferData.readString("ownRefreshTerminal");
		}
		if ((own_refresh_time != null) && (own_refresh_terminal != null)) {
			mPullToRefreshHeaderView.onHeaderRefreshComplete(own_refresh_time, own_refresh_terminal);
		}
		
		if (MainApplication.getInstance().deviceList == null) {
			MainApplication.getInstance().deviceList = new ArrayList<HashMap<String, String>>();
			listSize = 0;
		} else {
			listSize = MainApplication.getInstance().deviceList.size();
		}
		
		if (listSize > 1) {
			MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
		}
		deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
		lv_list.setAdapter(deviceAdapter);
		
		if ((Value.isNeedReqTermListFlag) && (MainActivity.isCurrentTab(MainActivity.TAB_ONE))) {
			// 从服务器请求设备列表
			reqTermListEvent();
		} else {
			if (listSize > 0) {
				noDeviceLayout.setVisibility(View.GONE);
				mPullToRefreshHeaderView.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.VISIBLE);
				mPullToRefreshHeaderView.setVisibility(View.GONE);
			}
		}
	}
	
	/**
	 * 下拉刷新
	 * yyyy-MM-dd hh:mm:ss 12小时制  yyyy-MM-dd HH:mm:ss 24小时制
	 */
	@Override
	public void onHeaderRefresh(PullToRefreshHeaderView view) {
		// TODO Auto-generated method stub
		mPullToRefreshHeaderView.postDelayed(new Runnable() {
			@Override
			public void run() {
				isPullToRefresh = true;
				ZmqHandler.mHandler = deviceHandler;
				// 从服务器请求设备列表
				reqTermListEvent();
			}
		}, 500);
	}
	
	/**
	 * 生成JSON的请求设备列表字符串
	 */
	private String generateReqTermListJson() {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ReqTermList");
			jsonObj.put("UserName", userName);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 请求报警消息事件
	 */
	public void requestAlarmEvent() {
		Value.ownFragmentRequestAlarmFlag = true;
		String data = MainApplication.getInstance().generateRequestAlarmJson(0, 5);
		sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, data);
	}
	
	/**
	 * 将终端列表的设备排序，在线在前，不在线在后
	 */
	private static ArrayList<HashMap<String, String>> orderDeviceList(ArrayList<HashMap<String, String>> list) {
		int len = list.size();
		ArrayList<HashMap<String, String>> listObj = new ArrayList<HashMap<String, String>>();
		
		//在线设备
		for (int i=0; i<len; i++) {
			HashMap<String, String> item = list.get(i);
			if (item.get("isOnline").equals("true")) {
				listObj.add(item);
			}
		}
		//不在线设备
		for (int i=0; i<len; i++) {
			HashMap<String, String> item = list.get(i);
			if (!item.get("isOnline").equals("true")) {
				listObj.add(item);
			}
		}
		return listObj;
	}
	
	/**
	 * 发送Handler消息
	 */
	public void sendHandlerMsg(Handler handler, int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, String obj, int timeout) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessageDelayed(msg, timeout);
	}
	public void sendHandlerMsg(Handler handler, int what, int arg1, int arg2, Object obj) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	private Handler deviceHandler = new Handler() {
		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUESTING:
					if ((mDialog == null) || (!mDialog.isShowing())) {
						mDialog = Utils.createLoadingDialog(mActivity, (String) msg.obj);
						mDialog.show();
					}
					break;
				case REQUEST_TIMEOUT:
					if ((mDialog != null) && (mDialog.isShowing())) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (deviceHandler.hasMessages(REQUEST_TIMEOUT)) {
						deviceHandler.removeMessages(REQUEST_TIMEOUT);
					}
					if (mPullToRefreshHeaderView.getHeaderState() == PullToRefreshView.REFRESHING) {
						mPullToRefreshHeaderView.onHeaderRefreshComplete();
					}
					isPullToRefresh = false;
					Value.isNeedReqTermListFlag = true;
					Toast.makeText(mActivity, ""+msg.obj, Toast.LENGTH_SHORT).show();
					if (listSize > 1) {
						MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
						deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
						lv_list.setAdapter(deviceAdapter);
					} else {
						deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
						lv_list.setAdapter(deviceAdapter);
					}
					break;
				//请求终端列表
				case R.id.request_terminal_list_id:
					if (deviceHandler.hasMessages(REQUEST_TIMEOUT)) {
						deviceHandler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							//请求终端列表成功
							Value.isNeedReqTermListFlag = false;
							if (mPullToRefreshHeaderView.getHeaderState() == PullToRefreshView.REFRESHING) {
								own_refresh_time = getResources().getString(R.string.last_Updated_on)+":"+Utils.getNowTime("yyyy-MM-dd HH:mm:ss");
								own_refresh_terminal = getResources().getString(R.string.terminal)+":"+Build.MODEL;
								preferData.writeData("ownRefreshTime", own_refresh_time);
								preferData.writeData("ownRefreshTerminal", own_refresh_terminal);
								mPullToRefreshHeaderView.onHeaderRefreshComplete(own_refresh_time, own_refresh_terminal);
							}
							ArrayList<HashMap<String, String>> listObj = (ArrayList<HashMap<String, String>>) msg.obj;
							if (listObj != null) {
								MainApplication.getInstance().xmlDevice.updateList(listObj);
								MainApplication.getInstance().deviceList = listObj;
								listSize = MainApplication.getInstance().getDeviceListSize();
								
								if (listSize > 1) {
									MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
								}
								deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
								lv_list.setAdapter(deviceAdapter);
								
							} else {
								listSize = 0;
								MainApplication.getInstance().xmlDevice.deleteAllItem();
								if (MainApplication.getInstance().deviceList != null) {
									MainApplication.getInstance().deviceList.clear();
									deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
									lv_list.setAdapter(deviceAdapter);
								}
							}
						} else {
							// 请求终端列表失败
							if (mPullToRefreshHeaderView.getHeaderState() == PullToRefreshView.REFRESHING) {
								mPullToRefreshHeaderView.onHeaderRefreshComplete();
							}
							Toast.makeText(mActivity, msg.obj+"，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
						if (!isPullToRefresh) {
							//请求报警数据
							requestAlarmEvent();
						} else {
							isPullToRefresh = false;
						}
					} else {
						deviceHandler.removeMessages(R.id.request_terminal_list_id);
					}
					break;
				// 刷新终端列表图片
				case REFRESH_DEVICE_LIST:
					if (deviceAdapter != null) {
						if (msg.obj != null) {
							listSize = MainApplication.getInstance().getDeviceListSize();
							if (listSize > 1) {
								MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
								deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
								lv_list.setAdapter(deviceAdapter);
							} else {
								deviceAdapter.notifyDataSetChanged();
							}
						} else {
							deviceAdapter.notifyDataSetChanged();
						}
					}
					break;
			}
			listSize = MainApplication.getInstance().getDeviceListSize();
			if (listSize > 0) {
				noDeviceLayout.setVisibility(View.GONE);
				mPullToRefreshHeaderView.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.VISIBLE);
				mPullToRefreshHeaderView.setVisibility(View.GONE);
			}
		}
	};
	
	/**
	 * 请求终端列表的网络操作
	 */
	public void reqTermListEvent() {
		if (Value.isLoginSuccess) {
			String data = generateReqTermListJson();
			if (!isPullToRefresh) {
//				sendHandlerMsg(deviceHandler, IS_REQUESTING, "正在请求终端列表...");
			}
			sendHandlerMsg(deviceHandler, REQUEST_TIMEOUT,getResources().getString(R.string.requesting_terminal_list_failed), Value.REQ_TIME_10S);
			sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
		} else {
			if (Utils.isNetworkAvailable(mActivity)) {
				final OkOnlyDialog myDialog=new OkOnlyDialog(mActivity);
				myDialog.setTitle(getResources().getString(R.string.tips));
				myDialog.setMessage(getResources().getString(R.string.network_instability));
				myDialog.setPositiveButton(getResources().getString(R.string.confirm), new OnClickListener() {
					@Override
					public void onClick(View v) {
						myDialog.dismiss();
						// 发送注销数据
						MainApplication.getInstance().sendLogoutData();
						
						// 终止主程序和服务广播
						MainApplication.getInstance().stopActivityandService();
						
						//登录界面
						mActivity.startActivity(new Intent(mActivity, LoginActivity.class));
						mActivity.finish();
					}
				});
			} else {
				Toast.makeText(mActivity, getResources().getString(R.string.no_available_network_connection), Toast.LENGTH_SHORT).show();
			}
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_add_device:
				startActivityForResult(new Intent(mActivity, AddDeviceActivity.class), 0);
				mActivity.overridePendingTransition(R.anim.up_in, R.anim.fragment_nochange);
				break;
		}
	}
	
	/**
	 * 设备项ListView的点击事件
	 */
	private class OnItemClickListenerImpl implements OnItemClickListener {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, final int position, long id) {
			// TODO Auto-generated method stub
			listPosition = position;
			HashMap<String, String> item = MainApplication.getInstance().deviceList.get(listPosition);
			mDeviceName = item.get("deviceName");
			mDeviceId = item.get("deviceID");
			mDeviceBg = item.get("deviceBg");
			
			if (Utils.isNetworkAvailable(mActivity)) {
				
				if (Utils.getOnlineState(MainApplication.getInstance().deviceList.get(position).get("isOnline"))) {
					
					if (Value.isLoginSuccess) {
						//读取流量保护开关设置
						boolean isProtectTraffic = true;
						if (preferData.isExist("ProtectTraffic")) {
							isProtectTraffic = preferData.readBoolean("ProtectTraffic");
						}
						
						if (MainApplication.getInstance().deviceList.get(position).get("LinkState").equals("linked")) {
							if (!isProtectTraffic) {
								//实时视频
								Intent intent = new Intent(mActivity, PlayerActivity.class);
								intent.putExtra("deviceID", mDeviceId);
								intent.putExtra("deviceName", mDeviceName);
								intent.putExtra("dealerName", MainApplication.getInstance().deviceList.get(position).get("dealerName"));
								intent.putExtra("playerClarity", MainApplication.getInstance().deviceList.get(position).get("playerClarity"));
								mActivity.startActivity(intent);
							} else {
								if (Utils.isWiFiNetwork(mActivity)) {
									//实时视频
									Intent intent = new Intent(mActivity, PlayerActivity.class);
									intent.putExtra("deviceID", mDeviceId);
									intent.putExtra("deviceName", mDeviceName);
									intent.putExtra("dealerName", MainApplication.getInstance().deviceList.get(position).get("dealerName"));
									intent.putExtra("playerClarity", MainApplication.getInstance().deviceList.get(position).get("playerClarity"));
									mActivity.startActivity(intent);
								} else {
									final OkCancelDialog myDialog=new OkCancelDialog(mActivity);
									myDialog.setTitle("");
									myDialog.setMessage(getResources().getString(R.string.the_current_network_is_not_WiFi));
									myDialog.setPositiveButton(getResources().getString(R.string.confirm), new OnClickListener() {
										@Override
										public void onClick(View v) {
											myDialog.dismiss();
											//实时视频
											Intent intent = new Intent(mActivity, PlayerActivity.class);
											intent.putExtra("deviceID", mDeviceId);
											intent.putExtra("deviceName", mDeviceName);
											intent.putExtra("dealerName", MainApplication.getInstance().deviceList.get(position).get("dealerName"));
											intent.putExtra("playerClarity", MainApplication.getInstance().deviceList.get(position).get("playerClarity"));
											mActivity.startActivity(intent);
										}
									});
									myDialog.setNegativeButton(getResources().getString(R.string.cancel), new OnClickListener() {
										@Override
										public void onClick(View v) {
											myDialog.dismiss();
										}
									});
								}
							}
						} else {
							Toast.makeText(mActivity, getResources().getString(R.string.not_online), Toast.LENGTH_SHORT).show();
						}
					} else {
						final OkOnlyDialog myDialog=new OkOnlyDialog(mActivity);
						myDialog.setTitle(getResources().getString(R.string.tips));
						myDialog.setMessage(getResources().getString(R.string.no_available_network_connection));
						myDialog.setPositiveButton(getResources().getString(R.string.confirm), new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog.dismiss();
								// 发送注销数据
								MainApplication.getInstance().sendLogoutData();
								
								// 终止主程序和服务广播
								MainApplication.getInstance().stopActivityandService();
								 
								//登录界面
								mActivity.startActivity(new Intent(mActivity, LoginActivity.class));
								mActivity.finish();
							}
						});
					}
				} else {
					Toast.makeText(mActivity,getResources().getString(R.string.not_online), Toast.LENGTH_SHORT).show();
				}
			} else {
				Toast.makeText(mActivity, getResources().getString(R.string.no_available_network_connection), Toast.LENGTH_SHORT).show();
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
			return true;
		}
	}

	/**
	 * 设备项ListView的长按键的PopupWindow选项
	 */
	public void showPopupWindow(View view) {
		LayoutInflater inflater = (LayoutInflater) mActivity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View pop_view = inflater.inflate(R.layout.pop_event_main, null);
		ListView pop_listView = (ListView)pop_view.findViewById(R.id.pop_list);
		
		List<String> item_list = new ArrayList<String>();
		item_list.add(getResources().getString(R.string.device_Management));
		item_list.add(getResources().getString(R.string.remote_video));
		PopupWindowAdapter popAdapter = new PopupWindowAdapter(mActivity, item_list);
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
				ZmqHandler.mHandler = deviceHandler;
				HashMap<String, String> item = MainApplication.getInstance().deviceList.get(listPosition);
				mDeviceName = item.get("deviceName");
				mDeviceId = item.get("deviceID");
				mDeviceBg = item.get("deviceBg");
				String dealerName = item.get("dealerName");
				Intent intent = null;
				switch (position) {
					case 0: //设备管理
						intent = new Intent(mActivity, DeviceManagerActivity.class);
						intent.putExtra("deviceName", mDeviceName);
						intent.putExtra("deviceID", mDeviceId);
						intent.putExtra("deviceBg", mDeviceBg);
						intent.putExtra("dealerName", dealerName);
						startActivityForResult(intent, 0);
						mActivity.overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
						break;
					case 1: //远程录像
						if (item.get("LinkState").equals("linked")) {
							intent = new Intent(mActivity, SetDateActivity.class);
							intent.putExtra("deviceName", mDeviceName);
							intent.putExtra("deviceID", mDeviceId);
							intent.putExtra("dealerName", dealerName);
							startActivity(intent);
						} else {
							Toast.makeText(mActivity, getResources().getString(R.string.not_online), Toast.LENGTH_SHORT).show();
						}
						break;
				}
				if (mPopupWindow.isShowing()) {
					mPopupWindow.dismiss();
				}
			}
		});
	}
	
	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		super.onActivityResult(requestCode, resultCode, data);
		if (resultCode == 1) {
			// 刷新终端设备列表
			XmlDevice xmlDevice = new XmlDevice(mActivity);
			ArrayList<HashMap<String, String>> xmlList = xmlDevice.readXml();
			if (xmlList != null) {
				MainApplication.getInstance().deviceList = xmlList;
			}
			sendHandlerMsg(deviceHandler, REFRESH_DEVICE_LIST, 0, 0, null);
		} 
		else if (resultCode == 2) {
			// 添加终端设备
			listSize = MainApplication.getInstance().xmlDevice.getListSize();
			
			if (listSize > 1) {
				MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
			}
			deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
			lv_list.setAdapter(deviceAdapter);
			
			if (listSize > 0) {
				noDeviceLayout.setVisibility(View.GONE);
				mPullToRefreshHeaderView.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.VISIBLE);
				mPullToRefreshHeaderView.setVisibility(View.GONE);
			}
		}
	}
	
	@Override
	public void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		if ((mDialog != null) && (mDialog.isShowing())) {
			mDialog.dismiss();
			mDialog = null;
		}
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		// 注销广播
		if (ownReceiver != null) {
			mActivity.unregisterReceiver(ownReceiver);
		}
	}

	public class OwnReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			String action = intent.getAction();
			if (action.equals(BackstageService.CHANGE_DEVICE_LIST_ACTION)) {
				if (intent.hasExtra("isTermActive")) {
					boolean isTermActive = intent.getBooleanExtra("isTermActive", false);
					if (isTermActive) {
						sendHandlerMsg(deviceHandler, REFRESH_DEVICE_LIST, 0, 0, isTermActive);
					} else {
						sendHandlerMsg(deviceHandler, REFRESH_DEVICE_LIST, 0, 0, null);
					}
				} else {
					sendHandlerMsg(deviceHandler, REFRESH_DEVICE_LIST, 0, 0, null);
				}
			}
		}
	}
}
