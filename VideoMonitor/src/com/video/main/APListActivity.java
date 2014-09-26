package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONException;
import org.json.JSONObject;
import org.zeromq.ZMQ;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;
import com.video.utils.InputPwdDialog;
import com.video.utils.Utils;

@SuppressLint("HandlerLeak")
public class APListActivity extends Activity implements OnClickListener {

	private Context mContext;
	private ListView lv_ap_list;
	private ArrayList<HashMap<String, Object>> wifiList = new ArrayList<HashMap<String, Object>>();
	private WiFiAdapter wifiAdapter;
	
	private ZMQ.Context context = null;
	private ZMQ.Socket zmqSocket = null;
	
	private Dialog mDialog = null;
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	private final int RECV_REQUEST = 3;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.ap_list);
		
		initView();
		initData();
	}
	
	private void initView() {
		ImageButton ib_search_ap_back = (ImageButton) this.findViewById(R.id.ib_search_ap_back);
		ib_search_ap_back.setOnClickListener(this);
		
		lv_ap_list = (ListView) this.findViewById(R.id.lv_ap_list);
		lv_ap_list.setOnItemClickListener(new OnItemClickListenerImpl());
	}
	
	@SuppressWarnings("unchecked")
	private void initData() {
		mContext = APListActivity.this;
		if (wifiList == null) {
			wifiList = new ArrayList<HashMap<String, Object>>();
		}
		Intent intent = this.getIntent();
		if (intent != null) {
			wifiList = (ArrayList<HashMap<String, Object>>) intent.getSerializableExtra("wifiList");
		}
		if ((wifiList != null) && (wifiList.size() > 0)) {
			wifiAdapter = new WiFiAdapter(mContext, wifiList);
			lv_ap_list.setAdapter(wifiAdapter);
		}
		new Thread(){
			public void run(){
				initZmq();
			}
		}.start();
		
	}
	
	/**
	 * 初始化Zmq
	 */
	private void initZmq() {
		context = ZMQ.context(1); 
		zmqSocket = context.socket(ZMQ.REQ); 
		zmqSocket.connect(APSearchActivity.TerminalIPandPort);
	}
	
	private class OnItemClickListenerImpl implements OnItemClickListener {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, final int position, long id) {
			// TODO Auto-generated method stub
			showInputPwdDialog(wifiList.get(position));
		}
	}
	
	@Override
	public void onClick(View view) {
		// TODO Auto-generated method stub
		switch (view.getId()) {
			case R.id.ib_search_ap_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.up_out);
				break;
		}
	}
	
	public void showInputPwdDialog(final HashMap<String, Object> wifiItem) {
		final InputPwdDialog myDialog = new InputPwdDialog(mContext);
		myDialog.setTitle(getResources().getString(R.string.enter_network_password));
		myDialog.setCanceledOnTouchOutside(false);
		myDialog.setPositiveButton(getResources().getString(R.string.confirm), new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
				String wifiPwd = myDialog.getEditTextContent(); 
				String requestData = generateSetWiFiInfoJson(wifiItem, wifiPwd);
				if (requestData != null) {
					new ZmqSetWiFiInfoThread(requestData).start();
				} else {
					Toast.makeText(mContext, getResources().getString(R.string.analysis_of_WiFi_information_error), Toast.LENGTH_SHORT).show();
				}
			}
		});
		myDialog.setNegativeButton(getResources().getString(R.string.cancel), new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
			}
		});
	}
	
	/**
	 * 生成JSON配置设备WiFi网络字符串
	 */
	private String generateSetWiFiInfoJson(HashMap<String, Object> wifiItem, String key) {
		if (wifiItem == null) {
			return null;
		}
		if (key == null) {
			key = "";
		}
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "tunnel");
			jsonObj.put("command", "set_wifi");
			JSONObject subObj = new JSONObject();
				subObj.put("ssid", wifiItem.get("WiFiSSID"));
				subObj.put("key", key);
				subObj.put("enable", 1); //1:打开WiFi  0:关闭WiFi
				subObj.put("encryptMode", wifiItem.get("WiFiEncryptMode")); //0-none auth,1-wep ,2-wpa,3-wpa2
				subObj.put("encryptFormat", wifiItem.get("WiFiEncryptFormat")); //0-enc tkip,1-enc aes
			jsonObj.put("param", subObj);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	public class ZmqSetWiFiInfoThread extends Thread {
		
		private String sendData;
		
		public ZmqSetWiFiInfoThread(String sendData) {
			this.sendData = sendData;
		}
		
		@Override
		public void run() {
			try {
				sendHandlerMsg(IS_REQUESTING);
				sendHandlerDelayedMsg(REQUEST_TIMEOUT, Value.REQ_TIME_60S);
				Boolean requestFlag = zmqSocket.send(sendData.getBytes());
				if (!requestFlag) {
					return ;
				}
				Utils.log("send zmq data："+sendData);
				sleep(500);
				
				byte[] respondByteArray = zmqSocket.recv();
				if (respondByteArray == null) {
					return ;
				}
				String respondData = new String(respondByteArray);
				Utils.log("receive zmq data："+respondData);
				
				JSONObject obj = new JSONObject(respondData);
				if (obj.getString("command").equals("set_wifi")) {
					if (obj.getBoolean("result")) {
						sendHandlerMsg(RECV_REQUEST, 0);
					} else {
						sendHandlerMsg(RECV_REQUEST, -1);
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * 发送Handler消息
	 */
	private void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	private void sendHandlerMsg(int what, int arg1) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		handler.sendMessage(msg);
	}
	private void sendHandlerDelayedMsg(int what, int timeout) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessageDelayed(msg, timeout);
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case IS_REQUESTING:
					if ((mDialog == null) || (!mDialog.isShowing())) {
						mDialog = Utils.createLoadingDialog(mContext,getResources().getString(R.string.configuring_the_network));
						mDialog.show();
					}
					if (handler.hasMessages(IS_REQUESTING)) {
						handler.removeMessages(IS_REQUESTING);
					}
					break;
				case REQUEST_TIMEOUT:
					if ((mDialog != null) && (mDialog.isShowing())) {
						mDialog.dismiss();
						mDialog = null;
					}
					Toast.makeText(mContext, getResources().getString(R.string.configurate_failed), Toast.LENGTH_SHORT).show();
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					break;
				// 连接网络
				case RECV_REQUEST:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							Toast.makeText(mContext, getResources().getString(R.string.configurate_success), Toast.LENGTH_SHORT).show();
							finish();
							overridePendingTransition(R.anim.fragment_nochange, R.anim.up_out);
						} else {
							Toast.makeText(mContext, getResources().getString(R.string.configurate_failed), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(RECV_REQUEST);
					}
					break;
			}
		}
	};
	
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		if ((mDialog != null) && (mDialog.isShowing())) {
			mDialog.dismiss();
			mDialog = null;
		}
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		if (zmqSocket != null) {
			new disconnectZmqThread().start();
		}
	}
	
	public class disconnectZmqThread extends Thread {
		@Override
		public void run() {
			try {
				zmqSocket.disconnect(APSearchActivity.TerminalIPandPort);
				zmqSocket = null;
			} catch (Exception e) {
				e.printStackTrace();
			}
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
				convertView = LayoutInflater.from(context).inflate(R.layout.ap_list_item, null);
				holder = new ViewHolder();
				convertView.setTag(holder);
				holder.iv_ap_level = (ImageView) convertView.findViewById(R.id.iv_ap_level);
				holder.tv_ap_ssid = (TextView) convertView.findViewById(R.id.tv_ap_ssid);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			if (holder.iv_ap_level != null) {
				holder.iv_ap_level.setImageResource((Integer) list.get(position).get("WiFiLevel"));
			}
			if (holder.tv_ap_ssid != null) {
				holder.tv_ap_ssid.setText((CharSequence) list.get(position).get("WiFiSSID"));
			}
			return convertView;
		}

		static class ViewHolder {
			ImageView iv_ap_level;
			TextView tv_ap_ssid;
		}
	}
}
