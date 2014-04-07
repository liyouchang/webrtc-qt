package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
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
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.data.XmlData;
import com.video.main.PullToRefreshView.OnFooterRefreshListener;
import com.video.main.PullToRefreshView.OnHeaderRefreshListener;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.DeviceItemAdapter;
import com.video.utils.PopupWindowAdapter;
import com.video.utils.Utils;

public class OwnFragment extends Fragment implements OnClickListener, OnHeaderRefreshListener, OnFooterRefreshListener {
	
	private FragmentActivity mActivity;
	private View mView;
	private XmlData xmlData;
	private PreferData preferData = null;
	private String userName = null;
	private String list_refresh_time = null;
	private String list_refresh_terminal = null;
	//修改终端名称
	private String mDeviceName = null;
	private String mDeviceId = null;
	
	private ImageButton button_add;
	private PopupWindow mPopupWindow;
	private ProgressDialog progressDialog;
	
	private static ArrayList<HashMap<String, String>> deviceList = null;
	private DeviceItemAdapter deviceAdapter = null;
	private ListView lv_list;
	private PullToRefreshView mPullToRefreshView;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		return inflater.inflate(R.layout.own, container, false);
	}
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		mActivity = getActivity();
		mView = getView();
		
		initView();
		initData();
	}
	
	private void initView () {
		button_add = (ImageButton)mView.findViewById(R.id.btn_add_device);
		button_add.setOnClickListener(this);
		
		lv_list = (ListView) mView.findViewById(R.id.device_list);
		lv_list.setOnItemLongClickListener(new OnItemLongClickListenerImpl());
		mPullToRefreshView = (PullToRefreshView) mView.findViewById(R.id.main_pull_refresh_view);
		mPullToRefreshView.setOnHeaderRefreshListener(this);
        mPullToRefreshView.setOnFooterRefreshListener(this);
	}
	
	private void initData() {
		//初始化Activity要使用的参数
		ZmqHandler.setHandler(handler);
		xmlData = new XmlData(mActivity);
		preferData = new PreferData(mActivity);
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		//初始化下拉刷新的显示
		if (preferData.isExist("listRefreshTime")) {
			list_refresh_time = preferData.readString("listRefreshTime");
		}
		if (preferData.isExist("listRefreshTerminal")) {
			list_refresh_terminal = preferData.readString("listRefreshTerminal");
		}
		if ((list_refresh_time != null) && (list_refresh_terminal != null)) {
			mPullToRefreshView.onHeaderRefreshComplete(list_refresh_time, list_refresh_terminal);
		}
		//初始化终端列表的显示
		if (Value.isNeedReqTermListFlag) {
			reqTermListEvent();
		}
		deviceList = xmlData.readXml();
		if (deviceList != null) {
			deviceAdapter = new DeviceItemAdapter(mActivity, deviceList);
			lv_list.setAdapter(deviceAdapter);
		}
	}
	
	/**
	 * 获得一个设备项Item
	 * @param isOnline 设备是否在线
	 * @param deviceName 设备名称
	 * @param deviceID 设备的MAC
	 * @return 返回一个设备项Item
	 */
	private HashMap<String, String> getDeviceItem(boolean isOnline, String deviceName, String deviceID, String dealerName) {
		
		HashMap<String, String> item = new HashMap<String, String>();
		String state = "false";
		if (isOnline) {
			state = "true";
		} else {
			state = "false";
		}
		item.put("isOnline", state);
		item.put("deviceName", deviceName);
		item.put("deviceID", deviceID);
		item.put("dealerName", dealerName);
		return item;
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
	 * 生成JSON的生成终端名字字符串
	 */
	private String generateModifyTermNameJson() {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ModifyTerm");
			jsonObj.put("TermName", mDeviceName);
			jsonObj.put("MAC", mDeviceId);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 显示操作的进度条
	 */
	private void showProgressDialog(String info) {
		progressDialog = new ProgressDialog(mActivity);
        progressDialog.setMessage(info); 
        progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);  
        progressDialog.setIndeterminate(false);     
        progressDialog.setCancelable(false); 
        progressDialog.show(); 
	}
	
	private Handler handler = new Handler() {

		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUESTING:
					showProgressDialog((String) msg.obj);
					break;
				case REQUEST_TIMEOUT:
					if (progressDialog != null)
						progressDialog.dismiss();
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					Value.isNeedReqTermListFlag = false;
					Toast.makeText(mActivity, msg.obj+"，网络超时！", Toast.LENGTH_SHORT).show();
					break;
				case R.id.request_terminal_list_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							if (progressDialog != null)
								progressDialog.dismiss();
							deviceList = (ArrayList<HashMap<String, String>>) msg.obj;
							if (deviceList != null) {
								Value.isNeedReqTermListFlag = false;
								deviceAdapter = new DeviceItemAdapter(mActivity, deviceList);
								lv_list.setAdapter(deviceAdapter);
								xmlData.updateList(deviceList);
							}
						} else {
							if (progressDialog != null)
								progressDialog.dismiss();
							Toast.makeText(mActivity, msg.obj+"，"+Utils.getErrorReason(resultCode)+"！", Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.request_terminal_list_id);
					}
					break;
			}
		}
	};
	
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
		handler.sendMessage(msg);
	}
	
	/**
	 * 请求终端列表的网络操作
	 */
	public void reqTermListEvent() {
		if (Utils.isNetworkAvailable(mActivity)) {
			Handler sendHandler = ZmqThread.zmqThreadHandler;
			String data = generateReqTermListJson();
			sendHandlerMsg(IS_REQUESTING, "正在请求终端列表...");
			sendHandlerMsg(REQUEST_TIMEOUT, "请求终端课表失败", Value.requestTimeout);
			sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mActivity, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_add_device:
				startActivity(new Intent(mActivity, AddDeviceActivity.class));
				break;
		}
	}
	
	/**
	 * 设备项ListView的长点击事件
	 */
	private class OnItemLongClickListenerImpl implements OnItemLongClickListener {
		@Override
		public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
			// TODO Auto-generated method stub
			showPopupWindow(mPullToRefreshView, position);
			return false;
		}
	}
	
	/**
	 * 上拖刷新
	 */
	@Override
	public void onFooterRefresh(PullToRefreshView view) {
		// TODO Auto-generated method stub
		mPullToRefreshView.postDelayed(new Runnable() {
			@Override
			public void run() {
				mPullToRefreshView.onFooterRefreshComplete();
			}
		}, 1000);
	}
	
	/**
	 * 下拉刷新
	 */
	@Override
	public void onHeaderRefresh(PullToRefreshView view) {
		mPullToRefreshView.postDelayed(new Runnable() {
			@Override
			public void run() {
				list_refresh_time = "上次更新于: "+Utils.getNowTime("yyyy-MM-dd hh:mm:ss");
				list_refresh_terminal = "终端: "+Build.MODEL;
				preferData.writeData("listRefreshTime", list_refresh_time);
				preferData.writeData("listRefreshTerminal", list_refresh_terminal);
				mPullToRefreshView.onHeaderRefreshComplete(list_refresh_time, list_refresh_terminal);
			}
		}, 1500);
	}

	/**
	 * 设备项ListView的长按键的PopupWindow选项
	 */
	@SuppressWarnings("deprecation")
	public void showPopupWindow(View view, int position) {
		LayoutInflater inflater = (LayoutInflater) mActivity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View pop_view = inflater.inflate(R.layout.pop_event_main, null);
		ListView pop_listView = (ListView)pop_view.findViewById(R.id.pop_list);
		
		List<String> item_list = new ArrayList<String>();
		item_list.add("修改终端名称");
		item_list.add("删除终端绑定");
		item_list.add("设置背景图片");
		item_list.add("删除背景图片");
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
				switch (position) {
					case 0:
						HashMap<String, String> item = deviceList.get(position);
						mDeviceName = item.get("deviceName");
						mDeviceId = item.get("deviceID");
						
						break;
					default : break;
				}
				if (mPopupWindow.isShowing()) {
					mPopupWindow.dismiss();
				}
			}
		});
	}
}
