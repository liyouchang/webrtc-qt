package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;
import org.json.JSONException;
import org.json.JSONObject;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import com.video.R;
import com.video.data.Value;
import com.video.service.MainApplication;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.OkCancelDialog;
import com.video.utils.Utils;
import com.video.utils.WiFiAlertDialog;

public class WiFiActivity extends Activity implements OnClickListener {

	private Context mContext;
	private ArrayList<HashMap<String, String>> onlineTermList = null;
	private Button button_wifi_term_select;
	public static String onlineDealerName = "";
	public static HashMap<String, Object> selectedWiFi = null;
	
	private Button btn_wifi_name;
	private EditText et_wifi_pwd;
	private Button button_delete_wifi_password;
	private Dialog mDialog = null;
	
	private final static int IS_REQUSTING = 1;
	private final static int REQUST_TIMEOUT = 2;
	private final static int IS_SETTING = 3;
	private final static int SET_TIMEOUT = 4;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.wifi);
		
		initView();
		initData();
	}

	private void initView() {
		ImageButton button_back = (ImageButton) super.findViewById(R.id.ib_wifi_back);
		button_back.setOnClickListener(this);
		
		button_wifi_term_select = (Button) super.findViewById(R.id.btn_wifi_term_list);
		button_wifi_term_select.setOnClickListener(this);
		
		btn_wifi_name = (Button) this.findViewById(R.id.btn_wifi_name);
		btn_wifi_name.setOnClickListener(this);
		
		btn_wifi_name = (Button) super.findViewById(R.id.btn_wifi_name);
		
		button_delete_wifi_password = (Button) this.findViewById(R.id.btn_wifi_password_del);
		button_delete_wifi_password.setOnClickListener(this);
		
		et_wifi_pwd = (EditText) super.findViewById(R.id.et_wifi_password);
		et_wifi_pwd.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_wifi_password.setVisibility(View.INVISIBLE);
				} else {
					button_delete_wifi_password.setVisibility(View.VISIBLE);
				}
			}
			
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
		});
		
		Button button_submit = (Button) super.findViewById(R.id.btn_wifi_submit);
		button_submit.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = WiFiActivity.this;
		ZmqHandler.mHandler = handler;
		onlineTermList = MainApplication.getInstance().getOnlineDeviceList();
		if ((onlineTermList != null) && (onlineTermList.size() > 0)) {
			button_wifi_term_select.setText((CharSequence) onlineTermList.get(0).get("deviceName"));
			onlineDealerName = ""+onlineTermList.get(0).get("dealerName");
		} else {
			button_wifi_term_select.setText("没有在线设备");
		}
	}
	
	/**
	 * 生成JSON请求WiFi列表信息字符串
	 */
	private String generateGetWiFiInfoJson() {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "tunnel");
			jsonObj.put("command", "wifi_info");
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 生成JSON配置设备WiFi网络字符串
	 */
	private String generateSetTermWiFiInfoJson(String key) {
		String result = "";
		if (key == null) {
			key = "";
		}
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "tunnel");
			jsonObj.put("command", "set_wifi");
			JSONObject subObj = new JSONObject();
				subObj.put("enable", 1); //1:打开WiFi  0:关闭WiFi
				if (selectedWiFi != null) {
					subObj.put("ssid", selectedWiFi.get("WiFiSSID"));
					subObj.put("key", key);
					subObj.put("auth", selectedWiFi.get("WiFAuth"));
					subObj.put("enc", selectedWiFi.get("WiFiEnc"));
					subObj.put("mode", selectedWiFi.get("WiFiMode"));
				}
			jsonObj.put("param", subObj);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 自定义Dialog
	 * @param context 上下文
	 * @param msg 显示的信息
	 * @return 返回Dialog
	 */
	public Dialog createLoadingDialog(Context context, String msg) {
		LayoutInflater inflater = LayoutInflater.from(context);
		View v = inflater.inflate(R.layout.dialog_layout, null);
		LinearLayout layout = (LinearLayout) v.findViewById(R.id.dialog_view);
		ImageView spaceshipImage = (ImageView) v.findViewById(R.id.dialog_img);
		TextView tipTextView = (TextView) v.findViewById(R.id.dialog_textView);
		Animation hyperspaceJumpAnimation = AnimationUtils.loadAnimation(context, R.anim.dialog_anim);
		spaceshipImage.startAnimation(hyperspaceJumpAnimation);
		tipTextView.setText(msg);
		Dialog loadingDialog = new Dialog(context, R.style.dialog_style);
		loadingDialog.setCanceledOnTouchOutside(false);
		loadingDialog.setOnCancelListener(new OnCancelListener() {
			@Override
			public void onCancel(DialogInterface arg0) {
				if (handler.hasMessages(REQUST_TIMEOUT)) {
					handler.removeMessages(REQUST_TIMEOUT);
				}
				if (handler.hasMessages(R.id.requst_wifi_list_id)) {
					handler.removeMessages(R.id.requst_wifi_list_id);
				}
			}
		});
		loadingDialog.setContentView(layout, new LinearLayout.LayoutParams(
				LinearLayout.LayoutParams.FILL_PARENT,
				LinearLayout.LayoutParams.FILL_PARENT));
		return loadingDialog;
	}
	
	@SuppressLint("HandlerLeak")
	public Handler handler = new Handler() {

		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUSTING:
					mDialog = createLoadingDialog(mContext, "正在请求设备周围WiFi列表...");
					mDialog.show();
					break;
				case REQUST_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					if (handler.hasMessages(REQUST_TIMEOUT)) {
						handler.removeMessages(REQUST_TIMEOUT);
					}
					if (handler.hasMessages(R.id.requst_wifi_list_id)) {
						handler.removeMessages(R.id.requst_wifi_list_id);
					}
					Toast.makeText(mContext, "请求WiFi列表失败，请重试！ ", Toast.LENGTH_SHORT).show();
					break;
				case R.id.requst_wifi_list_id:
					if (handler.hasMessages(REQUST_TIMEOUT)) {
						handler.removeMessages(REQUST_TIMEOUT);
						if (mDialog != null)
							mDialog.dismiss();
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							showWiFiListDialog((ArrayList<HashMap<String, Object>>) msg.obj);
						} else {
							Toast.makeText(mContext, "设备附近暂时没有WiFi信号！ ", Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.requst_wifi_list_id);
					}
					break;
				case IS_SETTING:
					mDialog = Utils.createLoadingDialog(mContext, "正在配置设备WiFi网络...");
					mDialog.show();
					break;
				case SET_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					if (handler.hasMessages(SET_TIMEOUT)) {
						handler.removeMessages(SET_TIMEOUT);
					}
					Toast.makeText(mContext, "配置设备WiFi网络超时，请重试！ ", Toast.LENGTH_SHORT).show();
					break;
				case R.id.set_term_wifi_id:
					if (handler.hasMessages(SET_TIMEOUT)) {
						handler.removeMessages(SET_TIMEOUT);
						if (mDialog != null)
							mDialog.dismiss();
						int resultCode = msg.arg1;
						if (resultCode == 1) {
							Toast.makeText(mContext, "配置设备WiFi网络成功！ ", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "配置设备WiFi网络失败，请重试！ ", Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.set_term_wifi_id);
					}
					break;
			}
		}
	};
	
	/**
	 * 发送Handler消息
	 */
	private void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	private void sendHandlerMsg(int what, int timeout) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessageDelayed(msg, timeout);
	}
	private void sendHandlerMsg(Handler handler, int what, HashMap<String, String> obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	/**
	 * 请求设备周围WiFi列表
	 */
	private void getWiFiListInfoEvent() {
		Handler sendHandler = ZmqThread.zmqThreadHandler;
		String data = generateGetWiFiInfoJson();
		sendHandlerMsg(IS_REQUSTING);
		sendHandlerMsg(REQUST_TIMEOUT, Value.REQ_TIME_10S);
		HashMap<String, String> map = new HashMap<String, String>();
		map.put("peerId", onlineDealerName);
		map.put("peerData", data);
		sendHandlerMsg(sendHandler, R.id.send_to_peer_id, map);
	}
	
	/**
	 * 配置设备WiFi网络
	 */
	private void setTermWiFiInfoEvent(String key) {
		Handler sendHandler = ZmqThread.zmqThreadHandler;
		String data = generateSetTermWiFiInfoJson(key);
		sendHandlerMsg(IS_SETTING);
		sendHandlerMsg(SET_TIMEOUT, Value.REQ_TIME_10S);
		HashMap<String, String> map = new HashMap<String, String>();
		map.put("peerId", onlineDealerName);
		map.put("peerData", data);
		sendHandlerMsg(sendHandler, R.id.send_to_peer_id, map);
	}
	
	/**
	 * 显示在线终端设备对话框
	 */
	private void showOnlineTermList() {
		ArrayList<HashMap<String, Object>> list = new ArrayList<HashMap<String, Object>>();
		HashMap<String, Object> map = null;
		int len = onlineTermList.size();
		for (int i=0; i<len; i++) {
			map = new HashMap<String, Object>();
			map.put("WiFiSSID", onlineTermList.get(i).get("deviceName"));
			map.put("WiFiLevel", R.drawable.icon_device_name);
			list.add(map);
		}
		WiFiAlertDialog wifiDialog = new WiFiAlertDialog(mContext);
		wifiDialog.setTitle("在线设备列表");
		WiFiAdapter adapter = new WiFiAdapter(mContext, list);
		wifiDialog.setAdapter(adapter);
		wifiDialog.setOnItemClickListenerButton(button_wifi_term_select, onlineTermList);
	}
	
	/**
	 * 显示请求后的终端周围WiFi列表对话框
	 */
	private void showWiFiListDialog(ArrayList<HashMap<String, Object>> list) {
		WiFiAlertDialog wifiDialog = new WiFiAlertDialog(mContext);
		wifiDialog.setTitle("WiFi列表");
		WiFiAdapter adapter = new WiFiAdapter(mContext, list);
		wifiDialog.setAdapter(adapter);
		wifiDialog.setOnItemClickListenerWiFiButton(btn_wifi_name, list);
	}
	
	/**
	 * 配置设备WiFi温馨提示
	 */
	private void showSetTermWiFiTip() {
		final OkCancelDialog myDialog=new OkCancelDialog(mContext);
		myDialog.setTitle("温馨提示");
		myDialog.setMessage("确认要配置的WiFi没有密码？");
		myDialog.setPositiveButton("确认", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
				setTermWiFiInfoEvent("");
			}
		});
		myDialog.setNegativeButton("取消", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
			}
		});
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.ib_wifi_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
				break;
			case R.id.btn_wifi_term_list:
				onlineTermList = MainApplication.getInstance().getOnlineDeviceList();
				if ((onlineTermList != null) && (onlineTermList.size() > 0)) {
					showOnlineTermList();
					System.out.println("MyDebug: dealerName："+onlineDealerName);
				} else {
					Toast.makeText(mContext, "没有在线设备 ", Toast.LENGTH_SHORT).show();
				}
				break;
			case R.id.btn_wifi_password_del:
				et_wifi_pwd.setText("");	
				break;
			case R.id.btn_wifi_name:
				onlineTermList = MainApplication.getInstance().getOnlineDeviceList();
				if ((onlineTermList != null) && (onlineTermList.size() > 0)) {
					getWiFiListInfoEvent();
				} else {
					Toast.makeText(mContext, "没有在线设备，无法请求WiFi列表！ ", Toast.LENGTH_SHORT).show();
				}
				break;
			case R.id.btn_wifi_submit:
				if ((onlineTermList != null) && (onlineTermList.size() > 0)) {
					String pwd = et_wifi_pwd.getText().toString().trim();
					if (selectedWiFi == null) {
						Toast.makeText(mContext, "请先搜索WiFi，再配置", Toast.LENGTH_SHORT).show();
					}
					else if (pwd.length() == 0) {
						showSetTermWiFiTip();
					} else {
						setTermWiFiInfoEvent(pwd);
					}
				} else {
					Toast.makeText(mContext, "设备不在线，无法配置网络 ", Toast.LENGTH_SHORT).show();
				}
				break;
		}
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		selectedWiFi = null;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
		}
		return super.onKeyDown(keyCode, event);
	}
	
	public static class WiFiAdapter extends BaseAdapter {

		private Context context;
		private ArrayList<HashMap<String, Object>> list = null;

		public WiFiAdapter(Context context, ArrayList<HashMap<String, Object>> list) {
			this.context = context;
			this.list = list;
		}

		@Override
		public int getCount() {
			return list.size();
		}

		@Override
		public Object getItem(int position) {
			return list.get(position);
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup viewGroup) {
			ViewHolder holder;

			if (convertView == null) {
				convertView = LayoutInflater.from(context).inflate(R.layout.wifi_list_item, null);
				holder = new ViewHolder();
				convertView.setTag(holder);
				holder.iv_wifi_level = (ImageView) convertView.findViewById(R.id.iv_wifi_level);
				holder.tv_wifi_ssid = (TextView) convertView.findViewById(R.id.tv_wifi_item);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			if (holder.iv_wifi_level != null) {
				holder.iv_wifi_level.setImageResource((Integer) list.get(position).get("WiFiLevel"));
			}
			if (holder.tv_wifi_ssid != null) {
				holder.tv_wifi_ssid.setText((CharSequence) list.get(position).get("WiFiSSID"));
			}
			return convertView;
		}

		class ViewHolder {
			ImageView iv_wifi_level;
			TextView tv_wifi_ssid;
		}
	}
}
