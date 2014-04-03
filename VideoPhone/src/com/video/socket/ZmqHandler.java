package com.video.socket;

import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.os.Handler;
import android.os.Message;

import com.video.R;

public class ZmqHandler extends Handler {
	
	private static Handler mHandler = null;
	
	public static void setHandler(Handler handler) {
		mHandler = handler;
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
			    item.put("isOnline", "false");
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
	
	@Override
	public void handleMessage(Message msg) {
		String recvData = (String)msg.obj;
		JSONObject obj = null;
		try {
			obj = new JSONObject(recvData);
			String type = obj.getString("type");
			if (mHandler != null) {
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
						mHandler.obtainMessage(R.id.request_terminal_list_id, resultCode, 0).sendToTarget();
					}
				}
			}
		} catch (JSONException e) {
			e.printStackTrace();
		}
	}
}
