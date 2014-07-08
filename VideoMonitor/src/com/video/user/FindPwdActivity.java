package com.video.user;

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
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

public class FindPwdActivity extends Activity implements OnClickListener {

	private Context mContext;
	private Dialog mDialog = null;
	
	private EditText et_name;
	private EditText et_email;
	private EditText et_password;
	private Button btn_submit;
	private Button button_delete_username;
	private Button button_delete_email;
	private Button button_delete_password;
	
	private String userName = "";
	private String userEmail = "";
	private String userPassword = "";
	
	private final int IS_SUBMITTING = 1;
	private final int SUBMIT_TIMEOUT = 2;
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.find_pwd);
		
		initView();
		initData();
	}

	private void initView() {
		ImageButton button_back = (ImageButton) this.findViewById(R.id.btn_find_back);
		button_back.setOnClickListener(this);
		
		button_delete_username = (Button) this.findViewById(R.id.btn_find_username_del);
		button_delete_username.setOnClickListener(this);
		
		button_delete_email = (Button) this.findViewById(R.id.btn_find_email_del);
		button_delete_email.setOnClickListener(this);
		
		button_delete_password = (Button) this.findViewById(R.id.btn_find_password_del);
		button_delete_password.setOnClickListener(this);
		
		et_name = (EditText)super.findViewById(R.id.et_find_username);
		et_name.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_username.setVisibility(View.INVISIBLE);
				} else {
					button_delete_username.setVisibility(View.VISIBLE);
				}
			}
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
			@Override
			public void afterTextChanged(Editable s) {}
		});
		
		et_email = (EditText)super.findViewById(R.id.et_find_email);
		et_email.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_email.setVisibility(View.INVISIBLE);
				} else {
					button_delete_email.setVisibility(View.VISIBLE);
				}
			}
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
			@Override
			public void afterTextChanged(Editable s) {}
		});
		
		et_password = (EditText)super.findViewById(R.id.et_find_password);
		et_password.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_password.setVisibility(View.INVISIBLE);
				} else {
					button_delete_password.setVisibility(View.VISIBLE);
				}
			}
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
			@Override
			public void afterTextChanged(Editable s) {}
		});
		
		btn_submit = (Button)super.findViewById(R.id.btn_find_pwd_submit);
		btn_submit.setOnClickListener(this);
	}

	private void initData() {
		mContext = FindPwdActivity.this;
		ZmqHandler.mHandler = handler;
	}
	
	/**
	 * 生成JSON的注册字符串
	 */
	private String generateFindPwdJson(String username, String email, String password) {
		String newPwd = username+":"+Value.realm+":"+password;
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ResetPwd");
			jsonObj.put("UserName", username);
			jsonObj.put("Email", email);
			jsonObj.put("NewPwd", Utils.CreateMD5Pwd(newPwd));
			return jsonObj.toString();
		} catch (JSONException e) {
			System.out.println("MyDebug: generateFindPwdJson()异常！");
			e.printStackTrace();
		}
		return null;
	}
	
	private Handler handler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_SUBMITTING:
					if (mDialog == null) {
						mDialog = Utils.createLoadingDialog(mContext, "正在找回...");
						mDialog.show();
					}
					break;
				case SUBMIT_TIMEOUT:
					if ((mDialog != null) && (mDialog.isShowing())) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (handler.hasMessages(SUBMIT_TIMEOUT)) {
						handler.removeMessages(SUBMIT_TIMEOUT);
					}
					Toast.makeText(mContext, "重置密码失败，网络超时！", Toast.LENGTH_SHORT).show();
					break;
				case R.id.find_pwd_id:
					if (handler.hasMessages(SUBMIT_TIMEOUT)) {
						handler.removeMessages(SUBMIT_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							Toast.makeText(mContext, "恭喜您，重置密码成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "重置密码失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.find_pwd_id);
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
	
	private void clickFindPwdEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkFindPwdData()) {
				String data = generateFindPwdJson(userName, userEmail, userPassword);
				sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
				sendHandlerMsg(IS_SUBMITTING);
				sendHandlerMsg(SUBMIT_TIMEOUT, Value.REQ_TIME_10S);
			}
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_find_back:
				finish();
				break;
			case R.id.btn_find_username_del:
				et_name.setText("");
				break;
			case R.id.btn_find_email_del:
				et_email.setText("");
				break;
			case R.id.btn_find_password_del:
				et_password.setText("");
				break;
			case R.id.btn_find_pwd_submit:
				clickFindPwdEvent();
				break;
		}
	}
	
	/**
	 * @return true:注册信息格式正确  false:注册信息格式错误
	 */
	private boolean checkFindPwdData() {
		boolean resultFlag = false;
		
		//获取EditText输入框的字符串
		userName = et_name.getText().toString().trim();
		userEmail = et_email.getText().toString().trim();
		userPassword = et_password.getText().toString().trim();
		
		if (userName.equals("")) {
			resultFlag = false;
			Toast.makeText(mContext, "请输入用户名！", Toast.LENGTH_SHORT).show();
		}
		else if (Utils.isChineseString(userName)) {
			resultFlag = false;
			Toast.makeText(mContext, "不支持中文！", Toast.LENGTH_SHORT).show();
		}
		else if ((userName.length()<3) || (userName.length()>20)) {
			resultFlag = false;
			Toast.makeText(mContext, "用户名长度范围3~20！", Toast.LENGTH_SHORT).show();
		} else {
			resultFlag = true;
			if (userEmail.equals("")) {
				resultFlag = false;
				Toast.makeText(mContext, "请输入电子邮箱！", Toast.LENGTH_SHORT).show();
			}
			else if (Utils.isChineseString(userEmail)) {
				resultFlag = false;
				Toast.makeText(mContext, "不支持中文！", Toast.LENGTH_SHORT).show();
			}
			else if ((userEmail.length()<6) || (userEmail.length()>20)) {
				resultFlag = false;
				Toast.makeText(mContext, "电子邮箱长度范围6~20！", Toast.LENGTH_SHORT).show();
			}
			else if (userEmail.indexOf("@")<=0) {
				resultFlag = false;
				Toast.makeText(mContext, "邮箱格式不正确！", Toast.LENGTH_SHORT).show();
			}
			else if (userEmail.indexOf(".")<=0) {
				resultFlag = false;
				Toast.makeText(mContext, "邮箱格式不正确！", Toast.LENGTH_SHORT).show();
			} else {
				resultFlag = true;
				if (userPassword.equals("")) {
					resultFlag = false;
					Toast.makeText(mContext, "请输入新密码！", Toast.LENGTH_SHORT).show();
				}
				else if (Utils.isChineseString(userPassword)) {
					resultFlag = false;
					Toast.makeText(mContext, "不支持中文！", Toast.LENGTH_SHORT).show();
				}
				else if ((userPassword.length()<6) || (userPassword.length()>20)) {
					resultFlag = false;
					Toast.makeText(mContext, "密码长度范围6~20！", Toast.LENGTH_SHORT).show();
				} else {
					resultFlag = true;
				}
			}
		}
		return resultFlag;
	}
}
