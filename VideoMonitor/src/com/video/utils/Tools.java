package com.video.utils;

import java.io.File;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;

import com.video.R;


public class Tools {
	
	/**
	 * Byte[]数据拷贝
	 */
	public static void CopyByteArray(byte[] dst, byte[] src, int len) {
		System.arraycopy(src, 0, dst, 0, len);
	}
	
	/**
	 * Byte[]数据拷贝
	 */
	public static void CopyByteArray(byte[] dst, int dstPos, byte[] src, int srcPos, int len) {
		System.arraycopy(src, srcPos, dst, dstPos, len);
	}
	
	/**
	 * 设置short的值
	 */
	public static void setWordValue(byte[] buffer, int offset, short value) {
		buffer[offset] = (byte) (value & 0x00FF);
		buffer[offset + 1] = (byte) ((value & 0xFF00) >> 8);
	}

	/**
	 * 获得short的值
	 */
	public static int getWordValue(byte[] buffer, int offset) {
		int result = 0;
		result = (byte) (0xFF & buffer[offset + 1]);
		result = (int) ((result << 8) & 0x0000FF00);
		result += (int) (buffer[offset] & 0x000000FF);
		return result;
	}
	
	/**
	 * 设置int的值
	 */
	public static void setIntValue(byte[] buffer, int offset, int value) {
		buffer[offset] = (byte) ((value & 0x000000FF));
		buffer[offset + 1] = (byte) ((value & 0x0000FF00) >> 8);
		buffer[offset + 2] = (byte) ((value & 0x00FF0000) >> 16);
		buffer[offset + 3] = (byte) ((value & 0xFF000000) >> 24);
	}
	
	/**
	 * 获得int的值
	 */
	public static int getIntValue(byte[] buffer, int offset) {
		int result = 0;
		result = (0xFF & buffer[offset]) + 
				 ((0xFF & buffer[offset + 1]) << 8) + 
				 ((0xFF & buffer[offset + 2]) << 16) + 
				 ((0xFF & buffer[offset + 3]) << 24);
		return result;
	}
	
	/**
	 * 获得软件的版本号
	 * @param context 上下文
	 * @return 返回版本号
	 */
	public static int getVersionCode(Context context) {
		int versionCode = -1;
		try {
			versionCode = context.getPackageManager().getPackageInfo("com.video", 0).versionCode;
		} catch (NameNotFoundException e) {
			e.printStackTrace();
		}
		return versionCode;
	}

	/**
	 * 获得软件的版本名称
	 * @param context 上下文
	 * @return 返回版本名称
	 */
	public static String getVersionName(Context context) {
		String versionName = "";
		try {
			versionName = context.getPackageManager().getPackageInfo("com.video", 0).versionName;
		} catch (NameNotFoundException e) {
			e.printStackTrace();
		}
		return versionName;
	}
	
	/**
	 * 获得应用程序的名称
	 * @param context 上下文
	 * @return 返回应用程序名称
	 */
	public static String getAppName(Context context) {
		return context.getResources().getText(R.string.app_name).toString();
	}
	
	/** 
     * 安装应用程序
     */ 
	public static void installAPK(Context context, String apkSavePath, String packageName) {  
		File apkfile = new File(apkSavePath, packageName);
        if (!apkfile.exists()) 
        	return ; 
        Intent intent = new Intent(Intent.ACTION_VIEW);  
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);  
        intent.setDataAndType(Uri.parse("file://" + apkfile.toString()), "application/vnd.android.package-archive");  
        context.startActivity(intent);  
        android.os.Process.killProcess(android.os.Process.myPid());
    }
	
    /** 
     * 卸载应用程序
     */  
    public static void uninstallAPK(Context context, String packageName) {  
        Uri packageURI = Uri.parse("package:" + packageName);  
        Intent uninstallIntent = new Intent(Intent.ACTION_DELETE, packageURI);  
        context.startActivity(uninstallIntent);  
    }
}
