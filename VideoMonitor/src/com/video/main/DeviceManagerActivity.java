package com.video.main;

import java.io.File;

import org.json.JSONException;
import org.json.JSONObject;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.service.MainApplication;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.OkCancelDialog;
import com.video.utils.Utils;

@SuppressLint("HandlerLeak")
public class DeviceManagerActivity extends Activity implements OnClickListener, OnLongClickListener  {

	private Context mContext;
	private PreferData preferData = null;
	private Dialog mDialog = null;
	private boolean isNeedRefresh = false;
	
	private String userName = "";
	private String deviceBg = "";
	
	private static String mDeviceName = null;
	private static String mDeviceID = null;
	private static String mDeviceBg = null;
	private static String mDealerName = null;
	
	private RelativeLayout rl_device_item;
	private RelativeLayout rl_device_bg;
	private TextView tv_device_name;
	private TextView tv_device_mac;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.device_manager);
		
		initView();
		initData();
	}
	
	@Override
	public void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		mContext = DeviceManagerActivity.this;
		ZmqHandler.mHandler = handler;
	}
	
	private void initView() {
		ImageButton back = (ImageButton) this.findViewById(R.id.ib_device_manager_back);
		back.setOnClickListener(this);
		
		rl_device_item = (RelativeLayout) this.findViewById(R.id.rl_device_item);
		rl_device_item.setOnClickListener(this);
		
		rl_device_bg = (RelativeLayout) this.findViewById(R.id.rl_device_bg);
		rl_device_bg.setOnClickListener(this);
		rl_device_bg.setOnLongClickListener(this);
		
		tv_device_name = (TextView) this.findViewById(R.id.tv_device_name);
		tv_device_mac = (TextView) this.findViewById(R.id.tv_device_mac);
		
		Button btn_wifi = (Button) this.findViewById(R.id.btn_wifi);
		btn_wifi.setOnClickListener(this);
		
		Button btn_shared_device_manager = (Button) this.findViewById(R.id.btn_shared_device_manager);
		btn_shared_device_manager.setOnClickListener(this);
		
		Button btn_delete_device = (Button) this.findViewById(R.id.btn_delete_device);
		btn_delete_device.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = DeviceManagerActivity.this;
		ZmqHandler.mHandler = handler;
		preferData = new PreferData(mContext);
		
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		
		Intent intent = this.getIntent();
		mDeviceName = (String) intent.getCharSequenceExtra("deviceName");
		mDeviceID = (String) intent.getCharSequenceExtra("deviceID");
		mDeviceBg = (String) intent.getCharSequenceExtra("deviceBg");
		mDealerName = (String) intent.getCharSequenceExtra("dealerName");
		
		if (mDeviceName != null) {
			tv_device_name.setText(mDeviceName);
		}
		if (mDeviceID != null) {
			tv_device_mac.setText(mDeviceID);
		}
		String deviceBgPath = MainApplication.getInstance().thumbnailsPath+File.separator+mDeviceID+".jpg";
		File deviceBgFile = new File(deviceBgPath);
		if (deviceBgFile.exists()) {
			setDeviceBg(deviceBgPath);
		}
	}
	
	private void closeActivity() {
		if (isNeedRefresh) {
			setResult(1);
		}
		finish();
		overridePendingTransition(R.anim.fragment_nochange, R.anim.down_out);
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		Intent intent = null;
		switch (v.getId()) {
			case R.id.ib_device_manager_back:
				closeActivity();
				break;
			case R.id.rl_device_item:
				intent = new Intent(mContext, ModifyDeviceNameActivity.class);
				intent.putExtra("deviceName", mDeviceName);
				intent.putExtra("deviceID", mDeviceID);
				startActivityForResult(intent, 0);
				overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
				break;
			case R.id.rl_device_bg:
				intent = new Intent(mContext, SetDeviceBgActivity.class);
				startActivityForResult(intent, 1);
				overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_wifi:
				intent = new Intent(mContext, WiFiActivity.class);
				intent.putExtra("dealerName", mDealerName);
				startActivityForResult(intent, 1);
				overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_shared_device_manager:
				intent = new Intent(mContext, AddShareActivity.class);
				intent.putExtra("deviceName", mDeviceName);
				intent.putExtra("deviceID", mDeviceID);
				startActivity(intent);
				overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_delete_device:
				deleteDevice();
				break;
		}
	}
	
	@Override
	public boolean onLongClick(View arg0) {
		// TODO Auto-generated method stub
		switch (arg0.getId()) {
			case R.id.rl_device_bg:
				deleteDeviceBg();
				break;
		}
		return true;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			closeActivity();
		}
		return super.onKeyDown(keyCode, event);
	}
	
	/**
	 * 生成JSON的上传背景图片字符串
	 */
	private String generateUploadBgJson(String imgPath) {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_PushBackPic");
			jsonObj.put("UserName", userName);
			jsonObj.put("MAC", mDeviceID);
			jsonObj.put("Picture", Utils.imageToBase64(imgPath));
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的删除背景图片字符串
	 */
	private String generateDeleteBgJson() {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelBackPic");
			jsonObj.put("UserName", userName);
			jsonObj.put("MAC", mDeviceID);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的删除终端绑定字符串
	 */
	private String generateDeleteDeviceJson() {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelTerm");
			jsonObj.put("UserName", userName);
			jsonObj.put("MAC", mDeviceID);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 删除终端绑定的网络操作
	 */
	public void deleteDeviceEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			String data = generateDeleteDeviceJson();
			sendHandlerMsg(handler, IS_REQUESTING, "正在删除设备...");
			sendHandlerMsg(handler, REQUEST_TIMEOUT, "删除设备失败，网络超时！", Value.REQ_TIME_10S);
			sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	/**
	 * 上传背景图片
	 */
	private void uploadDeviceBg(Intent intent) {
		Bundle bundle = intent.getExtras();
		deviceBg = bundle.getString("ImageBgPath");
		String sendData = generateUploadBgJson(deviceBg);
		sendHandlerMsg(handler, IS_REQUESTING, "正在上传图片...");
		sendHandlerMsg(handler, REQUEST_TIMEOUT, "上传图片失败，请重试！", 20000);
		sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, sendData);
	}
	
	/**
	 * 设置背景图片
	 */
	private void setDeviceBg(String bgFilePath) {
		BitmapFactory.Options opts = new BitmapFactory.Options();
		opts.inJustDecodeBounds = true;
		BitmapFactory.decodeFile(bgFilePath, opts);
		opts.inJustDecodeBounds = false;
		opts.inSampleSize = Utils.computeSampleSize(opts, -1, 128*128);
		try {
			Bitmap bm = BitmapFactory.decodeFile(bgFilePath, opts);
			Drawable drawable =new BitmapDrawable(bm);
			rl_device_bg.setBackgroundDrawable(drawable);
		} catch (OutOfMemoryError e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * 删除背景图片
	 */
	private void deleteDeviceBg() {
		if (mDeviceBg.equals("null")) {
			Toast.makeText(mContext, "无背景图片，不需要删除！", Toast.LENGTH_SHORT).show();
		} else {
			final OkCancelDialog myDialog = new OkCancelDialog(mContext);
			myDialog.setTitle("温馨提示");
			myDialog.setMessage("确认删除背景图片？");
			myDialog.setPositiveButton("确认", new OnClickListener() {
				@Override
				public void onClick(View v) {
					myDialog.dismiss();
					String data = generateDeleteBgJson();
					sendHandlerMsg(handler, IS_REQUESTING, "正在删除图片...");
					sendHandlerMsg(handler, REQUEST_TIMEOUT, "删除图片失败，网络超时！", Value.REQ_TIME_10S);
					sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
				}
			});
			myDialog.setNegativeButton("取消", new OnClickListener() {
				@Override
				public void onClick(View v) {
					myDialog.dismiss();
				}
			});
		}
	}
	
	/**
	 * 删除设备
	 */
	private void deleteDevice() {
		final OkCancelDialog myDialog = new OkCancelDialog(mContext);
		myDialog.setTitle("温馨提示");
		myDialog.setMessage("确认删除终端绑定？");
		myDialog.setPositiveButton("确认", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
				deleteDeviceEvent();
			}
		});
		myDialog.setNegativeButton("取消", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
			}
		});
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUESTING:
					if ((mDialog == null) || (!mDialog.isShowing())) {
						mDialog = Utils.createLoadingDialog(mContext, (String) msg.obj);
						mDialog.show();
					}
					break;
				case REQUEST_TIMEOUT:
					if ((mDialog != null) && (mDialog.isShowing())) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					Toast.makeText(mContext, ""+msg.obj, Toast.LENGTH_SHORT).show();
					break;
				//上传背景图片
				case R.id.upload_back_image_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							isNeedRefresh = true;
							MainApplication.getInstance().xmlDevice.updateItemBg(mDeviceID, (String) msg.obj);
							String filePath = MainApplication.getInstance().thumbnailsPath+File.separator+mDeviceID+".jpg";
							Utils.deleteLocalFile(filePath);
							if (deviceBg != null) {
								setDeviceBg(deviceBg);
							}
							mDeviceBg = (String) msg.obj;
							Toast.makeText(mContext, "上传背景图片成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "上传背景图片失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.upload_back_image_id);
					}
					break;
				//删除背景图片
				case R.id.delete_back_image_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							isNeedRefresh = true;
							MainApplication.getInstance().xmlDevice.updateItemBg(mDeviceID, "null");
							String filePath = MainApplication.getInstance().thumbnailsPath+File.separator+mDeviceID+".jpg";
							Utils.deleteLocalFile(filePath);
							rl_device_bg.setBackgroundResource(R.drawable.device_item_bg);
							mDeviceBg = "null";
							Toast.makeText(mContext, "删除背景图片成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "删除背景图片失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_back_image_id);
					}
					break;
				//删除设备
				case R.id.delete_device_item_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							isNeedRefresh = true;
							MainApplication.getInstance().xmlDevice.deleteItem(mDeviceID);
							Toast.makeText(mContext, "删除设备成功！", Toast.LENGTH_SHORT).show();
							closeActivity();
						} else {
							Toast.makeText(mContext, "删除设备失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_device_item_id);
					}
					break;
			}
		}
	};
	
	/**
	 * 发送Handler消息
	 */
	public void sendHandlerMsg(Handler handler, int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, String obj, int timeout) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessageDelayed(msg, timeout);
	}
	public void sendHandlerMsg(Handler handler, int what, int arg1, int arg2, Object obj) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		super.onActivityResult(requestCode, resultCode, data);
		ZmqHandler.mHandler = handler;
		if (resultCode == 1) {
			//修改终端名称
			isNeedRefresh = true;
			Bundle bundle = data.getExtras();
			mDeviceName = bundle.getString("deviceName");
			tv_device_name.setText(mDeviceName);
		}
		else if (resultCode == 2) {
			//上传背景图片
			isNeedRefresh = true;
			uploadDeviceBg(data);
		}
	}
	
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		if ((mDialog != null) && (mDialog.isShowing())) {
			mDialog.dismiss();
			mDialog = null;
		}
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
}
