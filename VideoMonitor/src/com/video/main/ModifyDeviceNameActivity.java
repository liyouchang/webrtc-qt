package com.video.main;

import org.json.JSONException;
import org.json.JSONObject;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Toast;
import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.data.XmlDevice;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

@SuppressLint("HandlerLeak")
public class ModifyDeviceNameActivity extends Activity implements OnClickListener {

	private Context mContext;
	private XmlDevice xmlData;
	private PreferData preferData = null;
	private String userName = null;
	private EditText et_name;
	private Button button_delete_devicename;
	private Dialog mDialog = null;
	
	private String mDeviceName = "";
	private String mDeviceId = "";
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.modify_device_name);
		
		initView();
		initData();
	}

	private void initView() {
		ImageButton button_back = (ImageButton) this.findViewById(R.id.btn_modify_device_back);
		button_back.setOnClickListener(this);
		
		Button button_ok = (Button) this.findViewById(R.id.btn_modify_device_ok);
		button_ok.setOnClickListener(this);
		
		button_delete_devicename = (Button) this.findViewById(R.id.btn_modify_device_name_del);
		button_delete_devicename.setOnClickListener(this);
		
		et_name = (EditText) this.findViewById(R.id.et_modify_device_name);
		et_name.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_devicename.setVisibility(View.INVISIBLE);
				} else {
					button_delete_devicename.setVisibility(View.VISIBLE);
				}
			}
			
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
		});
	}
	
	private void initData() {
		mContext = ModifyDeviceNameActivity.this;
		ZmqHandler.mHandler = handler;
		xmlData = new XmlDevice(mContext);
		preferData = new PreferData(mContext);
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		
		Bundle bundle = this.getIntent().getExtras();
		mDeviceName = bundle.getString("deviceName");
		mDeviceId = bundle.getString("deviceID");
		
		et_name.setText(mDeviceName);
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
			jsonObj.put("UserName", userName);
			jsonObj.put("MAC", mDeviceId);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUESTING:
					mDialog = Utils.createLoadingDialog(mContext, getResources().getString(R.string.is_submitting_the_modification));
					mDialog.show();
					break;
				case REQUEST_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					Value.isNeedReqTermListFlag = false;
					Toast.makeText(mContext, getResources().getString(R.string.modifying_the_terminal_name_failed), Toast.LENGTH_SHORT).show();
					break;
				case R.id.modify_device_name_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog != null)
							mDialog.dismiss();
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							Toast.makeText(mContext, getResources().getString(R.string.modifying_the_terminal_name_success), Toast.LENGTH_SHORT).show();
							xmlData.updateItemName(mDeviceId, mDeviceName);
							
							Bundle bundle = new Bundle();
							bundle.putString("deviceId", mDeviceId);
							bundle.putString("deviceName", mDeviceName);
							Intent intent = new Intent();
							intent.putExtras(bundle);
							setResult(1, intent);
							finish();
							overridePendingTransition(0, R.anim.down_out);
						} else {
							Toast.makeText(mContext, getResources().getString(R.string.modifying_terminal_name_failed)+","+Utils.getErrorReason(resultCode)+"！", Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.modify_device_name_id);
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
	private void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	public void clickAddDeviceEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkModifyDeviceData()) {
				String data = generateModifyTermNameJson();
				sendHandlerMsg(IS_REQUESTING);
				sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
				sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
			}
		} else {
			Toast.makeText(mContext, getResources().getString(R.string.no_available_network_connection), Toast.LENGTH_SHORT).show();
		}
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_modify_device_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.down_out);
				break;
			case R.id.btn_modify_device_ok:
				clickAddDeviceEvent();
				break;
			case R.id.btn_modify_device_name_del:
				et_name.setText("");
				break;
		}
	}
	
	/**
	 * @return true:注册信息格式正确  false:注册信息格式错误
	 */
	private boolean checkModifyDeviceData() {
		boolean resultFlag = false;
		
		//获取EditText输入框的字符串
		String newDeviceName = et_name.getText().toString().trim();
		
		if (newDeviceName.equals("")) {
			resultFlag = false;
			Toast.makeText(mContext, getResources().getString(R.string.please_enter_the_device_name), Toast.LENGTH_SHORT).show();
		}
		else if ((newDeviceName.length()<2) || (newDeviceName.length()>20)) {
			resultFlag = false;
			Toast.makeText(mContext,getResources().getString(R.string.The_length_range_of_device_name), Toast.LENGTH_SHORT).show();
		} else {
			resultFlag = true;
			mDeviceName = newDeviceName;
		}
		return resultFlag;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			ModifyDeviceNameActivity.this.finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.down_out);
		}
		return super.onKeyDown(keyCode, event);
	}
}
