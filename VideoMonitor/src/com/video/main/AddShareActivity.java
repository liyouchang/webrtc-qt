package com.video.main;

import java.util.ArrayList;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.LayoutParams;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqHandler;
import com.video.utils.PopupWindowAdapter;
import com.video.utils.Utils;
import com.video.utils.ViewPagerAdapter;

public class AddShareActivity extends Activity implements OnClickListener, OnPageChangeListener {

	private Context mContext;
	private PreferData preferData = null;
	
	private EditText et_name;
	private Button button_delete_name;
	private Dialog mDialog = null;
	//终端列表项
	private String mDeviceName = null;
	private String mDeviceId = null;
	private String userName = "";
	private String shareName = "";
	
	private final int IS_ADDING = 1;
	private final int ADD_TIMEOUT = 2;
	private final int REQ_TIMEOUT = 3;
	private final int IS_DELETING = 4;
	private final int DELETE_TIMEOUT = 5;
	
	private TextView viewpage_device;
	private TextView viewpage_user;
	
	private ViewPager mViewPager;
	private List<View> pageList;
	private View device_page;
	private View user_page;
	
	private TextView userShareExplain = null;
	private ListView userListView = null;
	private ArrayList<String> userList = null;
	private int userListPosition = -1;
	private ShareUserAdapter userAdapter = null;
	private PopupWindow mPopupWindow;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.add_share_page);
		
		initViewPageView();
		initView();
		initData();
	}
	
	/**
	 * 初始化添加终端的Page界面
	 */
	private void initViewPageView() {
		mContext = AddShareActivity.this;
		mViewPager = (ViewPager) this.findViewById(R.id.share_viewpager);
		mViewPager.setOnPageChangeListener(this);
		
		LayoutInflater inflater = LayoutInflater.from(mContext);
		device_page = inflater.inflate(R.layout.add_share_device, null);
		user_page = inflater.inflate(R.layout.add_share_user, null);
		pageList = new ArrayList<View>();
		pageList.add(device_page);
		pageList.add(user_page);
		mViewPager.setAdapter(new ViewPagerAdapter(pageList));
	}

	private void initView() {
		viewpage_device = (TextView) this.findViewById(R.id.tv_vp_share_device);
		viewpage_user = (TextView) this.findViewById(R.id.tv_vp_share_user);
		viewpage_device.setOnClickListener(this);
		viewpage_user.setOnClickListener(this);
		
		//分享设备
		Button button_ok = (Button) device_page.findViewById(R.id.btn_add_share_ok);
		button_ok.setOnClickListener(this);
		
		button_delete_name = (Button) device_page.findViewById(R.id.btn_add_share_name_del);
		button_delete_name.setOnClickListener(this);
		
		et_name = (EditText) device_page.findViewById(R.id.et_add_share_name);
		et_name.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				if (s.length() == 0) {
					button_delete_name.setVisibility(View.INVISIBLE);
				} else {
					button_delete_name.setVisibility(View.VISIBLE);
				}
			}
			
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}
			
			@Override
			public void afterTextChanged(Editable s) {
			}
		});
		
		//分享用户
		userShareExplain = (TextView) user_page.findViewById(R.id.tv_share_user_explain);
		userListView = (ListView) user_page.findViewById(R.id.shared_list);
		userListView.setOnItemClickListener(new OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				userListPosition = position;
				showPopupWindow(userListView);
			}
		});
	}
	
	private void initData() {
		preferData = new PreferData(mContext);
		ZmqHandler.mHandler = handler;
		Bundle bundle = this.getIntent().getExtras();
		mDeviceName = bundle.getString("deviceName");
		mDeviceId = bundle.getString("deviceID");
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		userShareExplain.setText("【"+mDeviceName+"】终端已分享给以下用户：");
		
		reqMACShareListEvent();
	}
	
	/**
	 * 生成JSON的添加终端分享字符串
	 */
	private String generateAddShareTermJson(String mac) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_AddShareTerm");
			jsonObj.put("UserName", shareName);
			jsonObj.put("MAC", mac);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 生成JSON的指定终端分享的用户字符串
	 */
	private String generateReqMacShareListJson(String mac) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ReqMAClist");
			jsonObj.put("UserName", userName);
			jsonObj.put("MAC", mac);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 生成JSON的删除终端分享字符串
	 */
	private String generateDelShareTermItemJson(String user, String mac) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelShareTerm");
			jsonObj.put("UserName", user);
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
				case IS_ADDING:
					mDialog = Utils.createLoadingDialog(mContext, "正在分享...");
					mDialog.show();
					break;
				case ADD_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					Toast.makeText(mContext, "添加终端分享失败，网络超时！", Toast.LENGTH_SHORT).show();
					if (handler.hasMessages(ADD_TIMEOUT)) {
						handler.removeMessages(ADD_TIMEOUT);
					}
					break;
				case REQ_TIMEOUT:
					if (handler.hasMessages(R.id.requst_device_share_user_id)) {
						handler.removeMessages(R.id.requst_device_share_user_id);
					}
					userShareExplain.setText("请求指定终端分享用户列表超时，请重试！");
					break;
				case IS_DELETING:
					mDialog = Utils.createLoadingDialog(mContext, "正在取消终端分享...");
					mDialog.show();
					break;
				case DELETE_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					Toast.makeText(mContext, "取消终端分享失败，网络超时！", Toast.LENGTH_SHORT).show();
					break;
				//添加终端分享
				case R.id.add_device_share_id:
					if (handler.hasMessages(ADD_TIMEOUT)) {
						handler.removeMessages(ADD_TIMEOUT);
						if (mDialog != null)
							mDialog.dismiss();
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							Toast.makeText(mContext, "添加终端分享成功！", Toast.LENGTH_SHORT).show();
						} else {
							Toast.makeText(mContext, "添加终端分享失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.add_device_share_id);
					}
					break;
				//请求终端分享用户列表
				case R.id.requst_device_share_user_id:
					if (handler.hasMessages(REQ_TIMEOUT)) {
						handler.removeMessages(REQ_TIMEOUT);
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							userList = (ArrayList<String>) msg.obj;
							if (userList.size() == 0) {
								userShareExplain.setText("【"+mDeviceName+"】还未分享给任何人！");
							} else {
								userShareExplain.setText("【"+mDeviceName+"】终端已分享给以下用户：");
								userAdapter = new ShareUserAdapter(mContext, userList);
								userListView.setAdapter(userAdapter);
							}
						} else {
							userShareExplain.setText("请求指定终端分享用户列表失败，"+Utils.getErrorReason(resultCode));
						}
					} else {
						handler.removeMessages(R.id.requst_device_share_user_id);
					}
					break;
				//取消终端分享
				case R.id.delete_device_share_id:
					if (handler.hasMessages(DELETE_TIMEOUT)) {
						handler.removeMessages(DELETE_TIMEOUT);
						if (mDialog != null) {
							mDialog.dismiss();
							mDialog = null;
						}
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							userList.remove(userListPosition);
							userAdapter.notifyDataSetChanged();
							Toast.makeText(mContext, "取消终端分享成功！", Toast.LENGTH_SHORT).show();
							if (userList.size() == 0) {
								userShareExplain.setText("【"+mDeviceName+"】还未分享给任何人！");
							}
						} else {
							Toast.makeText(mContext, "取消终端分享失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.delete_device_share_id);
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
	
	public void clickAddDeviceEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkAddShareData()) {
				Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
				String data = generateAddShareTermJson(mDeviceId);
				sendHandlerMsg(IS_ADDING);
				sendHandlerMsg(ADD_TIMEOUT, Value.REQ_TIME_10S);
				sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
			}
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	public void reqMACShareListEvent() {
		Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
		String data = generateReqMacShareListJson(mDeviceId);
		sendHandlerMsg(REQ_TIMEOUT, Value.REQ_TIME_10S);
		sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
	}
	
	/**
	 * 改变ViewPage页和标题显示
	 */
	private void changeViewPage(int index) {
		viewpage_device.setBackgroundResource(R.drawable.viewpage_unselected);
		viewpage_user.setBackgroundResource(R.drawable.viewpage_unselected);
		
		viewpage_device.setTextColor(getResources().getColorStateList(R.color.white));
		viewpage_user.setTextColor(getResources().getColorStateList(R.color.white));
		
		switch (index) {
			case 0:
				viewpage_device.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_device.setTextColor(getResources().getColorStateList(R.color.orange));
				mViewPager.setCurrentItem(0);
				break;
			case 1:
				viewpage_user.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_user.setTextColor(getResources().getColorStateList(R.color.orange));
				mViewPager.setCurrentItem(1);
				break;
		}
	}
	
	@Override
	public void onPageScrollStateChanged(int arg0) {
		// TODO Auto-generated method stub
	}
	@Override
	public void onPageScrolled(int arg0, float arg1, int arg2) {
		// TODO Auto-generated method stub
	}
	@Override
	public void onPageSelected(int arg0) {
		// TODO Auto-generated method stub
		changeViewPage(arg0);
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_add_share_ok:
				clickAddDeviceEvent();
				break;
			case R.id.btn_add_share_name_del:
				et_name.setText("");
				break;
			case R.id.tv_vp_share_device:
				changeViewPage(0);
				break;
			case R.id.tv_vp_share_user:
				changeViewPage(1);
				break;
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.down_out);
		}
		return super.onKeyDown(keyCode, event);
	}
	
	/**
	 * @return true:注册信息格式正确  false:注册信息格式错误
	 */
	private boolean checkAddShareData() {
		boolean resultFlag = false;
		
		//获取EditText输入框的字符串
		shareName = et_name.getText().toString().trim();
		
		
		if (shareName.equals("")) {
			resultFlag = false;
			Toast.makeText(mContext, "请输入用户名！", Toast.LENGTH_LONG).show();
		}
		else if (Utils.isChineseString(shareName)) {
			resultFlag = false;
			Toast.makeText(mContext, "不支持中文！", Toast.LENGTH_LONG).show();
		}
		else if ((shareName.length()<3) || (shareName.length()>20)) {
			resultFlag = false;
			Toast.makeText(mContext, "用户名长度范围3~20！", Toast.LENGTH_LONG).show();
		} else if (userName.equals(shareName)) {
			resultFlag = false;
			Toast.makeText(mContext, "不能将设备分享给自己！", Toast.LENGTH_LONG).show();
		} else {
			resultFlag = true;
		}
		return resultFlag;
	}
	
	public class ShareUserAdapter extends BaseAdapter {

		private Context context;
		private ArrayList<String> list = null;

		public ShareUserAdapter(Context context, ArrayList<String> list) {
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
				convertView = LayoutInflater.from(context).inflate(R.layout.share_user_item, null);
				holder = new ViewHolder();
				convertView.setTag(holder);
				holder.tv_users = (TextView) convertView.findViewById(R.id.tv_share_user_item);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			if (holder.tv_users != null) {
				holder.tv_users.setText(list.get(position));
			}
			return convertView;
		}

		class ViewHolder {
			TextView tv_users;
		}
	}
	
	/**
	 * 设备项ListView的长按键的PopupWindow选项
	 */
	public void showPopupWindow(View view) {
		LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View pop_view = inflater.inflate(R.layout.pop_event_main, null);
		ListView pop_listView = (ListView)pop_view.findViewById(R.id.pop_list);
		
		List<String> item_list = new ArrayList<String>();
		item_list.add("取消分享该用户");
		PopupWindowAdapter popAdapter = new PopupWindowAdapter(mContext, item_list);
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
				String userListName = userList.get(position);
				switch (position) {
					case 0:
						Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
						String data = generateDelShareTermItemJson(userListName, mDeviceId);
						sendHandlerMsg(IS_DELETING);
						sendHandlerMsg(DELETE_TIMEOUT, Value.REQ_TIME_10S);
						sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
						break;
				}
				if (mPopupWindow.isShowing()) {
					mPopupWindow.dismiss();
				}
			}
		});
	}

}
