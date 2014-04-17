package com.video.user;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
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
	private ProgressDialog progressDialog;
	
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
		et_name = (EditText)super.findViewById(R.id.et_register_name);
		et_email = (EditText)super.findViewById(R.id.et_register_email);
		et_pwd = (EditText)super.findViewById(R.id.et_register_pwd);
		et_repwd = (EditText)super.findViewById(R.id.et_register_repwd);
		
		btn_register = (Button)super.findViewById(R.id.btn_register);
		btn_register.setOnClickListener(this);
	}

	private void initData() {
		mContext = RegisterActivity.this;
		ZmqHandler.setHandler(handler);
		
		preferData = new PreferData(mContext);
	}
	
	/**
	 * 生成JSON的注册字符串
	 */
	private String generateRegisterJson(String username, String pwd, String email) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_Registration");
			jsonObj.put("UserName", username);
			jsonObj.put("Pwd", Utils.CreateMD5Pwd(pwd));
			jsonObj.put("Email", email);
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
				case IS_REGISTERING:
					showProgressDialog("正在注册... ");
					break;
				case REGISTER_TIMEOUT:
					if (progressDialog != null)
						progressDialog.dismiss();
					showHandleDialog("注册失败，网络超时！");
					if (handler.hasMessages(REGISTER_TIMEOUT)) {
						handler.removeMessages(REGISTER_TIMEOUT);
					}
					break;
				case R.id.register_id:
					if (handler.hasMessages(REGISTER_TIMEOUT)) {
						handler.removeMessages(REGISTER_TIMEOUT);
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							if (progressDialog != null)
								progressDialog.dismiss();
							showHandleDialog("恭喜您，注册成功！");
							if (preferData.isExist("UserName")) {
								preferData.deleteItem("UserName");
								preferData.writeData("UserName", userName);
							} else {
								preferData.writeData("UserName", userName);
							}
							if (preferData.isExist("UserPwd")) {
								preferData.deleteItem("UserPwd");
							}
							if (preferData.isExist("AutoLogin")) {
								preferData.deleteItem("AutoLogin");
							}
						} else {
							if (progressDialog != null)
								progressDialog.dismiss();
							showHandleDialog("注册失败，"+Utils.getErrorReason(resultCode));
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
	
	private void clickRegisterEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkRegisterData()) {
				Handler sendHandler = ZmqThread.zmqThreadHandler;
				String data = generateRegisterJson(userName, userPwd, userEmail);
				sendHandlerMsg(IS_REGISTERING);
				sendHandlerMsg(REGISTER_TIMEOUT, Value.requestTimeout);
				sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
			}
		} else {
			showHandleDialog("没有可用的网络连接，请确认后重试！");
		}
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
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
				if (userPwd.equals("")) {
					resultFlag = false;
					et_pwd.setError("请输入密码！");
				}
				else if ((userPwd.length()<6) || (userPwd.length()>20)) {
					resultFlag = false;
					et_pwd.setError("密码长度范围6~20！");
				} else {
					resultFlag = true;
					if (userRepwd.equals("")) {
						resultFlag = false;
						et_repwd.setError("请输入确认密码！");
					}
					else if ((userRepwd.length()<6) || (userPwd.length()>20)) {
						resultFlag = false;
						et_repwd.setError("确认密码长度范围6~20！");
					}
					else if (!userPwd.equals(userRepwd)) {
						resultFlag = false;
						et_repwd.setError("两次输入的密码不一致！");
					} else {
						resultFlag = true;
					}
				}
			}
		}
		return resultFlag;
	}
}
