package com.video.service;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.app.Application;
import android.content.Intent;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.data.XmlDevice;
import com.video.data.XmlMessage;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

public class MainApplication extends Application {

	private static MainApplication AppInstance = null;
	public static MainApplication getInstance() {  
        return AppInstance;  
    }
	
	// 用户信息
	public String userName = "";
	public String userPwd = "";
	
	// 保存为缓存文件句柄
	public PreferData preferData = null;
	public XmlDevice xmlDevice = null;
	
	// 应用程序文件夹路径
	public boolean isSDExist = false;
	public String SDPath = null; 
	public String KaerVideoPath = null;
	public File KaerVideoFile = null; 
	public String imagePath = null;
	public File imageFile = null; 
	public String videoPath = null;
	public File videoFile = null; 
	public String cachePath = null;
	public File cacheFile = null;
	public String thumbnailsPath = null;
	public File thumbnailsFile = null; 
	
	// 终端设备列表
	public ArrayList<HashMap<String, String>> deviceList = null;
	public int unreadAlarmCount = 0;
	
	@Override  
    public void onCreate() {  
        // TODO Auto-generated method stub  
        super.onCreate();  
        initData();
    }
	
	/**
	 * 初始化数据
	 */
	public void initData() {
		AppInstance = this;
		
		// 初始化缓存文件句柄
		if (xmlDevice == null) {
			xmlDevice = new XmlDevice(AppInstance);
		}
		if (preferData == null) {
			preferData = new PreferData(AppInstance);
		}
		if (deviceList == null) {
			deviceList = new ArrayList<HashMap<String, String>>();
		}

		// 建立应用程序相应文件夹
		if (Utils.checkSDCard()) {
			isSDExist = true;
			SDPath = Environment.getExternalStorageDirectory().getAbsolutePath();
			// 父目录
			KaerVideoPath = SDPath + File.separator + "KaerVideo";
			KaerVideoFile = new File(KaerVideoPath);
			if(!KaerVideoFile.exists()){
				KaerVideoFile.mkdir();
			} 
			// 抓拍图片
			imagePath = KaerVideoPath + File.separator + "image";
			imageFile = new File(imagePath);
			if(!imageFile.exists()){
				imageFile.mkdir();
			}
			// 本地视频
			videoPath = KaerVideoPath + File.separator + "video";
			videoFile = new File(videoPath);
			if(!videoFile.exists()){
				videoFile.mkdir();
			}
			// 文件缓存
			cachePath = KaerVideoPath + File.separator + "cache";
			cacheFile = new File(cachePath);
			if(!cacheFile.exists()){
				cacheFile.mkdir();
			}
			// 缩略图
			thumbnailsPath = KaerVideoPath + File.separator + "thumbnails";
			thumbnailsFile = new File(thumbnailsPath);
			if(!thumbnailsFile.exists()){
				thumbnailsFile.mkdir();
			} 
		} else {
			// SD卡不存在
			isSDExist = false;
		}
		
		// 初始化全局变量
		deviceList = xmlDevice.readXml();
	}
	
	/**
	 * 获得设备列表的大小
	 */
	public int getDeviceListSize() {
		if (deviceList != null) {
			return deviceList.size();
		}
		return 0;
	}
	
	/**
	 * 通过deviceID从设备列表获得指定设备的position
	 */
	public int getDeviceListPositionByDeviceID(String deviceID) {
		int size = getDeviceListSize();
		for (int i=0; i<size; i++) {
			if (deviceList.get(i).get("deviceID").equals(deviceID)) {
				return i;
			}
		}
		return -1;
	}
	
	/**
	 * 通过dealerName从设备列表获得指定设备的position
	 */
	public int getDeviceListPositionByDealerName(String dealerName) {
		String[] sArray = dealerName.split("-");
		int size = getDeviceListSize();
		for (int i=0; i<size; i++) {
			if (deviceList.get(i).get("deviceID").equalsIgnoreCase(sArray[0].trim())) {
				return i;
			}
		}
		return -1;
	}
	
	/**
	 * 获得在线设备列表
	 */
	public ArrayList<HashMap<String, String>> getOnlineDeviceList() {
		int len = getDeviceListSize();
		ArrayList<HashMap<String, String>> listObj = new ArrayList<HashMap<String, String>>();
		
		for (int i=0; i<len; i++) {
			HashMap<String, String> item = deviceList.get(i);
			if (item.get("isOnline").equals("true")) {
				listObj.add(item);
			}
		}
		return listObj;
	}
	
	/**
	 * 发送Handler消息
	 */
	public void sendHandlerMsg(Handler handler, int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, int arg1) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, int arg1, int arg2) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, int arg1, int arg2, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, int arg1, int arg2, Object obj) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.obj = obj;
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
	public void sendHandlerMsg(Handler handler, int what, int arg1, int arg2, String obj, int timeout) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.obj = obj;
		handler.sendMessageDelayed(msg, timeout);
	}
	
	/**
	 * 生成JSON的turn和stun字符串
	 */
	public String generateStunandTurnJson(String turn, String stun) {
		JSONArray jsonArr = new JSONArray();
		JSONObject jsonObj = null;
		try {
			jsonObj = new JSONObject();
			jsonObj.put("uri", turn);
			jsonArr.put(jsonObj);
			jsonObj = new JSONObject();
			jsonObj.put("uri", stun);
			jsonArr.put(jsonObj);
			return jsonArr.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的realm字符串
	 */
	public String generateRealmJson() {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_Getrealm");
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的登录字符串
	 */
	public String generateLoginJson(String username, String pwd) {
		String newPwd = username+":"+Value.realm+":"+pwd;
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_Login");
			jsonObj.put("UserName", username);
			jsonObj.put("Pwd", Utils.CreateMD5Pwd(newPwd));
			return jsonObj.toString();
		} catch (JSONException e) {
			Utils.log("generateLoginJson()异常！");
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的注销登录字符串
	 */
	private String generateLogoutJson() {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_Logout");
			jsonObj.put("UserName", MainApplication.getInstance().userName);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	public void sendLogoutData() {
		// 发送注销消息
		String data = generateLogoutJson();
		sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
	}
	
	/**
	 * 生成JSON的请求报警数据字符串
	 */
	public String generateRequestAlarmJson(int msgId, int count) {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_ReqAlarm");
			jsonObj.put("UserName", userName);
			jsonObj.put("ID", msgId);
			jsonObj.put("Count", count);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 生成JSON的查看未读报警消息数字符串
	 */
	public String generateUnreadAlarmCountJson() {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_NotReadAlarm");
			jsonObj.put("UserName", userName);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 请求未读报警消息数事件
	 */
	public void requestUnreadAlarmCountEvent() {
		String data = generateUnreadAlarmCountJson();
		MainApplication.getInstance().sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_alarm_id, data);
	}
	
	/**
	 * 终止主程序和服务
	 */
	public void stopActivityandService() {
		// 清除报警消息缓存和未读消息数量
		XmlMessage xmlData = new XmlMessage(AppInstance);
		xmlData.deleteAllItem();
		if (preferData.isExist("AlarmCount")) {
			preferData.deleteItem("AlarmCount");
        }
		
		// 删除缓存和缩略图文件夹
		Utils.deleteDirectoryFiles(cacheFile);
		Utils.deleteDirectoryFiles(thumbnailsFile);
		
		// 复位全局变量
		Value.resetValues();
		
		// 停止后台服务
		Intent intent = new Intent(MainApplication.getInstance(), BackstageService.class);
    	MainApplication.getInstance().stopService(intent);
	}
}
