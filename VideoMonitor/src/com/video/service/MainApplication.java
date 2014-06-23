package com.video.service;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.Application;
import android.content.Intent;
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
	 * 从设备列表获得指定设备的position
	 */
	public int getDeviceListPosition(String dealerName) {
		if (MainApplication.getInstance().deviceList != null) {
			int size = MainApplication.getInstance().deviceList.size();
			for (int i=0; i<size; i++) {
				if (MainApplication.getInstance().deviceList.get(i).get("dealerName").equals(dealerName)) {
					return i;
				}
			}
			return -1;
		}
		return -1;
	}
	
	/**
	 * 发送更新设备列表状态的广播
	 */
	public void sendChangeDeviceListBroadcast() {
		Intent actionIntent = new Intent();
		actionIntent.setAction(BackstageService.CHANGE_DEVICE_LIST_ACTION);
		sendBroadcast(actionIntent);
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
