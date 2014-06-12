package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.play.TunnelCommunication;
import com.video.socket.HandlerApplication;
import com.video.user.LoginActivity;
import com.video.user.ModifyPwdActivity;
import com.video.utils.OkCancelDialog;
import com.video.utils.Utils;
import com.video.utils.WiFiAlertDialog;

public class MoreFragment extends Fragment implements OnClickListener {

	private FragmentActivity mActivity;
	private View mView;
	private Dialog mDialog = null;
	
	private LocalDeviceReceiver localDeviceReceiver = null;
	private ArrayList<HashMap<String, String>> localDeviceList = null;
	private WiFiAlertDialog localDeviceDialog = null;
	private LocalDeviceAdapter localDeviceAdapter = null;
	private final int SEARCH_TIMEOUT = 1;
	
	private Button button_logout;
	private PreferData preferData = null;
	private String userName = null;
	
	private boolean isRememberPwd = true;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		return inflater.inflate(R.layout.more, container, false);
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		mActivity = getActivity();
		mView = getView();
		
		initView();
		initData();
	}
	
	private void initView() {
		Button button_modify_pwd = (Button)mView.findViewById(R.id.btn_modify_pwd);
		button_modify_pwd.setOnClickListener(this);
		
		Button button_setting = (Button)mView.findViewById(R.id.btn_setting);
		button_setting.setOnClickListener(this);
		
		Button button_local_device = (Button)mView.findViewById(R.id.btn_local_device);
		button_local_device.setOnClickListener(this);
		
		Button button_wifi = (Button)mView.findViewById(R.id.btn_wifi);
		button_wifi.setOnClickListener(this);
		
		Button button_help = (Button)mView.findViewById(R.id.btn_help);
		button_help.setOnClickListener(this);
		
		Button button_about = (Button)mView.findViewById(R.id.btn_about);
		button_about.setOnClickListener(this);
		
		button_logout = (Button)mView.findViewById(R.id.btn_logout);
		button_logout.setOnClickListener(this);
 	}
	
	private void initData() {
		
		//注册广播
		localDeviceReceiver = new LocalDeviceReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(Value.SEARCH_LOCAL_DEVICE_ACTION);
		mActivity.registerReceiver(localDeviceReceiver, filter);
		
		preferData = new PreferData(mActivity);
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
	}
	
	/**
	 * 生成JSON的注销登录字符串
	 */
	private String generateLogoutJson() {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_Logout");
			jsonObj.put("UserName", userName);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 显示操作的提示
	 */
	private void showHandleDialog() {
		final OkCancelDialog myDialog=new OkCancelDialog(mActivity);
		myDialog.setTitle("温馨提示");
		myDialog.setMessage("确认退出当前账号的登录？");
		myDialog.setPositiveButton("确定", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
				Value.resetValues();
				ExitLogoutAPP();
			}
		});

		myDialog.setNegativeButton("取消", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
			}
		});
	}
	
	/**
	 * 发送Handler消息
	 */
	private void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	private void sendHandlerMsg(int what, int timeout) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessageDelayed(msg, timeout);
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case SEARCH_TIMEOUT:
					if (mDialog.isShowing())
						mDialog.dismiss();
					if (handler.hasMessages(SEARCH_TIMEOUT)) {
						handler.removeMessages(SEARCH_TIMEOUT);
					}
					Toast.makeText(mActivity, "搜索完毕，暂无本地设备", Toast.LENGTH_SHORT).show();
					break;
			}
		}
	};
	
	/**
	 * 退出当前账号登录的处理
	 */
	private void ExitLogoutAPP() {
		if (preferData.isExist("RememberPwd")) {
			isRememberPwd = preferData.readBoolean("RememberPwd");
		}
		if (!isRememberPwd) {
			if (preferData.isExist("UserPwd")) {
				preferData.deleteItem("UserPwd");
			}
			if (preferData.isExist("AutoLogin")) {
				preferData.deleteItem("AutoLogin");
			}
		}
		Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
		String data = generateLogoutJson();
		sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
		startActivity(new Intent(mActivity, LoginActivity.class));
		mActivity.finish();
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_modify_pwd:
				startActivity(new Intent(mActivity, ModifyPwdActivity.class));
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_setting:
				startActivity(new Intent(mActivity, SettingsActivity.class));
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_local_device:
				TunnelCommunication.getInstance().searchLocalDevice();
				mDialog = Utils.createLoadingDialog(mActivity, "正在搜索设备...");
				mDialog.show();
				sendHandlerMsg(SEARCH_TIMEOUT, Value.REQ_TIME_10S);
				break;
			case R.id.btn_wifi:
				startActivity(new Intent(mActivity, WiFiActivity.class));
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_help:
				startActivity(new Intent(mActivity, HelpActivity.class));
				break;
			case R.id.btn_about:
				startActivity(new Intent(mActivity, AboutActivity.class));
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_logout:
				showHandleDialog();
				break;
		}
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		mActivity.unregisterReceiver(localDeviceReceiver);
	}

	/**
	 * 显示本地设备对话框
	 */
	private void showLocalDeviceList() {
		localDeviceDialog = new WiFiAlertDialog(mActivity);
		localDeviceDialog.setTitle("本地设备列表");
		localDeviceAdapter = new LocalDeviceAdapter(mActivity, localDeviceList);
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
			if ((action.equals(Value.SEARCH_LOCAL_DEVICE_ACTION)) && (intent != null)) {
				
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

