package com.video.user;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
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
	
	private EditText et_name;
	private EditText et_email;
	private Button btn_submit;
	private Button button_delete_username;
	private Button button_delete_email;
	private Dialog mDialog = null;
	
	private String userName = "";
	private String userEmail = "";
	
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
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
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
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
		});
		
		btn_submit = (Button)super.findViewById(R.id.btn_find_pwd_submit);
		btn_submit.setOnClickListener(this);
	}

	private void initData() {
		mContext = FindPwdActivity.this;
		ZmqHandler.setHandler(handler);
	}
	
	/**
	 * 生成JSON的注册字符串
	 */
	private String generateFindPwdJson(String username, String email) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ResetPwd");
			jsonObj.put("UserName", username);
			jsonObj.put("Email", email);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 显示操作的提示
	 */
	private void showHandleDialog(String info) {
		AlertDialog aboutDialog = new AlertDialog.Builder(mContext)
				.setTitle("温馨提示")
				.setMessage(info)
				.setCancelable(false)
				.setPositiveButton("确定",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int whichButton) {
								dialog.dismiss();
							}
						}).create();
		aboutDialog.show();
	}
	
	private Handler handler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_SUBMITTING:
					mDialog = Utils.createLoadingDialog(mContext, "正在找回...");
					mDialog.show();
					break;
				case SUBMIT_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					if (handler.hasMessages(SUBMIT_TIMEOUT)) {
						handler.removeMessages(SUBMIT_TIMEOUT);
					}
					Toast.makeText(mContext, "找回密码失败，网络超时！", Toast.LENGTH_SHORT).show();
					break;
				case R.id.find_pwd_id:
					if (handler.hasMessages(SUBMIT_TIMEOUT)) {
						handler.removeMessages(SUBMIT_TIMEOUT);
						if (mDialog != null)
							mDialog.dismiss();
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							showHandleDialog("恭喜您！找回密码成功，服务器已将您的密码重置为【123456】！");
						} else {
							Toast.makeText(mContext, "找回密码失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
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
				Handler sendHandler = ZmqThread.zmqThreadHandler;
				String data = generateFindPwdJson(userName, userEmail);
				sendHandlerMsg(IS_SUBMITTING);
				sendHandlerMsg(SUBMIT_TIMEOUT, Value.requestTimeout);
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
			case R.id.btn_find_back:
				finish();
				break;
			case R.id.btn_find_username_del:
				et_name.setText("");
				break;
			case R.id.btn_find_email_del:
				et_email.setText("");
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
		
		if (userName.equals("")) {
			resultFlag = false;
			et_name.setError("请输入用户名！");
		}
		else if ((userName.length()<3) || (userName.length()>20)) {
			resultFlag = false;
			et_name.setError("用户名长度范围3~20！");
		} else {
			resultFlag = true;
			if (userEmail.equals("")) {
				resultFlag = false;
				et_email.setError("请输入电子邮箱！");
			}
			else if ((userEmail.length()<6) || (userEmail.length()>20)) {
				resultFlag = false;
				et_email.setError("电子邮箱长度范围6~20！");
			}
			else if (userEmail.indexOf("@")<=0) {
				resultFlag = false;
				et_email.setError("邮箱格式不正确！");
			}
			else if (userEmail.indexOf(".")<=0) {
				resultFlag = false;
				et_email.setError("邮箱格式不正确！");
			} else {
				resultFlag = true;
			}
		}
		return resultFlag;
	}
}
