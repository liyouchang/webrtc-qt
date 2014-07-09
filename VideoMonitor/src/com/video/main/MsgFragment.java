package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.BitmapDrawable;
import android.os.Build;
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
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.data.XmlMessage;
import com.video.main.PullToRefreshView.OnFooterRefreshListener;
import com.video.main.PullToRefreshView.OnHeaderRefreshListener;
import com.video.service.BackstageService;
import com.video.service.MainApplication;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.user.LoginActivity;
import com.video.utils.MessageItemAdapter;
import com.video.utils.OkOnlyDialog;
import com.video.utils.PopupWindowAdapter;
import com.video.utils.Utils;

public class MsgFragment extends Fragment implements OnClickListener, OnHeaderRefreshListener, OnFooterRefreshListener {

	private FragmentActivity mActivity;
	private View mView;
	private XmlMessage xmlData = null;
	private PreferData preferData = null;
	private int unreadAlarmCount = 0;
	private Dialog mDialog = null;
	
	private BackstageMessageReceiver backstageMessageReceiver = null;
	public final static String READ_MESSAGE_ACTION = "MsgFragment.read_message_action";
	
	/**
	 * 0:正常请求和下拉请求  1:上拖请求  2:请求新的报警数据  3:删除该条报警  4:删除当前全部报警  5:标记该条报警  6:标记当前全部报警
	 */
	private int reqAlarmType = 0;
	private String userName = "";
	
	//终端列表项
	private String mMsgID = null;
	private int listSize = 0;
	private int listPosition = 0;
	private PopupWindow mPopupWindow;
//	private RelativeLayout noMsgLayout = null;
	
	private static ArrayList<HashMap<String, String>> msgList = null;
	private MessageItemAdapter msgAdapter = null;
	private ListView lv_list;
	private PullToRefreshView mPullToRefreshView;
	private String msg_refresh_time = null;
	private String msg_refresh_terminal = null;
	
	private final int IS_REQUESTING = 1;
	private final int REQUEST_TIMEOUT = 2;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		return inflater.inflate(R.layout.msg, container, false);
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
		if (Value.isLoginSuccess) {
			if (MainActivity.isCurrentTab(3)) {
				ZmqHandler.mHandler = handler;
				//需要请求报警消息
				if (Value.isNeedReqAlarmListFlag) {
					Value.ownFragmentRequestAlarmFlag = false;
					reqAlarmType = 2;
					requestAlarmEvent(0, Value.newAlarmMessageCount);
				} else {
					if (Value.ownFragmentRequestAlarmFlag) {
						Value.ownFragmentRequestAlarmFlag = false;
						msgList = xmlData.readXml();
						if (msgList != null) {
							listSize = msgList.size();
							msgAdapter = new MessageItemAdapter(mActivity, msgList);
							lv_list.setAdapter(msgAdapter);
						}
					}
				}
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
	}
	
	private void initView() {
		lv_list = (ListView) mView.findViewById(R.id.msg_list);
		lv_list.setOnItemClickListener(new OnItemClickListenerImpl());
		lv_list.setOnItemLongClickListener(new OnItemLongClickListenerImpl());
		
//		noMsgLayout = (RelativeLayout) mView.findViewById(R.id.rl_no_alert_msg_image);
		
		mPullToRefreshView = (PullToRefreshView) mView.findViewById(R.id.main_pull_refresh_view);
		mPullToRefreshView.setOnHeaderRefreshListener(this);
        mPullToRefreshView.setOnFooterRefreshListener(this);
	}
	
	private void initData() {
		ZmqHandler.mHandler = handler;
		xmlData = new XmlMessage(mActivity);
		preferData = new PreferData(mActivity);
		
		//注册广播
		backstageMessageReceiver = new BackstageMessageReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(READ_MESSAGE_ACTION);
		filter.addAction(BackstageService.BACKSTAGE_MESSAGE_ACTION);
		mActivity.registerReceiver(backstageMessageReceiver, filter);
		
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
		
		//初始化下拉刷新的显示
		if (preferData.isExist("msgRefreshTime")) {
			msg_refresh_time = preferData.readString("msgRefreshTime");
		}
		if (preferData.isExist("msgRefreshTerminal")) {
			msg_refresh_terminal = preferData.readString("msgRefreshTerminal");
		}
		if ((msg_refresh_time != null) && (msg_refresh_terminal != null)) {
			mPullToRefreshView.onHeaderRefreshComplete(msg_refresh_time, msg_refresh_terminal);
		}
		
		msgList = new ArrayList<HashMap<String, String>>();
		msgList = xmlData.readXml();
		if (msgList != null) {
			listSize = msgList.size();
			msgAdapter = new MessageItemAdapter(mActivity, msgList);
			lv_list.setAdapter(msgAdapter);
		}
	}
	
	/**
	 * 生成JSON的删除该条报警字符串
	 */
	private String generateDeleteThisItemJson(int id) {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelAlarm");
			jsonObj.put("UserName", userName);
			jsonObj.put("ID", id);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的删除当前全部报警字符串
	 */
	private String generateDeleteThisListJson() {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_DelSelectAlarm");
			jsonObj.put("UserName", userName);
			JSONArray jsonArray = new JSONArray();
			for (int i=0; i<listSize; i++) {
				String id = msgList.get(i).get("msgID");
				jsonArray.put(Integer.parseInt(id.trim()));
			}
			jsonObj.put("SID", jsonArray);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的标记该条报警字符串
	 */
	private String generateMarkThisItemJson(int id) {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_MarkAlarm");
			jsonObj.put("UserName", userName);
			jsonObj.put("ID", id);
			if (xmlData.getItemState(id)) 
				return null;
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的标记当前全部报警字符串
	 */
	private String generateMarkThisListJson() {
		boolean result = false;
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_MarkSelectAlarm");
			jsonObj.put("UserName", userName);
			JSONArray jsonArray = new JSONArray();
			for (int i=0; i<listSize; i++) {
				if (msgList.get(i).get("isReaded").equals("false")) {
					result = true;
					String id = msgList.get(i).get("msgID");
					jsonArray.put(Integer.parseInt(id.trim()));
				}
			}
			jsonObj.put("SID", jsonArray);
			if (result == false)
				return null;
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	private int getAlarmCount() {
		int result = 0;
		for (int i=0; i<listSize; i++) {
			if (msgList.get(i).get("isReaded").equals("false")) {
				result++;
			}
		}
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
					if (mDialog == null) {
						mDialog = Utils.createLoadingDialog(mActivity, "正在请求报警数据...");
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
					if (mPullToRefreshView.getHeaderState() == PullToRefreshView.REFRESHING) {
						mPullToRefreshView.onHeaderRefreshComplete();
					}
					if (mPullToRefreshView.getFooterState() == PullToRefreshView.REFRESHING) {
						mPullToRefreshView.onFooterRefreshComplete();
					}
					Toast.makeText(mActivity, "请求报警数据超时，请重试！", Toast.LENGTH_SHORT).show();
					break;
				case R.id.request_alarm_id:
					if (handler.hasMessages(REQUEST_TIMEOUT)) {
						handler.removeMessages(REQUEST_TIMEOUT);
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (msg.arg1 == 0) {
							Value.isNeedReqAlarmListFlag = false;
							switch (reqAlarmType) {
								//正常请求和下拉请求
								case 0:
									if (mPullToRefreshView.getHeaderState() == PullToRefreshView.REFRESHING) {
										msg_refresh_time = "上次更新于: "+Utils.getNowTime("yyyy-MM-dd HH:mm:ss");
										msg_refresh_terminal = "终端: "+Build.MODEL;
										preferData.writeData("msgRefreshTime", msg_refresh_time);
										preferData.writeData("msgRefreshTerminal", msg_refresh_terminal);
										mPullToRefreshView.onHeaderRefreshComplete(msg_refresh_time, msg_refresh_terminal);
									}
									ArrayList<HashMap<String, String>> listObj = (ArrayList<HashMap<String, String>>) msg.obj;
									if (listObj != null) {
										xmlData.updateList(listObj);
										msgList = listObj;
										msgAdapter = new MessageItemAdapter(mActivity, msgList);
										lv_list.setAdapter(msgAdapter);
									} else {
										xmlData.deleteAllItem();
										if (msgList != null) {
											msgList.clear();
											msgAdapter.notifyDataSetChanged();
										}
									}
									Value.newAlarmMessageCount = 0;
									break;
								//上拖请求，请求以前的报警消息
								case 1:
									if (mPullToRefreshView.getFooterState() == PullToRefreshView.REFRESHING) {
										mPullToRefreshView.onFooterRefreshComplete();
									}
									ArrayList<HashMap<String, String>> list1 = (ArrayList<HashMap<String, String>>) msg.obj;
									if (list1 != null) {
										int len = list1.size();
										for (int i=0; i<len; i++) {
											msgList.add(list1.get(i));
											xmlData.addItem(list1.get(i));
										}
										msgAdapter.notifyDataSetChanged();
									}
									break;
								//请求新的报警数据
								case 2:
									ArrayList<HashMap<String, String>> list2 = (ArrayList<HashMap<String, String>>) msg.obj;
									if (list2 != null) {
										int len = list2.size();
										for (int i=0; i<len; i++) {
											msgList.add(0, list2.get(i));
										}
										xmlData.updateList(msgList);
										msgAdapter.notifyDataSetChanged();
									}
									Value.newAlarmMessageCount = 0;
									break;
								//删除该条报警
								case 3:
									msgList.remove(listPosition);
									xmlData.deleteItem(mMsgID);
									msgAdapter.notifyDataSetChanged();
									reqAlarmType = 1;
									requestAlarmEvent(xmlData.getMinUpdateID(), 1);
									break;
								//删除当前全部报警
								case 4:
									msgList.removeAll(msgList);
									xmlData.deleteAllItem();
									msgAdapter.notifyDataSetChanged();
									reqAlarmType = 0;
									requestAlarmEvent(0, 5);
									break;
								//标记该条报警
								case 5:
									msgList.get(listPosition).put("isReaded", "true");
									xmlData.updateItemState(mMsgID, "true");
									msgAdapter.notifyDataSetChanged();
									break;
								//标记当前全部报警
								case 6:
									for (int i=0; i<listSize; i++) {
										msgList.get(i).put("isReaded", "true");
										xmlData.updateItemState(msgList.get(i).get("isReaded"), "true");
									}
									msgAdapter.notifyDataSetChanged();
									break;
								default: break;
							}
							//实时更新列表的大小和未读报警消息的数量
							listSize = xmlData.getListSize();
							unreadAlarmCount = getAlarmCount();
							MainActivity.setAlarmIconAndText(unreadAlarmCount);
							preferData.writeData("AlarmCount", unreadAlarmCount);
						} else {
							Toast.makeText(mActivity, msg.obj+","+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.login_id);
					}
					break;
			}
//			if (listSize == 0) {
//				noMsgLayout.setVisibility(View.VISIBLE);
//			} else {
//				noMsgLayout.setVisibility(View.INVISIBLE);
//			}
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
		if (handler != null) {
			handler.sendMessage(msg);
		}
	}
	
	/**
	 * 请求报警消息
	 */
	public void requestAlarmEvent(int msgId, int count) {
		if (Utils.isNetworkAvailable(mActivity)) {
			String data = MainApplication.getInstance().generateRequestAlarmJson(msgId, count);
			sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, data);
			sendHandlerMsg(IS_REQUESTING);
			sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
		} else {
			Toast.makeText(mActivity, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	/**
	 * 上拖刷新
	 */
	@Override
	public void onFooterRefresh(PullToRefreshView view) {
		mPullToRefreshView.postDelayed(new Runnable() {
			@Override
			public void run() {
				reqAlarmType = 1;
				String data = MainApplication.getInstance().generateRequestAlarmJson(xmlData.getMinUpdateID(), 5);
				sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
				sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, data);
			}
		}, 1000);
	}
	
	/**
	 * 下拉刷新
	 * yyyy-MM-dd hh:mm:ss 12小时制  yyyy-MM-dd HH:mm:ss 24小时制
	 */
	@Override
	public void onHeaderRefresh(PullToRefreshView view) {
		mPullToRefreshView.postDelayed(new Runnable() {
			@Override
			public void run() {
				reqAlarmType = 0;
				String data = MainApplication.getInstance().generateRequestAlarmJson(0, 5);
				sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
				sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, data);
			}
		}, 1000);
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			
		}
	}
	
	/**
	 * 设备项ListView的点击事件，标记该条已读
	 */
	private class OnItemClickListenerImpl implements OnItemClickListener {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
			// TODO Auto-generated method stub
			listPosition = position;
			HashMap<String, String> item = msgList.get(listPosition);
			mMsgID = item.get("msgID");
			String sendData = generateMarkThisItemJson(Integer.parseInt(mMsgID));
			if (sendData != null) {
				reqAlarmType = 5;
				sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
				sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, sendData);
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
			showPopupWindow(mPullToRefreshView);
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
		item_list.add("删除该条报警");
		item_list.add("删除该页报警");
		item_list.add("标记该条已读");
		item_list.add("全部标记已读");
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
				
				HashMap<String, String> item = msgList.get(listPosition);
				mMsgID = item.get("msgID");
				String sendData = null;
				
				switch (position) {
					case 0: //删除该条报警
						reqAlarmType = 3;
						sendData = generateDeleteThisItemJson(Integer.parseInt(mMsgID));
						sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, sendData);
						break;
					case 1: //删除当前全部报警
						reqAlarmType = 4;
						sendData = generateDeleteThisListJson();
						sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, sendData);
						break;
					case 2: //标记该条报警
						reqAlarmType = 5;
						sendData = generateMarkThisItemJson(Integer.parseInt(mMsgID));
						if (sendData == null) {
							Toast.makeText(mActivity, "该条报警已标记！", Toast.LENGTH_SHORT).show();
						} else {
							sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
							sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, sendData);
						}
						break;
					case 3: //标记当前全部报警
						reqAlarmType = 6;
						sendData = generateMarkThisListJson();
						if (sendData == null) {
							Toast.makeText(mActivity, "当前全部报警已标记！", Toast.LENGTH_SHORT).show();
						} else {
							sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
							sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, sendData);
						}
						break;
				}
				if (mPopupWindow.isShowing()) {
					mPopupWindow.dismiss();
				}
			}
		});
	}
	
	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		mActivity.unregisterReceiver(backstageMessageReceiver);
	}

	@Override
	public void onDestroyView() {
		// TODO Auto-generated method stub
		super.onDestroyView();
	}
	
	/**
	 * @author sunfusheng
	 * 报警消息的广播接收
	 */
	public class BackstageMessageReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			String action = intent.getAction();
			if ((Value.isNeedReqAlarmListFlag) && (action.equals(BackstageService.BACKSTAGE_MESSAGE_ACTION))) {
				// 请求新的报警消息
				reqAlarmType = 2;
				Value.ownFragmentRequestAlarmFlag = false;
				requestAlarmEvent(0, Value.newAlarmMessageCount);
			}
			else if (action.equals(READ_MESSAGE_ACTION)) {
				// 标记为已读
				HashMap<String, String> item = null;
				String id = (String) intent.getCharSequenceExtra("listMsgID");
				for (int i=0; i<listSize; i++) {
					item = msgList.get(i);
					if (id.equals(item.get("msgID"))) {
						mMsgID = item.get("msgID");
						listPosition = i;
						break;
					}
				}
				String sendData = generateMarkThisItemJson(Integer.parseInt(mMsgID));
				if (sendData != null) {
					reqAlarmType = 5;
					sendHandlerMsg(REQUEST_TIMEOUT, Value.REQ_TIME_10S);
					sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, sendData);
				}
			}
		}
	}

}
