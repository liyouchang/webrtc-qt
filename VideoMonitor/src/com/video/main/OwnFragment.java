package com.video.main;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
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
import com.video.data.Value;
import com.video.play.PlayerActivity;
import com.video.service.BackstageService;
import com.video.service.MainApplication;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.user.LoginActivity;
import com.video.utils.DeviceItemAdapter;
import com.video.utils.OkCancelDialog;
import com.video.utils.OkOnlyDialog;
import com.video.utils.PopupWindowAdapter;
import com.video.utils.Utils;

public class OwnFragment extends Fragment implements OnClickListener {
	
	private static FragmentActivity mActivity;
	private View mView;
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
	
	private static DeviceItemAdapter deviceAdapter = null;
	private static ListView lv_list;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	private final int REFRESH_DEVICE_LIST = 3;
	
	private OwnReceiver ownReceiver;
	public static final String MSG_REFRESH_ACTION = "com.video.main.OwnFragment.msg_refresh_action";
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
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
		initData();
	}
	
	@Override
	public void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		if (MainActivity.isCurrentTab(1)) {
			ZmqHandler.mHandler = deviceHandler;
		}
	}
	
	private void initView () {
		ImageButton button_shared = (ImageButton) mView.findViewById(R.id.btn_device_share);
		button_shared.setOnClickListener(this);
		
		button_add = (ImageButton)mView.findViewById(R.id.btn_add_device);
		button_add.setOnClickListener(this);
		
		noDeviceLayout = (RelativeLayout) mView.findViewById(R.id.rl_no_device_list);
		
		lv_list = (ListView) mView.findViewById(R.id.device_list);
		lv_list.setOnItemClickListener(new OnItemClickListenerImpl());
		lv_list.setOnItemLongClickListener(new OnItemLongClickListenerImpl());
	}
	
	private void initData() {
		// 初始化Activity要使用的参数
		ZmqHandler.mHandler = deviceHandler;
		if (MainApplication.getInstance().preferData.isExist("UserName")) {
			userName = MainApplication.getInstance().preferData.readString("UserName");
		}
		
		// 注册广播
		ownReceiver = new OwnReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(MSG_REFRESH_ACTION);
		filter.addAction(BackstageService.CHANGE_DEVICE_LIST_ACTION);
		mActivity.registerReceiver(ownReceiver, filter);
		
		if (MainApplication.getInstance().deviceList == null) {
			MainApplication.getInstance().deviceList = new ArrayList<HashMap<String, String>>();
			listSize = 0;
		} else {
			listSize = MainApplication.getInstance().deviceList.size();
		}
		
		if (listSize > 1) {
			MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
		}
		deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
		lv_list.setAdapter(deviceAdapter);
		
		if (listSize == 0) {
			noDeviceLayout.setVisibility(View.VISIBLE);
		} else {
			noDeviceLayout.setVisibility(View.INVISIBLE);
		}
		
		if (Value.isNeedReqTermListFlag) {
			// 从服务器请求设备列表
			reqTermListEvent();
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
	
	/**
	 * 请求报警消息事件
	 */
	public void requestAlarmEvent() {
		Value.ownFragmentRequestAlarmFlag = true;
		String data = MainApplication.getInstance().generateRequestAlarmJson(0, 5);
		MainApplication.getInstance().sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, data);
	}
	
	/**
	 * 将终端列表的设备排序，在线在前，不在线在后
	 */
	private static ArrayList<HashMap<String, String>> orderDeviceList(ArrayList<HashMap<String, String>> list) {
		int len = list.size();
		ArrayList<HashMap<String, String>> listObj = new ArrayList<HashMap<String, String>>();
		
		//在线设备
		for (int i=0; i<len; i++) {
			HashMap<String, String> item = list.get(i);
			if (item.get("isOnline").equals("true")) {
				listObj.add(item);
			}
		}
		//不在线设备
		for (int i=0; i<len; i++) {
			HashMap<String, String> item = list.get(i);
			if (!item.get("isOnline").equals("true")) {
				listObj.add(item);
			}
		}
		return listObj;
	}
	
	private Handler deviceHandler = new Handler() {

		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUESTING:
					if (mDialog == null) {
						mDialog = Utils.createLoadingDialog(mActivity, (String) msg.obj);
						mDialog.show();
					}
					break;
				case REQUEST_TIMEOUT:
					if ((mDialog != null) && (mDialog.isShowing())) {
						mDialog.dismiss();
						mDialog = null;
					}
					if (deviceHandler.hasMessages(REQUEST_TIMEOUT)) {
						deviceHandler.removeMessages(REQUEST_TIMEOUT);
					}
					Value.isNeedReqTermListFlag = true;
					Toast.makeText(mActivity, ""+msg.obj, Toast.LENGTH_SHORT).show();
					if (listSize > 1) {
						MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
						deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
						lv_list.setAdapter(deviceAdapter);
					} else {
						deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
						lv_list.setAdapter(deviceAdapter);
					}
					break;
				//请求终端列表
				case R.id.request_terminal_list_id:
					if (deviceHandler.hasMessages(REQUEST_TIMEOUT)) {
						deviceHandler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							//请求终端列表成功
							Value.isNeedReqTermListFlag = false;
							ArrayList<HashMap<String, String>> listObj = (ArrayList<HashMap<String, String>>) msg.obj;
							if (listObj != null) {
								MainApplication.getInstance().xmlDevice.updateList(listObj);
								MainApplication.getInstance().deviceList = listObj;
								listSize = MainApplication.getInstance().getDeviceListSize();
								
								if (listSize > 1) {
									MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
								}
								deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
								lv_list.setAdapter(deviceAdapter);
								
							} else {
								listSize = 0;
								MainApplication.getInstance().xmlDevice.deleteAllItem();
								if (MainApplication.getInstance().deviceList != null) {
									MainApplication.getInstance().deviceList.clear();
									deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
									lv_list.setAdapter(deviceAdapter);
								}
							}
						} else {
							Toast.makeText(mActivity, msg.obj+"，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
						//请求报警数据
						requestAlarmEvent();
					} else {
						deviceHandler.removeMessages(R.id.request_terminal_list_id);
					}
					break;
				//删除终端绑定
				case R.id.delete_device_item_id:
					if (deviceHandler.hasMessages(REQUEST_TIMEOUT)) {
						deviceHandler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							MainApplication.getInstance().xmlDevice.deleteItem(mDeviceId);
							MainApplication.getInstance().deviceList.remove(listPosition);
							deviceAdapter.notifyDataSetChanged();
							Toast.makeText(mActivity, "删除终端绑定成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mActivity, "删除终端绑定失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						deviceHandler.removeMessages(R.id.delete_device_item_id);
					}
					break;
				//上传背景图片
				case R.id.upload_back_image_id:
					if (deviceHandler.hasMessages(REQUEST_TIMEOUT)) {
						deviceHandler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							//删除缩略图
							String filePath = MainApplication.getInstance().thumbnailsPath+File.separator+mDeviceId+".jpg";
							deleteImageFile(filePath);
							//设置背景图片
							MainApplication.getInstance().deviceList.get(listPosition).put("deviceBg", (String) msg.obj);
							deviceAdapter.notifyDataSetChanged();
							MainApplication.getInstance().xmlDevice.updateItemBg(mDeviceId, (String) msg.obj);
							Toast.makeText(mActivity, "上传背景图片成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mActivity, "上传背景图片失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						deviceHandler.removeMessages(R.id.upload_back_image_id);
					}
					break;
				//删除背景图片
				case R.id.delete_back_image_id:
					if (deviceHandler.hasMessages(REQUEST_TIMEOUT)) {
						deviceHandler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							//更新背景图片
							MainApplication.getInstance().deviceList.get(listPosition).put("deviceBg", "null");
							deviceAdapter.notifyDataSetChanged();
							MainApplication.getInstance().xmlDevice.updateItemBg(mDeviceId, "null");
							//删除缩略图
							String filePath = MainApplication.getInstance().thumbnailsPath+File.separator+mDeviceId+".jpg";
							deleteImageFile(filePath);
							Toast.makeText(mActivity, "删除背景图片成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mActivity, "删除背景图片失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						deviceHandler.removeMessages(R.id.delete_back_image_id);
					}
					break;
				// 刷新终端列表图片
				case REFRESH_DEVICE_LIST:
					if (deviceAdapter != null) {
						if (msg.obj != null) {
							listSize = MainApplication.getInstance().getDeviceListSize();
							if (listSize > 1) {
								MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
								deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
								lv_list.setAdapter(deviceAdapter);
							} else {
								deviceAdapter.notifyDataSetChanged();
							}
						} else {
							deviceAdapter.notifyDataSetChanged();
						}
					}
					break;
			}
			listSize = MainApplication.getInstance().getDeviceListSize();
			if (listSize == 0) {
				noDeviceLayout.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.INVISIBLE);
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
	 * 请求终端列表的网络操作
	 */
	public void reqTermListEvent() {
		if (Utils.isNetworkAvailable(mActivity)) {
			String data = generateReqTermListJson();
			MainApplication.getInstance().sendHandlerMsg(deviceHandler, IS_REQUESTING, "正在请求终端列表...");
			MainApplication.getInstance().sendHandlerMsg(deviceHandler, REQUEST_TIMEOUT, "请求终端列表失败，网络超时！", Value.REQ_TIME_10S);
			MainApplication.getInstance().sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mActivity, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	/**
	 * 删除终端绑定的网络操作
	 */
	public void delTermItemEvent(String id) {
		if (Utils.isNetworkAvailable(mActivity)) {
			String data = generateDelTermItemJson(id);
			MainApplication.getInstance().sendHandlerMsg(deviceHandler, IS_REQUESTING, "正在删除终端绑定...");
			MainApplication.getInstance().sendHandlerMsg(deviceHandler, REQUEST_TIMEOUT, "删除终端绑定失败，网络超时！", Value.REQ_TIME_10S);
			MainApplication.getInstance().sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
		} else {
			Toast.makeText(mActivity, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_device_share:
				startActivity(new Intent(mActivity, SharedActivity.class));
				mActivity.overridePendingTransition(R.anim.up_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_add_device:
				startActivityForResult(new Intent(mActivity, AddDeviceActivity.class), 0);
				mActivity.overridePendingTransition(R.anim.up_in, R.anim.fragment_nochange);
				break;
		}
	}
	
	/**
	 * 设备项ListView的点击事件
	 */
	private class OnItemClickListenerImpl implements OnItemClickListener {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, final int position, long id) {
			// TODO Auto-generated method stub
			listPosition = position;
			HashMap<String, String> item = MainApplication.getInstance().deviceList.get(listPosition);
			mDeviceName = item.get("deviceName");
			mDeviceId = item.get("deviceID");
			
			if (Utils.isNetworkAvailable(mActivity)) {
				
				if (Utils.getOnlineState(MainApplication.getInstance().deviceList.get(position).get("isOnline"))) {
					
					if (Value.isLoginSuccess) {
						//读取流量保护开关设置
						boolean isProtectTraffic = true;
						if (MainApplication.getInstance().preferData.isExist("ProtectTraffic")) {
							isProtectTraffic = MainApplication.getInstance().preferData.readBoolean("ProtectTraffic");
						}
						
						if (MainApplication.getInstance().deviceList.get(position).get("LinkState").equals("linked")) {
							if (!isProtectTraffic) {
								//实时视频
								Intent intent = new Intent(mActivity, PlayerActivity.class);
								intent.putExtra("deviceName", mDeviceName);
								intent.putExtra("dealerName", MainApplication.getInstance().deviceList.get(position).get("dealerName"));
								mActivity.startActivity(intent);
							} else {
								if (Utils.isWiFiNetwork(mActivity)) {
									//实时视频
									Intent intent = new Intent(mActivity, PlayerActivity.class);
									intent.putExtra("deviceName", mDeviceName);
									intent.putExtra("dealerName", MainApplication.getInstance().deviceList.get(position).get("dealerName"));
									mActivity.startActivity(intent);
								} else {
									final OkCancelDialog myDialog=new OkCancelDialog(mActivity);
									myDialog.setTitle("温馨提示");
									myDialog.setMessage("当前网络不是WiFi，继续观看视频？");
									myDialog.setPositiveButton("确认", new OnClickListener() {
										@Override
										public void onClick(View v) {
											myDialog.dismiss();
											//实时视频
											Intent intent = new Intent(mActivity, PlayerActivity.class);
											intent.putExtra("deviceName", mDeviceName);
											intent.putExtra("dealerName", MainApplication.getInstance().deviceList.get(position).get("dealerName"));
											mActivity.startActivity(intent);
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
						} else {
							Toast.makeText(mActivity, "未联机，无法请求视频！", Toast.LENGTH_SHORT).show();
						}
					} else {
						final OkOnlyDialog myDialog=new OkOnlyDialog(mActivity);
						myDialog.setTitle("温馨提示");
						myDialog.setMessage("网络不稳定，请重新登录！");
						myDialog.setPositiveButton("确认", new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog.dismiss();
								if (Value.beatHeartFailFlag) {
									Value.beatHeartFailFlag = false;
								}
								//登录界面
								mActivity.startActivity(new Intent(mActivity, LoginActivity.class));
							}
						});
					}
				} else {
					Toast.makeText(mActivity, "未联机，无法请求视频！", Toast.LENGTH_SHORT).show();
				}
			} else {
				Toast.makeText(mActivity, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
			}
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
		item_list.add("分享终端设备");
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
				HashMap<String, String> item = MainApplication.getInstance().deviceList.get(listPosition);
				mDeviceName = item.get("deviceName");
				mDeviceId = item.get("deviceID");
				Intent intent = null;
				switch (position) {
					case 0:
						intent = new Intent(mActivity, ModifyDeviceNameActivity.class);
						intent.putExtra("deviceName", mDeviceName);
						intent.putExtra("deviceID", mDeviceId);
						startActivityForResult(intent, 0);
						mActivity.overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
						break;
					case 1:
						intent = new Intent(mActivity, AddShareActivity.class);
						intent.putExtra("deviceName", mDeviceName);
						intent.putExtra("deviceID", mDeviceId);
						startActivity(intent);
						mActivity.overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
						break;
					case 2:
						intent = new Intent(mActivity, SetDeviceBgActivity.class);
						intent.putExtra("deviceName", mDeviceName);
						intent.putExtra("deviceID", mDeviceId);
						startActivityForResult(intent, 1);
						mActivity.overridePendingTransition(R.anim.down_in, R.anim.fragment_nochange);
						break;
					case 3:
						if (item.get("deviceBg").equals("null")) {
							Toast.makeText(mActivity, "无背景图片，不需要删除！", Toast.LENGTH_SHORT).show();
						} else {
							final OkCancelDialog myDialog1=new OkCancelDialog(mActivity);
							myDialog1.setTitle("温馨提示");
							myDialog1.setMessage("确认删除背景图片？");
							myDialog1.setPositiveButton("确认", new OnClickListener() {
								@Override
								public void onClick(View v) {
									myDialog1.dismiss();
									String data = generateDeleteImageJson(mDeviceId);
									MainApplication.getInstance().sendHandlerMsg(deviceHandler, IS_REQUESTING, "正在删除图片...");
									MainApplication.getInstance().sendHandlerMsg(deviceHandler, REQUEST_TIMEOUT, "删除图片失败，网络超时！", Value.REQ_TIME_10S);
									MainApplication.getInstance().sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
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
					case 4:
						final OkCancelDialog myDialog2=new OkCancelDialog(mActivity);
						myDialog2.setTitle("温馨提示");
						myDialog2.setMessage("确认删除终端绑定？");
						myDialog2.setPositiveButton("确认", new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog2.dismiss();
								delTermItemEvent(mDeviceId);
							}
						});
						myDialog2.setNegativeButton("取消", new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog2.dismiss();
							}
						});
						break;
				}
				if (mPopupWindow.isShowing()) {
					mPopupWindow.dismiss();
				}
			}
		});
	}
	
	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		super.onActivityResult(requestCode, resultCode, data);
		if (resultCode == 1) {
			// 修改终端名称
			listSize = MainApplication.getInstance().xmlDevice.getListSize();
			Bundle bundle = data.getExtras();
			String id = bundle.getString("deviceId");
			String name = bundle.getString("deviceName");
			int position = MainApplication.getInstance().getDeviceListPositionByDeviceID(id);
			if (position == -1) {
				return ;
			} else {
				MainApplication.getInstance().deviceList.get(position).put("deviceName", name);
			}
		}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
		else if (resultCode == 2) {
			// 设置背景图片
			Bundle bundle = data.getExtras();
			String path = bundle.getString("ImageBgPath");
			String sendData = generateUploadImageJson(mDeviceId, path);
			MainApplication.getInstance().sendHandlerMsg(deviceHandler, IS_REQUESTING, "正在上传图片...");
			MainApplication.getInstance().sendHandlerMsg(deviceHandler, REQUEST_TIMEOUT, "上传图片失败，请重试！", 15000);
			MainApplication.getInstance().sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, sendData);
		}
		else if (resultCode == 3) {
			// 添加终端设备
			listSize = MainApplication.getInstance().xmlDevice.getListSize();
			
			if (listSize > 1) {
				MainApplication.getInstance().deviceList = orderDeviceList(MainApplication.getInstance().deviceList);
				deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
				lv_list.setAdapter(deviceAdapter);
			} else {
				deviceAdapter = new DeviceItemAdapter(mActivity, MainApplication.getInstance().deviceList);
				lv_list.setAdapter(deviceAdapter);
			}
			
			if (listSize == 0) {
				noDeviceLayout.setVisibility(View.VISIBLE);
			} else {
				noDeviceLayout.setVisibility(View.INVISIBLE);
			}
		}
		MainApplication.getInstance().sendHandlerMsg(deviceHandler, REFRESH_DEVICE_LIST);
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		// 注销广播
		mActivity.unregisterReceiver(ownReceiver);
	}

	public class OwnReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			String action = intent.getAction();
			//更新未读报警消息的显示
			if (action.equals(MSG_REFRESH_ACTION)) {
				MainActivity.setAlarmIconAndText(intent.getIntExtra("AlarmCount", 0));
			}
			else if (action.equals(BackstageService.CHANGE_DEVICE_LIST_ACTION)) {
				if (!intent.hasExtra("isTermActive")) {
					boolean isTermActive = intent.getBooleanExtra("isTermActive", false);
					MainApplication.getInstance().sendHandlerMsg(deviceHandler, REFRESH_DEVICE_LIST, 0, 0, isTermActive);
				} else {
					MainApplication.getInstance().sendHandlerMsg(deviceHandler, REFRESH_DEVICE_LIST, 0, 0, null);
				}
			}
		}
	}
}
