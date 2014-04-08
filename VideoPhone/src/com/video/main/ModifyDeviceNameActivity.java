package com.video.main;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;
import com.video.data.XmlDevice;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqHandler;
import com.video.utils.Utils;

public class ModifyDeviceNameActivity extends Activity implements OnClickListener {

	private Context mContext;
	private XmlDevice xmlData;
	private EditText et_name;
	private ProgressDialog progressDialog;
	
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
		
		et_name = (EditText) this.findViewById(R.id.et_modify_device_name);
	}
	
	private void initData() {
		mContext = ModifyDeviceNameActivity.this;
		ZmqHandler.setHandler(handler);
		xmlData = new XmlDevice(mContext);
		
		Bundle bundle = this.getIntent().getExtras();
		mDeviceName = bundle.getString("deviceName");
		et_name.setText(mDeviceName);
		mDeviceId = bundle.getString("deviceID");
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
		progressDialog = new ProgressDialog(mContext);
        progressDialog.setMessage(info); 
        progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);  
        progressDialog.setIndeterminate(false);     
        progressDialog.setCancelable(false); 
        progressDialog.show(); 
	}
	
	private Handler handler = new Handler() {
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
					Toast.makeText(mContext, msg.obj+"，网络超时！", Toast.LENGTH_SHORT).show();
					break;
				case R.id.modify_device_name_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							if (progressDialog != null)
								progressDialog.dismiss();
							Toast.makeText(mContext, "修改终端名称成功！", Toast.LENGTH_SHORT).show();
							xmlData.updateItemName(mDeviceId, mDeviceName);
							finish();
							overridePendingTransition(0, R.anim.down_out);
						} else {
							if (progressDialog != null)
								progressDialog.dismiss();
							Toast.makeText(mContext, msg.obj+"，"+Utils.getErrorReason(resultCode)+"！", Toast.LENGTH_SHORT).show();
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
	
	public void clickAddDeviceEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkModifyDeviceData()) {
				Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
				String data = generateModifyTermNameJson();
				sendHandlerMsg(IS_REQUESTING, "正在提交修改...");
				sendHandlerMsg(REQUEST_TIMEOUT, "修改终端名称失败", Value.requestTimeout);
				sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
			}
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_modify_device_back:
				finish();
				overridePendingTransition(0, R.anim.down_out);
				break;
			case R.id.btn_modify_device_ok:
				clickAddDeviceEvent();
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
			et_name.setError("请输入设备名称！");
		}
		else if ((newDeviceName.length()<2) || (newDeviceName.length()>20)) {
			resultFlag = false;
			et_name.setError("设备名称长度范围2~20！");
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
			overridePendingTransition(0, R.anim.down_out);
		}
		return super.onKeyDown(keyCode, event);
	}
	
}
