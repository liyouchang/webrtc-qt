package com.video.main;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Environment;
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
import com.video.data.XmlShare;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqHandler;
import com.video.utils.DeviceItemAdapter;
import com.video.utils.PopupWindowAdapter;
import com.video.utils.Utils;

public class SharedActivity extends Activity implements OnClickListener {
	
	private Context mContext;
	private static XmlShare xmlShare;
	private PreferData preferData = null;
	private String userName = null;
	//终端列表项
	private static String mDeviceId = null;
	private static int listPosition = 0;
	private static int listSize = 0;
	private RelativeLayout noDeviceLayout = null;
	
	private PopupWindow mPopupWindow;
	private Dialog mDialog = null;
	
	private String thumbnailsPath = null;
	private File thumbnailsFile = null;
	private static ArrayList<HashMap<String, String>> sharedList = null;
	private static DeviceItemAdapter deviceAdapter = null;
	private ListView lv_list;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	
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
		lv_list.setOnItemLongClickListener(new OnItemLongClickListenerImpl());
	}
	
	private void initData() {
		//初始化Activity要使用的参数
		mContext = SharedActivity.this;
		ZmqHandler.setHandler(handler);
		xmlShare = new XmlShare(mContext);
		preferData = new PreferData(mContext);
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		
		String SD_path = Environment.getExternalStorageDirectory().getAbsolutePath();
		thumbnailsPath = SD_path + File.separator + "KaerVideo" + File.separator + "thumbnails";
		thumbnailsFile = new File(thumbnailsPath);
		if(!thumbnailsFile.exists()){
			thumbnailsFile.mkdirs();
		}
		
		//初始化终端列表的显示
		reqTermListEvent();
		sharedList = xmlShare.readXml();
		if (sharedList != null) {
			listSize = sharedList.size();
			deviceAdapter = new DeviceItemAdapter(mContext, thumbnailsFile, sharedList);
			lv_list.setAdapter(deviceAdapter);
			if (listSize == 0) {
				noDeviceLayout.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.INVISIBLE);
			}
		} else {
			listSize = xmlShare.getListSize();
			if (listSize == 0) {
				noDeviceLayout.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.INVISIBLE);
			}
		}
	}
	
	/**
	 * 生成JSON的请求分享列表字符串
	 */
	private String generateReqTermListJson() {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ReqShareList");
			jsonObj.put("UserName", userName);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 生成JSON的删除终端分享字符串
	 */
	private String generateDelShareTermItemJson(String mac) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelShareTerm");
			jsonObj.put("UserName", userName);
			jsonObj.put("MAC", mac);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	private Handler handler = new Handler() {

		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUESTING:
					mDialog = Utils.createLoadingDialog(mContext, (String) msg.obj);
					mDialog.show();
					break;
				case REQUEST_TIMEOUT:
					if (mDialog != null) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					Toast.makeText(mContext, ""+msg.obj, Toast.LENGTH_SHORT).show();
					break;
				//请求终端列表
				case R.id.request_device_share_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog != null) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							sharedList = (ArrayList<HashMap<String, String>>) msg.obj;
							if (sharedList != null) {
								xmlShare.updateList(sharedList);
								deviceAdapter = new DeviceItemAdapter(mContext, thumbnailsFile, sharedList);
								lv_list.setAdapter(deviceAdapter);
								listSize = sharedList.size();
							}
						} else {
							Toast.makeText(mContext, msg.obj+"，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.request_device_share_id);
					}
					break;
				//删除终端绑定
				case R.id.delete_device_share_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog != null) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							xmlShare.deleteItem(mDeviceId);
							sharedList.remove(listPosition);
							deviceAdapter.notifyDataSetChanged();
							listSize = xmlShare.getListSize();
							Toast.makeText(mContext, "删除终端分享成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "删除终端分享失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_device_share_id);
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
	
	public static Handler sharedHandler = new Handler() {
		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			if (msg.what == 0) {
				HashMap<String, String> item = (HashMap<String, String>)msg.obj;
				String mac = item.get("deviceID");
				for (int i=0; i<listSize; i++) {
					if (sharedList.get(i).get("deviceID").equals(mac)) {
						item.put("deviceName", sharedList.get(i).get("deviceName"));
						sharedList.get(i).put("isOnline", item.get("isOnline"));
						sharedList.get(i).put("dealerName", item.get("dealerName"));
						break;
					}
				}
				deviceAdapter.notifyDataSetChanged();
				xmlShare.updateItemState(mac, item.get("isOnline"), item.get("dealerName"));
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
		if (handler != null) {
			handler.sendMessage(msg);
		}
	}
	
	/**
	 * 请求终端分享列表的网络操作
	 */
	public void reqTermListEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
			String data = generateReqTermListJson();
			sendHandlerMsg(IS_REQUESTING, "正在请求分享列表...");
			sendHandlerMsg(REQUEST_TIMEOUT, "请求分享列表失败，网络超时！", Value.requestTimeout);
			sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	/**
	 * 删除终端分享的网络操作
	 */
	public void delTermItemEvent(String id) {
		if (Utils.isNetworkAvailable(mContext)) {
			Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
			String data = generateDelShareTermItemJson(id);
			sendHandlerMsg(IS_REQUESTING, "正在删除终端分享...");
			sendHandlerMsg(REQUEST_TIMEOUT, "删除终端分享失败，网络超时！", Value.requestTimeout);
			sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
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
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.up_out);
		}
		return super.onKeyDown(keyCode, event);
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
