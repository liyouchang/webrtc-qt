package com.video.user;

import org.json.JSONException;
import org.json.JSONObject;

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
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqHandler;
import com.video.utils.OkCancelDialog;
import com.video.utils.Utils;

public class ModifyPwdActivity extends Activity implements OnClickListener {

	private Context mContext;
	private PreferData preferData = null;
	
	private EditText et_old_pwd;
	private EditText et_new_pwd;
	private EditText et_new_repwd;
	private Button button_delete_old_password;
	private Button button_delete_new_password;
	private Button button_delete_new_repassword;
	private Dialog mDialog = null;
	
	private String userName = "";
	private String userPwd = "";
	private String userOldPwd = "";
	private String userNewPwd = "";
	private String userNewRepwd = "";
	
	private final int IS_MODIFYING = 1;
	private final int MODIFY_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.modify_pwd);
		
		initView();
		initData();
	}

	private void initView() {
		ImageButton button_back = (ImageButton) super.findViewById(R.id.ib_modify_back);
		button_back.setOnClickListener(this);
		
		button_delete_old_password = (Button) this.findViewById(R.id.btn_modify_old_password_del);
		button_delete_old_password.setOnClickListener(this);
		
		button_delete_new_password = (Button) this.findViewById(R.id.btn_modify_new_password_del);
		button_delete_new_password.setOnClickListener(this);
		
		button_delete_new_repassword = (Button) this.findViewById(R.id.btn_modify_new_repassword_del);
		button_delete_new_repassword.setOnClickListener(this);
		
		et_old_pwd = (EditText)super.findViewById(R.id.et_modify_old_password);
		et_old_pwd.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_old_password.setVisibility(View.INVISIBLE);
				} else {
					button_delete_old_password.setVisibility(View.VISIBLE);
				}
			}
			
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
		});
		
		et_new_pwd = (EditText)super.findViewById(R.id.et_modify_new_password);
		et_new_pwd.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_new_password.setVisibility(View.INVISIBLE);
				} else {
					button_delete_new_password.setVisibility(View.VISIBLE);
				}
			}
			
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
		});
		
		et_new_repwd = (EditText)super.findViewById(R.id.et_modify_new_repassword);
		et_new_repwd.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_new_repassword.setVisibility(View.INVISIBLE);
				} else {
					button_delete_new_repassword.setVisibility(View.VISIBLE);
				}
			}
			
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
		});
		
		Button button_submit = (Button) super.findViewById(R.id.btn_modify_pwd_submit);
		button_submit.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = ModifyPwdActivity.this;
		ZmqHandler.setHandler(handler);
		preferData = new PreferData(mContext);
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		
		if (preferData.isExist("UserPwd")) {
			userPwd = preferData.readString("UserPwd");
		}
	}
	
	/**
	 * 生成JSON的注册字符串
	 */
	private String generateModifyPwdJson(String username, String oldpwd, String newpwd) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ChangePwd");
			jsonObj.put("UserName", username);
			jsonObj.put("OldPwd", Utils.CreateMD5Pwd(oldpwd));
			jsonObj.put("NewPwd", Utils.CreateMD5Pwd(newpwd));
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
				case IS_MODIFYING:
					mDialog = Utils.createLoadingDialog(mContext, "正在提交...");
					mDialog.show();
					break;
				case MODIFY_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					if (handler.hasMessages(MODIFY_TIMEOUT)) {
						handler.removeMessages(MODIFY_TIMEOUT);
					}
					Toast.makeText(mContext, "修改密码失败，网络超时！", Toast.LENGTH_SHORT).show();
					break;
				case R.id.modify_pwd_id:
					if (handler.hasMessages(MODIFY_TIMEOUT)) {
						handler.removeMessages(MODIFY_TIMEOUT);
						if (mDialog != null)
							mDialog.dismiss();
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							final OkCancelDialog myDialog2=new OkCancelDialog(mContext);
							myDialog2.setTitle("温馨提示");
							myDialog2.setMessage("修改密码成功，是否重新登录？");
							myDialog2.setPositiveButton("确认", new OnClickListener() {
								@Override
								public void onClick(View v) {
									myDialog2.dismiss();
									if (preferData.isExist("UserPwd")) {
										preferData.deleteItem("UserPwd");
									}
									startActivity(new Intent(mContext, LoginActivity.class));
									ModifyPwdActivity.this.finish();
								}
							});
							myDialog2.setNegativeButton("取消", new OnClickListener() {
								@Override
								public void onClick(View v) {
									myDialog2.dismiss();
									ModifyPwdActivity.this.finish();
								}
							});
						} else {
							Toast.makeText(mContext, "修改密码失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.modify_pwd_id);
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
	
	private void clickModifyPwdEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkModifyPwdData()) {
				Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
				String data = generateModifyPwdJson(userName, userOldPwd, userNewPwd);
				sendHandlerMsg(IS_MODIFYING);
				sendHandlerMsg(MODIFY_TIMEOUT, Value.requestTimeout);
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
			case R.id.ib_modify_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
				break;
			case R.id.btn_modify_old_password_del:
				et_old_pwd.setText("");
				break;
			case R.id.btn_modify_new_password_del:
				et_new_pwd.setText("");	
				break;
			case R.id.btn_modify_new_repassword_del:
				et_new_repwd.setText("");
				break;
			case R.id.btn_modify_pwd_submit:
				clickModifyPwdEvent();
				break;
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
		}
		return super.onKeyDown(keyCode, event);
	}
	
	/**
	 * @return true:修改密码格式正确  false:修改密码格式错误
	 */
	private boolean checkModifyPwdData() {
		boolean resultFlag = false;
		
		//获取EditText输入框的字符串
		userOldPwd = et_old_pwd.getText().toString().trim();
		userNewPwd = et_new_pwd.getText().toString().trim();
		userNewRepwd = et_new_repwd.getText().toString().trim();
		
		if (userOldPwd.equals("")) {
			resultFlag = false;
			et_old_pwd.setError("请输入旧密码！");
		}
		else if (!userOldPwd.equals(userPwd)) {
			resultFlag = false;
			et_old_pwd.setError("旧密码输入错误！");
		} else {
			resultFlag = true;
			if (userNewPwd.equals("")) {
				resultFlag = false;
				et_new_pwd.setError("请输入新密码！");
			}
			else if ((userNewPwd.length()<6) || (userNewPwd.length()>20)) {
				resultFlag = false;
				et_new_pwd.setError("密码长度范围6~20！");
			} else {
				resultFlag = true;
				if (userNewRepwd.equals("")) {
					resultFlag = false;
					et_new_repwd.setError("请再次输入新密码！");
				}
				else if ((userNewRepwd.length()<6) || (userNewRepwd.length()>20)) {
					resultFlag = false;
					et_new_repwd.setError("确认密码长度范围6~20！");
				}
				else if (!userNewPwd.equals(userNewRepwd)) {
					resultFlag = false;
					et_new_repwd.setError("两次输入的密码不一致！");
				} else {
					resultFlag = true;
				}
			}
		}
		return resultFlag;
	}
}
