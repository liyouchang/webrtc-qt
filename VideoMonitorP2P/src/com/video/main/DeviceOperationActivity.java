package com.video.main;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.OkCancelDialog;
import com.video.utils.Utils;

public class DeviceOperationActivity extends Activity implements OnClickListener {

	private String deviceName = null;
	private String deviceId = null;
	private String dealerName = null;
	private String deviceBg = "null";
	private String userName = null;
	
	private Context mContext = null;
	private PreferData preferData = null;
	private Dialog mDialog = null;
	
	private Button device_name = null;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.device_operation);
		
		initView();
		initData();
	}
	
	private void initView() {
		ImageButton back = (ImageButton) this.findViewById(R.id.ib_device_operation_back);
		back.setOnClickListener(this);
		
		device_name = (Button) this.findViewById(R.id.btn_device_name);
		device_name.setOnClickListener(this);
		
		Button share_device = (Button) this.findViewById(R.id.btn_share_device);
		share_device.setOnClickListener(this);
		
		Button set_device_bg = (Button) this.findViewById(R.id.btn_set_device_bg);
		set_device_bg.setOnClickListener(this);
		
		Button delete_device_bg = (Button) this.findViewById(R.id.btn_delete_device_bg);
		delete_device_bg.setOnClickListener(this);
		
		Button delete_device = (Button) this.findViewById(R.id.btn_delete_device);
		delete_device.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = DeviceOperationActivity.this;
		ZmqHandler.mHandler = handler;
		preferData = new PreferData(mContext);
		
		Intent intent = this.getIntent();
		deviceName = (String) intent.getCharSequenceExtra("deviceName");
		deviceId = (String) intent.getCharSequenceExtra("deviceID");
		if (intent.hasExtra("dealerName")) {
			dealerName = (String) intent.getCharSequenceExtra("dealerName");
		}
		
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		
		device_name.setText(deviceName);
	}
	
	/**
	 * 生成JSON的删除背景图片字符串
	 */
	private String generateDeleteImageJson(String mac) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelBackPic");
			jsonObj.put("UserName", userName);
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
				case IS_REQUESTING:
					mDialog = Utils.createLoadingDialog(mContext, (String) msg.obj);
					mDialog.show();
					break;
				case REQUEST_TIMEOUT:
					if (mDialog.isShowing()) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					Value.isNeedReqTermListFlag = true;
					Toast.makeText(mContext, ""+msg.obj, Toast.LENGTH_SHORT).show();
					break;
				//删除终端绑定
				case R.id.delete_device_item_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog.isShowing()) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							Toast.makeText(mContext, "删除终端绑定成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "删除终端绑定失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_device_item_id);
					}
					break;
				//上传背景图片
				case R.id.upload_back_image_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog.isShowing()) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							Toast.makeText(mContext, "上传背景图片成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "上传背景图片失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.upload_back_image_id);
					}
					break;
				//删除背景图片
				case R.id.delete_back_image_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog.isShowing()) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							Toast.makeText(mContext, "删除背景图片成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "删除背景图片失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_back_image_id);
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
		if (handler != null) {
			handler.sendMessage(msg);
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		Intent intent = null;
		switch (v.getId()) {
			case R.id.ib_device_operation_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
				break;
			// 设备名称
			case R.id.btn_device_name:
				intent = new Intent(mContext, ModifyDeviceNameActivity.class);
				intent.putExtra("deviceName", deviceName);
				intent.putExtra("deviceID", deviceId);
				startActivityForResult(intent, 0);
				overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
				break;
			// 分享设备
			case R.id.btn_share_device:	
				intent = new Intent(mContext, AddShareActivity.class);
				intent.putExtra("deviceName", deviceName);
				intent.putExtra("deviceID", deviceId);
				startActivity(intent);
				overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
				break;
			// 设置背景图片
			case R.id.btn_set_device_bg:
				intent = new Intent(mContext, SetDeviceBgActivity.class);
				intent.putExtra("deviceName", deviceName);
				intent.putExtra("deviceID", deviceId);
				startActivityForResult(intent, 1);
				overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
				break;
			// 删除背景图片
			case R.id.btn_delete_device_bg:
				if (deviceBg.equals("null")) {
					Toast.makeText(mContext, "无背景图片，不需要删除！", Toast.LENGTH_SHORT).show();
				} else {
					final OkCancelDialog myDialog1=new OkCancelDialog(mContext);
					myDialog1.setTitle("温馨提示");
					myDialog1.setMessage("确认删除背景图片？");
					myDialog1.setPositiveButton("确认", new OnClickListener() {
						@Override
						public void onClick(View v) {
							myDialog1.dismiss();
							Handler sendHandler = ZmqThread.zmqThreadHandler;
							String data = generateDeleteImageJson(deviceId);
							sendHandlerMsg(IS_REQUESTING, "正在删除图片...");
							sendHandlerMsg(REQUEST_TIMEOUT, "删除图片失败，网络超时！", Value.REQ_TIME_10S);
							sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
						}
					});
					myDialog1.setNegativeButton("取消", new OnClickListener() {
						@Override
						public void onClick(View v) {
							myDialog1.dismiss();
						}
					});
				}
				break;
			// 删除设备
			case R.id.btn_delete_device:
				
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
}
