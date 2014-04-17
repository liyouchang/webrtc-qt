package com.video.user;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.main.MainActivity;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

public class LoginActivity extends Activity implements OnClickListener {

	private Context mContext;
	private PreferData preferData = null;
	
	private EditText et_name;
	private EditText et_pwd;
	private CheckBox cb_auto_login;
	private ProgressDialog progressDialog;
	private boolean isAutoLogin = false;
	
	private String userName = "";
	private String userPwd = "";
	
	private final int IS_LOGINNING = 1;
	private final int LOGIN_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.login);
		
		initView();
	}
	
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		initData();
	}
	
	private void initView() {
		et_name = (EditText) super.findViewById(R.id.et_name);
		et_pwd = (EditText) super.findViewById(R.id.et_pwd);
		cb_auto_login = (CheckBox) super.findViewById(R.id.cb_auto_login);
		cb_auto_login.setOnCheckedChangeListener(new onCheckedChangeListenerImpl());
		Button button_login = (Button) super.findViewById(R.id.btn_login);
		button_login.setOnClickListener(this);
		TextView text_register = (TextView)super.findViewById(R.id.tv_register);
		text_register.setOnClickListener(this);
		TextView text_find_pwd = (TextView)super.findViewById(R.id.tv_find_pwd);
		text_find_pwd.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = LoginActivity.this;
		ZmqHandler.setHandler(handler);
		preferData = new PreferData(mContext);
		
		if (isAutoLogin) {
			cb_auto_login.setChecked(true);
		} else {
			cb_auto_login.setChecked(false);
		}
		
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
			et_name.setText(userName);
		}
		
		if (preferData.isExist("UserPwd")) {
			userPwd = preferData.readString("UserPwd");
			et_pwd.setText(userPwd);
		}
		
		if (preferData.isExist("AutoLogin")) {
			isAutoLogin = preferData.readBoolean("AutoLogin");
		}
		if (isAutoLogin) {
			clickLoginEvent();
		}
	}
	
	/**
	 * 生成JSON的登录字符串
	 */
	private String generateLoginJson(String username, String pwd) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_Login");
			jsonObj.put("UserName", username);
			jsonObj.put("Pwd", Utils.CreateMD5Pwd(pwd));
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
				case IS_LOGINNING:
					showProgressDialog("正在登录... ");
					break;
				case LOGIN_TIMEOUT:
					if (progressDialog != null)
						progressDialog.dismiss();
					if (handler.hasMessages(LOGIN_TIMEOUT)) {
						handler.removeMessages(LOGIN_TIMEOUT);
					}
					Toast.makeText(mContext, "登录失败，网络超时！", Toast.LENGTH_SHORT).show();
					break;
				case R.id.login_id:
					if (handler.hasMessages(LOGIN_TIMEOUT)) {
						handler.removeMessages(LOGIN_TIMEOUT);
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							if (progressDialog != null)
								progressDialog.dismiss();
							Value.isLoginSuccess = true;
							startActivity(new Intent(mContext, MainActivity.class));
							LoginActivity.this.finish();
						} else {
							if (progressDialog != null)
								progressDialog.dismiss();
							Toast.makeText(mContext, "登录失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.login_id);
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
	private void sendHandlerMsg(Handler handler, int what,  String obj, int timeout) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessageDelayed(msg, timeout);
	}
	
	private class onCheckedChangeListenerImpl implements OnCheckedChangeListener {

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			// TODO Auto-generated method stub
			preferData.writeData("AutoLogin", isChecked);
		}
	}
	
	public void clickLoginEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkRegisterData()) {
				if (preferData.isExist("UserName")) {
					preferData.deleteItem("UserName");
					preferData.writeData("UserName", userName);
				} else {
					preferData.writeData("UserName", userName);
				}
				
				if (preferData.isExist("UserPwd")) {
					preferData.deleteItem("UserPwd");
					preferData.writeData("UserPwd", userPwd);
				} else {
					preferData.writeData("UserPwd", userPwd);
				}
				Handler sendHandler = ZmqThread.zmqThreadHandler;
				String data = generateLoginJson(userName, userPwd);
				sendHandlerMsg(IS_LOGINNING);
				sendHandlerMsg(LOGIN_TIMEOUT, Value.requestTimeout);
				if (isAutoLogin) {
					sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data, 1000);
				} else {
					sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
				}
			}
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_login:
				clickLoginEvent();
				break;
			case R.id.tv_register:
				startActivity(new Intent(this, RegisterActivity.class));
				break;
			case R.id.tv_find_pwd:
				startActivity(new Intent(this, FindPwdActivity.class));
				break;
		}
	}
	
	/**
	 * @return true:注册信息格式正确  false:注册信息格式错误
	 */
	private boolean checkRegisterData() {
		boolean resultFlag = false;
		
		//获取EditText输入框的字符串
		userName = et_name.getText().toString().trim();
		userPwd = et_pwd.getText().toString().trim();
		
		if (userName.equals("")) {
			resultFlag = false;
			et_name.setError("请输入用户名！");
		}
		else if ((userName.length()<3) || (userName.length()>20)) {
			resultFlag = false;
			et_name.setError("用户名长度范围3~20！");
		} else {
			resultFlag = true;
			if (userPwd.equals("")) {
				resultFlag = false;
				et_pwd.setError("请输入密码！");
			}
			else if ((userPwd.length()<6) || (userPwd.length()>20)) {
				resultFlag = false;
				et_pwd.setError("密码长度范围6~20！");
			} else {
				resultFlag = true;
			}
		}
		return resultFlag;
	}
}
