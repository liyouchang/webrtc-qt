package com.video.service;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.Application;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;

import com.video.data.PreferData;
import com.video.data.XmlDevice;
import com.video.utils.Utils;

public class MainApplication extends Application {

	private static MainApplication AppInstance = null;
	public static MainApplication getInstance() {  
        return AppInstance;  
    }
	
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
}
