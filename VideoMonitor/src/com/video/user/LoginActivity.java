package com.video.user;

import java.util.ArrayList;
import java.util.HashMap;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.main.APSearchActivity;
import com.video.main.MainActivity;
import com.video.play.TunnelCommunication;
import com.video.service.BackstageService;
import com.video.service.MainApplication;
import com.video.socket.ZmqCtrl;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.UpdateAPK;
import com.video.utils.Utils;
import com.video.utils.WiFiAlertDialog;

@SuppressLint("HandlerLeak")
public class LoginActivity extends Activity implements OnClickListener {

	private Context mContext;
	private PreferData preferData = null;
	
	private EditText et_name;
	private EditText et_pwd;
	private boolean isAutoLogin = false;
	private CheckBox cb_auto_login;
	private int loginTimes = 0;
	
	private Button button_delete_username;
	private Button button_delete_password;
	private Dialog mDialog = null;
	
	private String userName = "";
	private String userPwd = "";
	
	private final int IS_LOGINNING = 1;
	private final int LOGIN_TIMEOUT = 2;
	private final int LOGIN_AGAIN = 3;
	private final int SEARCH_TIMEOUT = 4;
	
	private LocalDeviceReceiver localDeviceReceiver = null;
	private ArrayList<HashMap<String, String>> localDeviceList = null;
	private WiFiAlertDialog localDeviceDialog = null;
	private LocalDeviceAdapter localDeviceAdapter = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.login);
		
		initView();
		
		if (!Value.isManulLogout) {
			new UpdateAPK(LoginActivity.this).startCheckUpgadeThread();
		}
	}
	
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		initData();
	}
	
	private void initView() {
		mContext = LoginActivity.this;
		//注册广播
		localDeviceReceiver = new LocalDeviceReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(BackstageService.SEARCH_LOCAL_DEVICE_ACTION);
		registerReceiver(localDeviceReceiver, filter);
		
		cb_auto_login = (CheckBox) super.findViewById(R.id.cb_auto_login);
		cb_auto_login.setOnCheckedChangeListener(new onCheckedChangeListenerImpl());
		
		Button button_login = (Button) super.findViewById(R.id.btn_login);
		button_login.setOnClickListener(this);
		
		Button button_register = (Button)super.findViewById(R.id.btn_login_regist);
		button_register.setOnClickListener(this);
		
		Button button_find_pwd = (Button)super.findViewById(R.id.btn_login_find);
		button_find_pwd.setOnClickListener(this);
		
		button_delete_username = (Button)super.findViewById(R.id.btn_login_username_del);
		button_delete_username.setOnClickListener(this);
		
		button_delete_password = (Button)super.findViewById(R.id.btn_login_password_del);
		button_delete_password.setOnClickListener(this);
		
		et_name = (EditText) super.findViewById(R.id.et_login_username);
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
		
		et_pwd = (EditText) super.findViewById(R.id.et_login_password);
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
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
		});
		ImageView login_demo = (ImageView) this.findViewById(R.id.iv_login_demo);
		login_demo.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = LoginActivity.this;
		ZmqCtrl.getInstance().init();
		ZmqHandler.mHandler = handler;

		if (preferData == null) {
			preferData = new PreferData(mContext);
		}
		
		if (preferData.isExist("AutoLogin")) {
			isAutoLogin = preferData.readBoolean("AutoLogin");
		}
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
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_LOGINNING:
					if ((mDialog == null) || (!mDialog.isShowing())) {
						mDialog = Utils.createLoadingDialog(mContext, getResources().getString(R.string.is_logging_in));
						mDialog.show();
					}
					break;
				case LOGIN_TIMEOUT:
					loginTimes ++;
					if (handler.hasMessages(LOGIN_TIMEOUT)) {
						handler.removeMessages(LOGIN_TIMEOUT);
					}
					// 终止主程序和服务广播
					MainApplication.getInstance().stopActivityandService();
					sendHandlerMsg(LOGIN_AGAIN, 3000);
					break;
				case LOGIN_AGAIN:
					if (loginTimes > 1) {
						loginTimes = 0;
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						Toast.makeText(mContext, getResources().getString(R.string.Login_timeout), Toast.LENGTH_SHORT).show();
					} else {
						ZmqCtrl.getInstance().init();
						String data = MainApplication.getInstance().generateLoginJson(userName, userPwd);
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
						sendHandlerMsg(LOGIN_TIMEOUT, Value.REQ_TIME_6S);
					}
					break;
				case SEARCH_TIMEOUT:
					if ((mDialog != null) && (mDialog.isShowing())) {
						Toast.makeText(mContext, getResources().getString(R.string.no_local_device), Toast.LENGTH_SHORT).show();
						mDialog.dismiss();
					}
					if (handler.hasMessages(SEARCH_TIMEOUT)) {
						handler.removeMessages(SEARCH_TIMEOUT);
					}
					break;
				case R.id.get_realm_id:
					if (msg.arg1 == 0) {
						sendLoginJsonData();
						System.out.println("MyDebug: 登录操作：获得realm成功！");
					} else {
						System.out.println("MyDebug: 登录操作：获得realm失败！");
						String data = MainApplication.getInstance().generateRealmJson();
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
					}
					break;
				case R.id.login_id:
					if (handler.hasMessages(LOGIN_TIMEOUT)) {
						handler.removeMessages(LOGIN_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						loginTimes = 0;
						if (msg.arg1 == 0) {
							//登录成功
							Value.isLoginSuccess = true;
							startActivity(new Intent(mContext, MainActivity.class));
							LoginActivity.this.finish();
							// 初始化IceServers
							TunnelCommunication.getInstance().tunnelInitialize(MainApplication.getInstance().generateIceServersJson(Value.stun, Value.turn, userName, userPwd));
						} else {
							Toast.makeText(mContext,getResources().getString(R.string.login_failed)+","+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
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
	
	private class onCheckedChangeListenerImpl implements OnCheckedChangeListener {

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			// TODO Auto-generated method stub
			if (preferData == null) {
				preferData = new PreferData(mContext);
			}
			preferData.writeData("AutoLogin", isChecked);
		}
	}
	
	public void sendLoginJsonData() {
		if (checkRegisterData()) {
			if (preferData.isExist("UserName")) {
				preferData.deleteItem("UserName");
			}
			if (preferData.isExist("UserPwd")) {
				preferData.deleteItem("UserPwd");
			}
			preferData.writeData("UserName", userName);
			preferData.writeData("UserPwd", userPwd);
			MainApplication.getInstance().userName = userName;
			MainApplication.getInstance().userPwd = userPwd;
			String data = MainApplication.getInstance().generateLoginJson(userName, userPwd);
			sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
		}
	}
	
	private void clickLoginEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkRegisterData()) {
				ZmqCtrl.getInstance().init();
				String data = MainApplication.getInstance().generateRealmJson();
				sendHandlerMsg(IS_LOGINNING);
				sendHandlerMsg(LOGIN_TIMEOUT, Value.REQ_TIME_6S);
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
			case R.id.btn_login:
				clickLoginEvent();
				break;
			case R.id.btn_login_regist:
				ZmqCtrl.getInstance().init();
				startActivity(new Intent(this, RegisterActivity.class));
				break;
			case R.id.btn_login_find:
				ZmqCtrl.getInstance().init();
				startActivity(new Intent(this, FindPwdActivity.class));
				break;
			case R.id.btn_login_username_del:
				et_name.setText("");
				break;
			case R.id.btn_login_password_del:
				et_pwd.setText("");
				break;
			case R.id.iv_login_demo:
//				ZmqCtrl.getInstance().init();
//				TunnelCommunication.getInstance().searchLocalDevice();
//				mDialog = Utils.createLoadingDialog(mContext, "正在搜索设备...");
//				mDialog.show();
//				sendHandlerMsg(SEARCH_TIMEOUT, Value.REQ_TIME_10S);
				startActivity(new Intent(mContext, APSearchActivity.class));
				overridePendingTransition(R.anim.up_in, R.anim.fragment_nochange);
				break;
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if ((mDialog != null) && (mDialog.isShowing())) {
			mDialog.dismiss();
			mDialog = null;
		}
		if (handler.hasMessages(LOGIN_TIMEOUT)) {
			handler.removeMessages(LOGIN_TIMEOUT);
		}
		if (handler.hasMessages(R.id.login_id)) {
			handler.removeMessages(R.id.login_id);
		}
		return super.onKeyDown(keyCode, event);
	}
	
	/**
	 * @return true:注册信息格式正确  false:注册信息格式错误
	 */
	private boolean checkRegisterData() {
		boolean resultFlag = false;
		
		//获取EditText输入框的字符串
		String name = et_name.getText().toString().trim();
		String pwd = et_pwd.getText().toString().trim();
		
		if (name.equals("")) {
			resultFlag = false;
			Toast.makeText(mContext, getResources().getString(R.string.please_enter_the_user_name), Toast.LENGTH_SHORT).show();
		}
		else if (Utils.isChineseString(name)) {
			resultFlag = false;
			Toast.makeText(mContext, getResources().getString(R.string.not_support_Chinese), Toast.LENGTH_SHORT).show();
		}
		else if ((name.length()<3) || (name.length()>20)) {
			resultFlag = false;
			Toast.makeText(mContext, getResources().getString(R.string.the_length_range_of_user_name), Toast.LENGTH_SHORT).show();
		} else {
			resultFlag = true;
			userName = name;
			if (pwd.equals("")) {
				resultFlag = false;
				Toast.makeText(mContext, getResources().getString(R.string.please_enter_password), Toast.LENGTH_SHORT).show();
			}
			else if (Utils.isChineseString(pwd)) {
				resultFlag = false;
				Toast.makeText(mContext, getResources().getString(R.string.not_support_Chinese), Toast.LENGTH_SHORT).show();
			}
			else if ((pwd.length()<6) || (pwd.length()>20)) {
				resultFlag = false;
				Toast.makeText(mContext, getResources().getString(R.string.password_length), Toast.LENGTH_SHORT).show();
			} else {
				resultFlag = true;
				userPwd = pwd;
			}
		}
		return resultFlag;
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		unregisterReceiver(localDeviceReceiver);
		Value.isManulLogout = false;
	}
	
	/**
	 * 显示本地设备对话框
	 */
	private void showLocalDeviceList() {
		localDeviceDialog = new WiFiAlertDialog(mContext);
		localDeviceDialog.setTitle(getResources().getString(R.string.local_device_list));
		localDeviceAdapter = new LocalDeviceAdapter(mContext, localDeviceList);
		localDeviceDialog.setAdapter(localDeviceAdapter);
		localDeviceDialog.setOnItemClickListenerLocalDevice(localDeviceList);
	}
	
	/**
	 * 搜索本地设备的适配器
	 */
	private class LocalDeviceAdapter extends BaseAdapter {
		
		private Context context;
		private ArrayList<HashMap<String, String>> list = null;

		public LocalDeviceAdapter(Context context, ArrayList<HashMap<String, String>> list) {
			this.context = context;
			this.list = list;
		}

		@Override
		public int getCount() {
			return list.size();
		}

		@Override
		public Object getItem(int position) {
			return list.get(position);
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup viewGroup) {
			ViewHolder holder;

			if (convertView == null) {
				convertView = LayoutInflater.from(context).inflate(R.layout.local_device_item, null);
				holder = new ViewHolder();
				convertView.setTag(holder);
				holder.tv_local_device_name = (TextView) convertView.findViewById(R.id.tv_local_device_name);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			if (holder.tv_local_device_name != null) {
				holder.tv_local_device_name.setText((CharSequence) list.get(position).get("MAC"));
			}
			return convertView;
		}

		class ViewHolder {
			TextView tv_local_device_name;
		}
	}


	/**
	 * @author sunfusheng
	 * 本地设备的广播接收
	 */
	public class LocalDeviceReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			boolean isRepeat = false;
			if ((mDialog != null) && (mDialog.isShowing())) {
				mDialog.dismiss();
			}
			if (handler.hasMessages(SEARCH_TIMEOUT)) {
				handler.removeMessages(SEARCH_TIMEOUT);
			}
			if (localDeviceList == null) {
				localDeviceList = new ArrayList<HashMap<String, String>>();
			}
			String action = intent.getAction();
			if ((action.equals(BackstageService.SEARCH_LOCAL_DEVICE_ACTION)) && (intent != null)) {
				
				String mac = (String) intent.getCharSequenceExtra("MAC");
				String ip = (String) intent.getCharSequenceExtra("IP");
				int port = intent.getIntExtra("Port", 22616);
				String gateway = (String) intent.getCharSequenceExtra("Gateway");
				String mask = (String) intent.getCharSequenceExtra("Mask");
				
				for (int i=0; i<localDeviceList.size(); i++) {
					if (mac.equals(localDeviceList.get(i).get("MAC"))) {
						isRepeat = true;
					}
				}
				if (!isRepeat) {
					HashMap<String, String> item = new HashMap<String, String>();
					item.put("MAC", mac);
					item.put("IP", ip);
					item.put("Port", ""+port);
					item.put("Gateway", gateway);
					item.put("Mask", mask);
					localDeviceList.add(item);
				} else {
					isRepeat = false;
				}
				if (localDeviceDialog == null) {
					showLocalDeviceList();
				} else {
					if (localDeviceDialog.isShowing()) {
						localDeviceAdapter.notifyDataSetChanged();
					} else {
						showLocalDeviceList();
					}
				}
			}
		}
	}
}
