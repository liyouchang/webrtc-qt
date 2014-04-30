package com.video.main;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
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
import com.video.data.Value;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqHandler;
import com.video.utils.Utils;

public class AddShareActivity extends Activity implements OnClickListener {

	private Context mContext;
	
	private EditText et_name;
	private Button button_delete_name;
	private Dialog mDialog = null;
	//终端列表项
	private String mDeviceId = null;
	private String shareName = "";
	
	private final int IS_ADDING = 1;
	private final int ADD_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.add_share);
		
		initView();
		initData();
	}

	private void initView() {
		ImageButton button_back = (ImageButton) this.findViewById(R.id.btn_add_share_back);
		button_back.setOnClickListener(this);
		
		Button button_ok = (Button) this.findViewById(R.id.btn_add_share_ok);
		button_ok.setOnClickListener(this);
		
		button_delete_name = (Button) this.findViewById(R.id.btn_add_share_name_del);
		button_delete_name.setOnClickListener(this);
		
		et_name = (EditText) this.findViewById(R.id.et_add_share_name);
		et_name.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_name.setVisibility(View.INVISIBLE);
				} else {
					button_delete_name.setVisibility(View.VISIBLE);
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
		mContext = AddShareActivity.this;
		ZmqHandler.setHandler(handler);
		Bundle bundle = this.getIntent().getExtras();
		mDeviceId = bundle.getString("deviceID");
	}
	
	/**
	 * 生成JSON的添加终端分享字符串
	 */
	private String generateAddShareTermJson(String mac) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_AddShareTerm");
			jsonObj.put("UserName", shareName);
			jsonObj.put("MAC", mac);
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
				case IS_ADDING:
					mDialog = Utils.createLoadingDialog(mContext, "正在分享...");
					mDialog.show();
					break;
				case ADD_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					Toast.makeText(mContext, "添加终端分享失败，网络超时！", Toast.LENGTH_SHORT).show();
					if (handler.hasMessages(ADD_TIMEOUT)) {
						handler.removeMessages(ADD_TIMEOUT);
					}
					break;
				case R.id.add_device_share_id:
					if (handler.hasMessages(ADD_TIMEOUT)) {
						handler.removeMessages(ADD_TIMEOUT);
						if (mDialog != null)
							mDialog.dismiss();
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							Toast.makeText(mContext, "添加终端分享成功！", Toast.LENGTH_SHORT).show();
							AddShareActivity.this.finish();
						} else {
							Toast.makeText(mContext, "添加终端分享失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.add_device_share_id);
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
			if (checkAddShareData()) {
				Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
				String data = generateAddShareTermJson(mDeviceId);
				sendHandlerMsg(IS_ADDING);
				sendHandlerMsg(ADD_TIMEOUT, Value.requestTimeout);
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
			case R.id.btn_add_share_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.down_out);
				break;
			case R.id.btn_add_share_ok:
				clickAddDeviceEvent();
				break;
			case R.id.btn_add_share_name_del:
				et_name.setText("");
				break;
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.down_out);
		}
		return super.onKeyDown(keyCode, event);
	}
	
	/**
	 * @return true:注册信息格式正确  false:注册信息格式错误
	 */
	private boolean checkAddShareData() {
		boolean resultFlag = false;
		
		//获取EditText输入框的字符串
		shareName = et_name.getText().toString().trim();
		
		if (shareName.equals("")) {
			resultFlag = false;
			et_name.setError("请输入用户名！");
		}
		else if ((shareName.length()<3) || (shareName.length()>20)) {
			resultFlag = false;
			et_name.setError("用户名长度范围3~20！");
		} else {
			resultFlag = true;
		}
		return resultFlag;
	}
}
