package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONArray;
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
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;
import com.video.utils.Utils;

@SuppressLint("HandlerLeak")
public class APSearchActivity extends Activity implements OnClickListener {

	private Context mContext;
	private ZMQ.Context context = null;
	private ZMQ.Socket zmqSocket = null;
	private ArrayList<HashMap<String, Object>> wifiList = new ArrayList<HashMap<String, Object>>();
	
	private Dialog mDialog = null;
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	private final int REQUEST_SUCCESS = 3;
	
	public static final String TerminalIPandPort = "tcp://10.110.110.1:22555";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.ap_search);
		
		initView();
		initData();
	}
	
	private void initView() {
		ImageButton ib_search_ap_back = (ImageButton) this.findViewById(R.id.ib_search_ap_back);
		ib_search_ap_back.setOnClickListener(this);
		
		Button btn_search = (Button) this.findViewById(R.id.btn_search);
		btn_search.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = APSearchActivity.this;
		if (wifiList == null) {
			wifiList = new ArrayList<HashMap<String, Object>>();
		}
	}
	
	/**
	 * 初始化Zmq
	 */
	private void initZmq() {
		context = ZMQ.context(1); 
		zmqSocket = context.socket(ZMQ.REQ); 
		zmqSocket.connect(TerminalIPandPort);
	}
	
	public class ZmqThread extends Thread {
		@Override
		public void run() {
			try {
				initZmq();
				
				String requestData = generateGetWiFiInfoJson();
				if (requestData == null) {
					return ;
				}
				sendHandlerMsg(IS_REQUESTING);
				sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
				Boolean requestFlag = zmqSocket.send(requestData.getBytes());
				if (!requestFlag) {
					return ;
				}
				Utils.log("send zmq data："+requestData);
				
				sleep(500);
				
				byte[] respondByteArray = zmqSocket.recv();
				if (respondByteArray == null) {
					return ;
				}
				String respondData = new String(respondByteArray);
				Utils.log("receive zmq data："+respondData);
				
				JSONObject obj = new JSONObject(respondData);
				if (obj.getString("command").equals("wifi_info")) {
					if (!obj.isNull("wifis")) {
						JSONArray jsonArray = obj.getJSONArray("wifis");
						wifiList = getWiFiList(jsonArray);
						sendHandlerMsg(REQUEST_SUCCESS);
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
	private void sendHandlerMsg(int what, int timeout) {
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
						mDialog = Utils.createLoadingDialog(mContext, getResources().getString(R.string.searching));
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
					if (handler.hasMessages(REQUEST_SUCCESS)) {
						handler.removeMessages(REQUEST_SUCCESS);
					}
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					Toast.makeText(mContext,getResources().getString(R.string.search_timeout), Toast.LENGTH_SHORT).show();
					break;
				case REQUEST_SUCCESS:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if ((wifiList != null) && (wifiList.size() > 0)) {
							Intent intent = new Intent(mContext, APListActivity.class);
							intent.putExtra("wifiList", wifiList);
							startActivity(intent);
							overridePendingTransition(R.anim.up_in, R.anim.fragment_nochange);
						} else {
							Toast.makeText(mContext, getResources().getString(R.string.the_available_network_equipment_around), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(REQUEST_SUCCESS);
					}
					break;
			}
		}
	};
	
	/**
	 * 生成JSON请求WiFi列表信息字符串
	 */
	private String generateGetWiFiInfoJson() {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "tunnel");
			jsonObj.put("command", "wifi_info");
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 请求终端周围WiFi列表
	 */
	private ArrayList<HashMap<String, Object>> getWiFiList(JSONArray jsonArray) {
		ArrayList<HashMap<String, Object>> list = new ArrayList<HashMap<String, Object>>();
		int len = jsonArray.length();
		if (len <= 0) {
			return null;
		}
	    try {
	    	for (int i=0; i<len; i++) { 
		    	JSONObject obj = (JSONObject) jsonArray.get(i); 
		    	HashMap<String, Object> map = new HashMap<String, Object>();
		    	map.put("WiFiSSID", obj.getString("ssid"));
		    	map.put("WiFiLevel", Utils.getWiFiIconResIdByLevel(obj.getInt("signalStrength")));
		    	map.put("WiFiEncryptMode", obj.getInt("encryptMode"));
		    	map.put("WiFiEncryptFormat", obj.getInt("encryptFormat"));
		    	map.put("WiFiEnable", obj.getInt("enable"));
				list.add(map);
	    	}
	    	return list;
		} catch (JSONException e) {
			e.printStackTrace();
			Utils.log("getTermWiFiList()异常！");
		}
		return null;
	}
	
	@Override
	public void onClick(View view) {
		// TODO Auto-generated method stub
		switch (view.getId()) {
			case R.id.ib_search_ap_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.up_out);
				break;
			case R.id.btn_search:
				new ZmqThread().start();
				break;
		}
	}
	
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
				zmqSocket.disconnect(TerminalIPandPort);
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

}
