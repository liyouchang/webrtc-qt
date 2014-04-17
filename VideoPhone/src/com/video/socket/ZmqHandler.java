package com.video.socket;

import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.os.Handler;
import android.os.Message;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.main.MainActivity;
import com.video.main.OwnFragment;

public class ZmqHandler extends Handler {
	
	private static Handler mHandler = null;
	
	public static void setHandler(Handler handler) {
		mHandler = handler;
	}
	
	/**
	 * 转换获得的状态，将int类型转为String类型
	 */
	private String getState(int state) {
	    String isActiveString = "false";
	    if (state == 1) {
	    	isActiveString = "true";
	    } else {
	    	isActiveString = "false";
	    }
	    return isActiveString;
	}
	
	/**
	 * 解析获得的请求终端列表JSONArray数据
	 */
	private ArrayList<HashMap<String, String>> getReqTermList(JSONArray jsonArray) {
		ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();
		HashMap<String, String> item = null;
		int len = jsonArray.length();
		  
	    try {
	    	for (int i=0; i<len; i++) { 
		    	JSONObject obj = (JSONObject) jsonArray.get(i); 
		    	item = new HashMap<String, String>();
			    item.put("deviceName", obj.getString("TermName")); 
			    item.put("deviceID", obj.getString("MAC"));
			    item.put("isOnline", getState(obj.getInt("Active")));
				item.put("dealerName", obj.getString("DealerName"));
				list.add(item);
	    	}
	    	return list;
		} catch (JSONException e) {
			e.printStackTrace();
			System.out.println("MyDebug: getReqTermList()异常！");
		}
		return null;
	}
	
	/**
	 * 解析获得的请求报警数据JSONArray数据
	 */
	private ArrayList<HashMap<String, String>> getReqAlarmList(JSONArray jsonArray) {
		ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();
		HashMap<String, String> item = null;
		int len = jsonArray.length();
		  
	    try {
	    	for (int i=0; i<len; i++) { 
		    	JSONObject obj = (JSONObject) jsonArray.get(i); 
		    	item = new HashMap<String, String>();
			    item.put("msgMAC", "来自: "+obj.getString("MAC")); 
			    item.put("msgEvent", "事件: "+obj.getString("AlarmInfo"));
				item.put("msgTime", obj.getString("DateTime"));
				item.put("imageURL", obj.getString("PictureURL"));
				item.put("msgID", obj.getString("ID"));
				item.put("isReaded", getState(obj.getInt("IsUsed")));
				list.add(item);
	    	}
	    	return list;
		} catch (JSONException e) {
			e.printStackTrace();
			System.out.println("MyDebug: getReqAlarmList()异常！");
		}
		return null;
	}
	
	@Override
	public void handleMessage(Message msg) {
		String recvData = (String)msg.obj;
		JSONObject obj = null;
		try {
			obj = new JSONObject(recvData);
			String type = obj.getString("type");
			//报警消息推送
			if (type.equals("Backstage_message")) {
				Value.isNeedReqAlarmListFlag = true;
				Value.requstAlarmCount++;
				PreferData preferData = new PreferData(MainActivity.mContext);
				if (preferData.isExist("AlarmCount")) {
					int alarmCount = preferData.readInt("AlarmCount");
					alarmCount++;
					preferData.writeData("AlarmCount", alarmCount);
					MainActivity.mainHandler.obtainMessage(0, alarmCount, 0).sendToTarget();
				}
			}
			//终端上下线消息推送
			else if (type.equals("Backstage_TermActive")) {
				HashMap<String, String> item = new HashMap<String, String>();
			    item.put("deviceID", obj.getString("MAC"));
			    item.put("isOnline", getState(obj.getInt("Active")));
				item.put("dealerName", obj.getString("DealerName"));
				OwnFragment.ownHandler.obtainMessage(0, item).sendToTarget();
			}
			//客户端和服务器的心跳
			else if (type.equals("Client_BeatHeart")) {
				int resultCode = obj.getInt("Result");
				if (resultCode != 0) {
					Value.isLoginSuccess = true;
				}
			}
			//各个界面下的handler操作
			else if (mHandler != null) {
				//注册
				if (type.equals("Client_Registration")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.register_id, resultCode, 0).sendToTarget();
				}
				//登录
				else if (type.equals("Client_Login")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.login_id, resultCode, 0).sendToTarget();
				}
				//重置密码
				else if (type.equals("Client_ResetPwd")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.find_pwd_id, resultCode, 0).sendToTarget();
				}
				//修改密码
				else if (type.equals("Client_ChangePwd")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.modify_pwd_id, resultCode, 0).sendToTarget();
				}
				//添加终端
				else if (type.equals("Client_AddTerm")) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						String dealerName = obj.getString("DealerName");
						mHandler.obtainMessage(R.id.add_device_id, resultCode, 0, dealerName).sendToTarget();
					} else {
						mHandler.obtainMessage(R.id.add_device_id, resultCode, 0).sendToTarget();
					}
				}
				//请求终端列表
				else if (type.equals("Client_ReqTermList")) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						JSONArray jsonArray = obj.getJSONArray("Terminal");
						mHandler.obtainMessage(R.id.request_terminal_list_id, resultCode, 0, getReqTermList(jsonArray)).sendToTarget();
					} else {
						mHandler.obtainMessage(R.id.request_terminal_list_id, resultCode, 0, "请求终端列表失败").sendToTarget();
					}
				}
				//修改终端名称
				else if (type.equals("Client_ModifyTerm")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.modify_device_name_id, resultCode, 0).sendToTarget();
				}
				//删除终端绑定
				else if (type.equals("Client_DelTerm")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.delete_device_item_id, resultCode, 0).sendToTarget();
				}
				//请求报警数据
				else if (type.equals("Client_ReqAlarm")) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						JSONArray jsonArray = obj.getJSONArray("AlarmData");
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, getReqAlarmList(jsonArray)).sendToTarget();
					} else {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, "请求报警数据失败").sendToTarget();
					}
				}
				//删除该条报警
				else if (type.equals("Client_DelAlarm")) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0).sendToTarget();
					} else {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, "删除该条报警失败").sendToTarget();
					}
				}
				//删除当前全部报警
				else if (type.equals("Client_DelSelectAlarm")) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0).sendToTarget();
					} else {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, "删除当前全部报警失败").sendToTarget();
					}
				}
				//标记该条报警
				else if (type.equals("Client_MarkAlarm")) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0).sendToTarget();
					} else {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, "标记该条报警失败").sendToTarget();
					}
				}
				//标记当前全部报警
				else if (type.equals("Client_MarkSelectAlarm")) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0).sendToTarget();
					} else {
						mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, "标记当前全部报警失败").sendToTarget();
					}
				}
			}
		} catch (JSONException e) {
			e.printStackTrace();
		}
	}
}
