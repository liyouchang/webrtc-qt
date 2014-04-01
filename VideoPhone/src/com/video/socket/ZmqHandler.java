package com.video.socket;

import org.json.JSONException;
import org.json.JSONObject;

import com.video.R;

import android.os.Handler;
import android.os.Message;

public class ZmqHandler extends Handler {
	
	private static Handler mHandler = null;
	
	public static void setHandler(Handler handler) {
		mHandler = handler;
	}
	
	@Override
	public void handleMessage(Message msg) {
		String recvData = (String)msg.obj;
		JSONObject obj = null;
		try {
			obj = new JSONObject(recvData);
			String type = obj.getString("type");
			if (mHandler != null) {
				//◊¢≤·
				if (type.equals("Client_Registration")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.register_id, resultCode, 0).sendToTarget();
				}
				//µ«¬º
				else if (type.equals("Client_Login")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.login_id, resultCode, 0).sendToTarget();
				}
				//÷ÿ÷√√‹¬Î
				else if (type.equals("Client_ResetPwd")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.find_pwd_id, resultCode, 0).sendToTarget();
				}
				//–ﬁ∏ƒ√‹¬Î
				else if (type.equals("Client_ChangePwd")) {
					int resultCode = obj.getInt("Result");
					mHandler.obtainMessage(R.id.modify_pwd_id, resultCode, 0).sendToTarget();
				}
				//ÃÌº”…Ë±∏
				else if (type.equals("Client_AddTerm")) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						String dealerName = obj.getString("DealerName");
						mHandler.obtainMessage(R.id.add_device_id, resultCode, 0, dealerName).sendToTarget();
					} else {
						mHandler.obtainMessage(R.id.add_device_id, resultCode, 0).sendToTarget();
					}
				}
			}
		} catch (JSONException e) {
			e.printStackTrace();
		}
	}
}
