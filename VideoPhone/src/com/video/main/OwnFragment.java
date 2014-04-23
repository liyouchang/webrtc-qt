package com.video.main;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager.LayoutParams;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.data.XmlDevice;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.DeviceItemAdapter;
import com.video.utils.PopupWindowAdapter;
import com.video.utils.Utils;

public class OwnFragment extends Fragment implements OnClickListener {
	
	private FragmentActivity mActivity;
	private View mView;
	private static XmlDevice xmlData;
	private PreferData preferData = null;
	private String userName = null;
	//终端列表项
	private static String mDeviceName = null;
	private static String mDeviceId = null;
	private static int listPosition = 0;
	private static int listSize = 0;
	private RelativeLayout noDeviceLayout = null;
	
	private ImageButton button_add;
	private PopupWindow mPopupWindow;
	private Dialog mDialog = null;
	
	private String thumbnailsPath = null;
	private File thumbnailsFile = null;
	private static ArrayList<HashMap<String, String>> deviceList = null;
	private static DeviceItemAdapter deviceAdapter = null;
	private ListView lv_list;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		return inflater.inflate(R.layout.own, container, false);
	}
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		mActivity = getActivity();
		mView = getView();
		
		initView();
	}
	
	@Override
	public void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		initData();
		if (Value.SetDeviceBgActivityImagePath != null) {
			Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
			String data = generateUploadImageJson(mDeviceId, Value.SetDeviceBgActivityImagePath);
			sendHandlerMsg(IS_REQUESTING, "正在上传图片...");
			sendHandlerMsg(REQUEST_TIMEOUT, "上传图片失败，请重试！", Value.requestTimeout);
			sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
		}
	}
	
	private void initView () {
		button_add = (ImageButton)mView.findViewById(R.id.btn_add_device);
		button_add.setOnClickListener(this);
		
		noDeviceLayout = (RelativeLayout) mView.findViewById(R.id.rl_no_device_list);
		
		lv_list = (ListView) mView.findViewById(R.id.device_list);
		lv_list.setOnItemLongClickListener(new OnItemLongClickListenerImpl());
	}
	
	private void initData() {
		//初始化Activity要使用的参数
		ZmqHandler.setHandler(handler);
		xmlData = new XmlDevice(mActivity);
		preferData = new PreferData(mActivity);
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		
		String SD_path = Environment.getExternalStorageDirectory().getAbsolutePath();
		thumbnailsPath = SD_path + File.separator + "KaerVideo" + File.separator + "thumbnails";
		thumbnailsFile = new File(thumbnailsPath);
		if(!thumbnailsFile.exists()){
			thumbnailsFile.mkdirs();
		}
		
		//初始化终端列表的显示
		if (Value.isNeedReqTermListFlag) {
			reqTermListEvent();
		} else {
			deviceList = xmlData.readXml();
			if (deviceList != null) {
				listSize = deviceList.size();
				deviceAdapter = new DeviceItemAdapter(mActivity, thumbnailsFile, deviceList);
				lv_list.setAdapter(deviceAdapter);
			} 
		}
		if (listSize == 0) {
			noDeviceLayout.setVisibility(View.VISIBLE);
		} else {
			noDeviceLayout.setVisibility(View.INVISIBLE);
		}
	}
	
	/**
	 * 生成JSON的请求设备列表字符串
	 */
	private String generateReqTermListJson() {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ReqTermList");
			jsonObj.put("UserName", userName);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 生成JSON的删除终端绑定字符串
	 */
	private String generateDelTermItemJson(String mac) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelTerm");
			jsonObj.put("UserName", userName);
			jsonObj.put("MAC", mac);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 生成JSON的上传背景图片字符串
	 */
	private String generateUploadImageJson(String mac, String imgPath) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_PushBackPic");
			jsonObj.put("UserName", userName);
			jsonObj.put("MAC", mac);
			jsonObj.put("Picture", Utils.imageToBase64(imgPath));
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
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

		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUESTING:
					mDialog = Utils.createLoadingDialog(mActivity, (String) msg.obj);
					mDialog.show();
					break;
				case REQUEST_TIMEOUT:
					if (mDialog != null) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
					}
					Value.isNeedReqTermListFlag = true;
					Value.SetDeviceBgActivityImagePath = null;
					Toast.makeText(mActivity, ""+msg.obj, Toast.LENGTH_SHORT).show();
					break;
				//请求终端列表
				case R.id.request_terminal_list_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog != null) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							deviceList = (ArrayList<HashMap<String, String>>) msg.obj;
							if (deviceList != null) {
								Value.isNeedReqTermListFlag = false;
								xmlData.updateList(deviceList);
								deviceAdapter = new DeviceItemAdapter(mActivity, thumbnailsFile, deviceList);
								lv_list.setAdapter(deviceAdapter);
								listSize = deviceList.size();
							}
						} else {
							Toast.makeText(mActivity, msg.obj+"，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.request_terminal_list_id);
					}
					break;
				//删除终端绑定
				case R.id.delete_device_item_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog != null) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							xmlData.deleteItem(mDeviceId);
							deviceList.remove(listPosition);
							deviceAdapter.notifyDataSetChanged();
							listSize = xmlData.getListSize();
							Toast.makeText(mActivity, "删除终端绑定成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mActivity, "删除终端绑定失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_device_item_id);
					}
					break;
				//上传背景图片
				case R.id.upload_back_image_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog != null) {
							mDialog.dismiss();
							mDialog = null;
						}
						Value.SetDeviceBgActivityImagePath = null;
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							//删除缩略图
							String filePath = thumbnailsPath+File.separator+mDeviceId+".jpg";
							deleteImageFile(filePath);
							//设置背景图片
							deviceList.get(listPosition).put("deviceBg", (String) msg.obj);
							deviceAdapter.notifyDataSetChanged();
							xmlData.updateItemBg(mDeviceId, (String) msg.obj);
							Toast.makeText(mActivity, "上传背景图片成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mActivity, "上传背景图片失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.upload_back_image_id);
					}
					break;
				//删除背景图片
				case R.id.delete_back_image_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if (mDialog != null) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							//更新背景图片
							deviceList.get(listPosition).put("deviceBg", "null");
							deviceAdapter.notifyDataSetChanged();
							xmlData.updateItemBg(mDeviceId, "null");
							//删除缩略图
							String filePath = thumbnailsPath+File.separator+mDeviceId+".jpg";
							deleteImageFile(filePath);
							Toast.makeText(mActivity, "删除背景图片成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mActivity, "删除背景图片失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_back_image_id);
					}
					break;
			}
			if (listSize == 0) {
				noDeviceLayout.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.INVISIBLE);
			}
		}
	};
	
	public static Handler ownHandler = new Handler() {
		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			if (msg.what == 0) {
				HashMap<String, String> item = (HashMap<String, String>)msg.obj;
				String mac = item.get("deviceID");
				for (int i=0; i<listSize; i++) {
					if (deviceList.get(i).get("deviceID").equals(mac)) {
						item.put("deviceName", deviceList.get(i).get("deviceName"));
						deviceList.get(i).put("isOnline", item.get("isOnline"));
						deviceList.get(i).put("dealerName", item.get("dealerName"));
						break;
					}
				}
				deviceAdapter.notifyDataSetChanged();
				xmlData.updateItemState(mac, item.get("isOnline"), item.get("dealerName"));
			}
		}
	};
	
	/**
	 * 删除指定的图片文件
	 * @param filePath 图片文件路径
	 */
	private void deleteImageFile(String filePath) {
		File file = new File(filePath);
        if (file.exists()) {
            file.delete();
        }
	}
	
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
		handler.sendMessage(msg);
	}
	
	/**
	 * 请求终端列表的网络操作
	 */
	public void reqTermListEvent() {
		if (Utils.isNetworkAvailable(mActivity)) {
			Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
			String data = generateReqTermListJson();
			sendHandlerMsg(IS_REQUESTING, "正在请求终端列表...");
			sendHandlerMsg(REQUEST_TIMEOUT, "请求终端课表失败，网络超时！", Value.requestTimeout);
			sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mActivity, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	/**
	 * 删除终端绑定的网络操作
	 */
	public void delTermItemEvent(String id) {
		if (Utils.isNetworkAvailable(mActivity)) {
			Handler sendHandler = ZmqThread.zmqThreadHandler;
			String data = generateDelTermItemJson(id);
			sendHandlerMsg(IS_REQUESTING, "正在删除终端绑定...");
			sendHandlerMsg(REQUEST_TIMEOUT, "删除终端绑定失败，网络超时！", Value.requestTimeout);
			sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mActivity, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_add_device:
				startActivity(new Intent(mActivity, AddDeviceActivity.class));
				break;
		}
	}
	
	/**
	 * 设备项ListView的长点击事件
	 */
	private class OnItemLongClickListenerImpl implements OnItemLongClickListener {
		@Override
		public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
			// TODO Auto-generated method stub
			listPosition = position;
			showPopupWindow(lv_list);
			return false;
		}
	}

	/**
	 * 设备项ListView的长按键的PopupWindow选项
	 */
	public void showPopupWindow(View view) {
		LayoutInflater inflater = (LayoutInflater) mActivity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View pop_view = inflater.inflate(R.layout.pop_event_main, null);
		ListView pop_listView = (ListView)pop_view.findViewById(R.id.pop_list);
		
		List<String> item_list = new ArrayList<String>();
		item_list.add("修改终端名称");
		item_list.add("设置背景图片");
		item_list.add("删除背景图片");
		item_list.add("删除终端绑定");
		PopupWindowAdapter popAdapter = new PopupWindowAdapter(mActivity, item_list);
		pop_listView.setAdapter(popAdapter);
		
		mPopupWindow = new PopupWindow(pop_view, Utils.screenWidth, 200, true);
		mPopupWindow.setHeight(LayoutParams.WRAP_CONTENT); 
		mPopupWindow.setBackgroundDrawable(new BitmapDrawable());
		mPopupWindow.setOutsideTouchable(true);
		
		mPopupWindow.setAnimationStyle(R.style.PopupAnimationBottom);
		mPopupWindow.showAtLocation(view, Gravity.BOTTOM, 0, 0);
		mPopupWindow.update();

		pop_listView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				HashMap<String, String> item = deviceList.get(listPosition);
				mDeviceName = item.get("deviceName");
				mDeviceId = item.get("deviceID");
				Intent intent = null;
				switch (position) {
					case 0:
						intent = new Intent(mActivity, ModifyDeviceNameActivity.class);
						intent.putExtra("deviceName", mDeviceName);
						intent.putExtra("deviceID", mDeviceId);
						startActivity(intent);
						mActivity.overridePendingTransition(R.anim.down_in, 0);
						break;
					case 1:
						intent = new Intent(mActivity, SetDeviceBgActivity.class);
						intent.putExtra("deviceName", mDeviceName);
						intent.putExtra("deviceID", mDeviceId);
						startActivityForResult(intent, 1);
						mActivity.overridePendingTransition(R.anim.down_in, 0);
						break;
					case 2:
						Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
						String data = generateDeleteImageJson(mDeviceId);
						sendHandlerMsg(IS_REQUESTING, "正在删除图片...");
						sendHandlerMsg(REQUEST_TIMEOUT, "删除图片失败，网络超时！", Value.requestTimeout);
						sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
						break;
					case 3:
						delTermItemEvent(mDeviceId);
						break;
				}
				if (mPopupWindow.isShowing()) {
					mPopupWindow.dismiss();
				}
			}
		});
	}
}
