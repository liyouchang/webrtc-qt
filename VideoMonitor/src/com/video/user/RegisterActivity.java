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
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.service.MainApplication;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

public class RegisterActivity extends Activity implements OnClickListener {

	private Context mContext;
	private PreferData preferData = null;
	
	private EditText et_name;
	private EditText et_email;
	private EditText et_pwd;
	private EditText et_repwd;
	private Button btn_register;
	private Button button_delete_username;
	private Button button_delete_email;
	private Button button_delete_password;
	private Button button_delete_repassword;
	private Dialog mDialog = null;
	
	private String userName = "";
	private String userEmail = "";
	private String userPwd = "";
	private String userRepwd = "";
	
	private final int IS_REGISTERING = 1;
	private final int REGISTER_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub 
		super.onCreate(savedInstanceState);
		setContentView(R.layout.register);
		
		initView();
		initData();
	}
	
	private void initView() {
		ImageButton button_back = (ImageButton) this.findViewById(R.id.btn_register_back);
		button_back.setOnClickListener(this);
		
		button_delete_username = (Button) this.findViewById(R.id.btn_register_username_del);
		button_delete_username.setOnClickListener(this);
		
		button_delete_email = (Button) this.findViewById(R.id.btn_register_email_del);
		button_delete_email.setOnClickListener(this);
		
		button_delete_password = (Button) this.findViewById(R.id.btn_register_password_del);
		button_delete_password.setOnClickListener(this);
		
		button_delete_repassword = (Button) this.findViewById(R.id.btn_register_repassword_del);
		button_delete_repassword.setOnClickListener(this);
		
		et_name = (EditText)super.findViewById(R.id.et_register_username);
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
		
		et_email = (EditText)super.findViewById(R.id.et_register_email);
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
		
		et_pwd = (EditText)super.findViewById(R.id.et_register_password);
		et_pwd.addTextChangedListener(new TextWatcher() {
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
		
		et_repwd = (EditText)super.findViewById(R.id.et_register_repassword);
		et_repwd.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_repassword.setVisibility(View.INVISIBLE);
				} else {
					button_delete_repassword.setVisibility(View.VISIBLE);
				}
			}
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
			@Override
			public void afterTextChanged(Editable s) {}
		});
		
		btn_register = (Button)super.findViewById(R.id.btn_register);
		btn_register.setOnClickListener(this);
	}

	private void initData() {
		mContext = RegisterActivity.this;
		ZmqHandler.mHandler = handler;
		preferData = new PreferData(mContext);
	}
	
	/**
	 * 生成JSON的注册字符串
	 */
	private String generateRegisterJson(String username, String pwd, String email) {
		String newPwd = username+":"+Value.realm+":"+pwd;
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_Registration");
			jsonObj.put("UserName", username);
			jsonObj.put("Pwd", Utils.CreateMD5Pwd(newPwd));
			jsonObj.put("Email", email);
			return jsonObj.toString();
		} catch (JSONException e) {
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
				case IS_REGISTERING:
					if (mDialog == null) {
						mDialog = Utils.createLoadingDialog(mContext, getResources().getString(R.string.is_Registering));
						mDialog.show();
					}
					break;
				case REGISTER_TIMEOUT:
					if ((mDialog != null) && (mDialog.isShowing())) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (handler.hasMessages(REGISTER_TIMEOUT)) {
						handler.removeMessages(REGISTER_TIMEOUT);
					}
					Toast.makeText(mContext, getResources().getString(R.string.registration_fails_timeout), Toast.LENGTH_SHORT).show();
					break;
				case R.id.get_realm_id:
					if (msg.arg1 == 0) {
						sendRegisterJsonData();
						System.out.println("MyDebug: 注册操作：获得realm成功，发送注册信息！");
					} else {
						System.out.println("MyDebug: 注册操作：获得realm失败，重新发送获得realm信息！");
						String data = MainApplication.getInstance().generateRealmJson();
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
					}
					break;
				case R.id.register_id:
					if (handler.hasMessages(REGISTER_TIMEOUT)) {
						handler.removeMessages(REGISTER_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							Toast.makeText(mContext, getResources().getString(R.string.registration_is_successful), Toast.LENGTH_SHORT).show();
							if (preferData.isExist("UserName")) {
								preferData.deleteItem("UserName");
							}
							preferData.writeData("UserName", userName);
							if (preferData.isExist("UserPwd")) {
								preferData.deleteItem("UserPwd");
							}
							if (preferData.isExist("AutoLogin")) {
								preferData.deleteItem("AutoLogin");
							}
						} else {
							Toast.makeText(mContext, getResources().getString(R.string.registration_Failed)+","+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.register_id);
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
	
	private void sendRegisterJsonData() {
		String data = generateRegisterJson(userName, userPwd, userEmail);
		if (data != null) {
			sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mContext, getResources().getString(R.string.encrypted_data_error), Toast.LENGTH_SHORT).show();
		}
	}
	
	private void clickRegisterEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkRegisterData()) {
				String data = MainApplication.getInstance().generateRealmJson();
				sendHandlerMsg(IS_REGISTERING);
				sendHandlerMsg(REGISTER_TIMEOUT, Value.REQ_TIME_10S);
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
			case R.id.btn_register_back:
				finish();
				break;
			case R.id.btn_register_username_del:
				et_name.setText("");
				break;
			case R.id.btn_register_email_del:
				et_email.setText("");
				break;
			case R.id.btn_register_password_del:
				et_pwd.setText("");
				break;
			case R.id.btn_register_repassword_del:
				et_repwd.setText("");
				break;
			case R.id.btn_register:
				clickRegisterEvent();
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
		userEmail = et_email.getText().toString().trim();
		userPwd = et_pwd.getText().toString().trim();
		userRepwd = et_repwd.getText().toString().trim();
		
		if (userName.equals("")) {
			resultFlag = false;
			Toast.makeText(mContext, getResources().getString(R.string.please_enter_the_user_name), Toast.LENGTH_SHORT).show();
		}
		else if (Utils.isChineseString(userName)) {
			resultFlag = false;
			Toast.makeText(mContext,getResources().getString(R.string.not_support_Chinese), Toast.LENGTH_SHORT).show();
		}
		else if ((userName.length()<3) || (userName.length()>20)) {
			resultFlag = false;
			Toast.makeText(mContext, getResources().getString(R.string.the_length_range_of_user_name), Toast.LENGTH_SHORT).show();
		} else {
			resultFlag = true;
			if (userEmail.equals("")) {
				resultFlag = false;
				Toast.makeText(mContext, getResources().getString(R.string.please_enter_the_email), Toast.LENGTH_SHORT).show();
			}
			else if (Utils.isChineseString(userEmail)) {
				resultFlag = false;
				Toast.makeText(mContext, getResources().getString(R.string.not_support_Chinese), Toast.LENGTH_SHORT).show();
			}
			else if ((userEmail.length()<6) || (userEmail.length()>20)) {
				resultFlag = false;
				Toast.makeText(mContext, getResources().getString(R.string.email_lengths_range), Toast.LENGTH_SHORT).show();
			}
			else if (userEmail.indexOf("@")<=0) {
				resultFlag = false;
				Toast.makeText(mContext, getResources().getString(R.string.email_format_is_incorrect), Toast.LENGTH_SHORT).show();
			}
			else if (userEmail.indexOf(".")<=0) {
				resultFlag = false;
				Toast.makeText(mContext, getResources().getString(R.string.email_format_is_incorrect), Toast.LENGTH_SHORT).show();
			} else {
				resultFlag = true;
				if (userPwd.equals("")) {
					resultFlag = false;
					Toast.makeText(mContext, getResources().getString(R.string.please_enter_password), Toast.LENGTH_SHORT).show();
				}
				else if (Utils.isChineseString(userPwd)) {
					resultFlag = false;
					Toast.makeText(mContext, getResources().getString(R.string.not_support_Chinese), Toast.LENGTH_SHORT).show();
				}
				else if ((userPwd.length()<6) || (userPwd.length()>20)) {
					resultFlag = false;
					Toast.makeText(mContext, getResources().getString(R.string.password_length), Toast.LENGTH_SHORT).show();
				} else {
					resultFlag = true;
					if (userRepwd.equals("")) {
						resultFlag = false;
						Toast.makeText(mContext, getResources().getString(R.string.please_enter_the_new_password_again), Toast.LENGTH_SHORT).show();
					}
					else if (Utils.isChineseString(userRepwd)) {
						resultFlag = false;
						Toast.makeText(mContext, getResources().getString(R.string.not_support_Chinese), Toast.LENGTH_SHORT).show();
					}
					else if ((userRepwd.length()<6) || (userPwd.length()>20)) {
						resultFlag = false;
						Toast.makeText(mContext, getResources().getString(R.string.confirm_password_length), Toast.LENGTH_SHORT).show();
					}
					else if (!userPwd.equals(userRepwd)) {
						resultFlag = false;
						Toast.makeText(mContext, getResources().getString(R.string.two_passwords_do_not_match), Toast.LENGTH_SHORT).show();
					} else {
						resultFlag = true;
					}
				}
			}
		}
		return resultFlag;
	}
}
