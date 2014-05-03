package com.video.utils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.HashMap;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;

public class UpdateAPK {

	private Context mContext = null;
	private static final int APK_IS_NEED_UPGRADE = 1;//检查是否升级APK应用软件
	private static final int APK_IS_DOWNLOADING = 2;//正在下载apk应用软件
	private static final int DOWNLOAD_PERCENT = 3;//下载apk应用软件的进度
    private static final int APK_DOWNLOAD_FINISH = 4;//apk应用软件下载完成
	
	private int apkLeagth = 0; //apk文件大小
	private String apkSavePath; //apk下载保存的目录
	private String apkName = "VideoMonitor.apk"; //下载安装的软件名称
	public static HashMap<String, String> xmlHashMap; //存储应用软件xml的信息
	private boolean isCancelUpgrade = false; //是否取消升级的标志
	
    private ProgressBar progressBar_upgrade_percent;
    private TextView tv_upgrade_percent;
	private AlertDialog downLoadDialog;
	
	public UpdateAPK(Context context) {
		this.mContext = context;
	}
	
	public void startCheckUpgadeThread() {
		new CheckUpgadeThread().start();
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				//检查是否升级APK应用软件
				case APK_IS_NEED_UPGRADE:
					showUpgradeVersionDialog();
					break;
				//正在下载apk应用软件
				case APK_IS_DOWNLOADING:
					showDownloadDialog();
					break;
				//下载apk应用软件的进度
				case DOWNLOAD_PERCENT:
					progressBar_upgrade_percent.setProgress(msg.arg1);
					tv_upgrade_percent.setText(msg.arg1+"%");
					break;
				//apk应用软件下载完成
				case APK_DOWNLOAD_FINISH:
					progressBar_upgrade_percent.setProgress(100);
					tv_upgrade_percent.setText("100%");
					installAPK();
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
	private void sendHandlerMsg(int what, int arg1) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		handler.sendMessage(msg);
	}
	
	/**
	 * 检查APK是否升级的线程
	 */
	public class CheckUpgadeThread extends Thread {
		@Override
		public void run() {
			// TODO Auto-generated method stub
			super.run();
			try {
				if (isNeedUpgradeAPK()) {
					sendHandlerMsg(APK_IS_NEED_UPGRADE);
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * 软件需要升级的对话框
	 */
	private void showUpgradeVersionDialog() {
		LayoutInflater mInflater = LayoutInflater.from(mContext);
		View view = mInflater.inflate(R.layout.upgrade_content, null);
		StringBuffer info = new StringBuffer();
		info.append("微视界更新信息\n");
		info.append("新版本号："+xmlHashMap.get("versionName")+"\n");
		info.append("软件大小："+xmlHashMap.get("apkLeagth")+"\n");
		info.append("更新原因："+xmlHashMap.get("upgradeReason"));
		TextView tv_info = (TextView)view.findViewById(R.id.tv_content);
		tv_info.setText(info);
		AlertDialog aboutDialog = new AlertDialog.Builder(mContext)
			.setTitle("微视界已更新")
			.setIcon(R.drawable.upgrade_icon)
			.setView(view)
			.setCancelable(false)
			.setPositiveButton("现在升级",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int whichButton) {
							sendHandlerMsg(APK_IS_DOWNLOADING);
							dialog.dismiss();
						}
					})
			.setNegativeButton("忽略升级",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int which) {
							dialog.dismiss();
						}
					}).create();
		aboutDialog.show();
	}
	
	/**
	 * 正在升级的对话框
	 */
	private void showDownloadDialog() {
		
		LayoutInflater mInflater = LayoutInflater.from(mContext);
		View view = mInflater.inflate(R.layout.download_progressbar, null);
		StringBuffer info = new StringBuffer();
		info.append("微视界更新信息\n");
		info.append("新版本号："+xmlHashMap.get("versionName")+"\n");
		info.append("软件大小："+xmlHashMap.get("apkLeagth")+"\n");
		info.append("更新原因："+xmlHashMap.get("upgradeReason"));
		TextView tv_info = (TextView)view.findViewById(R.id.tv_upgrade_info);
		tv_info.setText(info);
		progressBar_upgrade_percent = (ProgressBar)view.findViewById(R.id.progressBar_percent);
		progressBar_upgrade_percent.setProgress(0);
		tv_upgrade_percent = (TextView)view.findViewById(R.id.tv_percent);
		tv_upgrade_percent.setText("0%");
		downLoadDialog = new AlertDialog.Builder(mContext)
			.setTitle("正在下载软件包")
			.setIcon(R.drawable.download_icon)
			.setView(view)
			.setCancelable(false)
			.setPositiveButton("取消升级",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int whichButton) {
							isCancelUpgrade = true;
							dialog.dismiss();
						}
					}).create();
		downLoadDialog.show();
		//启动下载APK应用软件的线程
		 new downloadingApkThread().start();
	}
	
	/**
	 * Http请求连接网络
	 * @param path请求网络的路径地址
	 * @return 返回请求网络成功的数据流
	 * @throws Exception
	 */
	private InputStream httpRequestNetWork(String path) throws Exception {
		InputStream inputStream = null;
		//Http GET方式连接服务器
		URL url = new URL(path);
		
		HttpURLConnection connection = (HttpURLConnection) url.openConnection();
		connection.setConnectTimeout(5*1000);
		connection.setDoInput(true);
		connection.setRequestMethod("GET");
		
		int code = connection.getResponseCode();
		if (code == 200) {
			inputStream = connection.getInputStream();
			//获得apk文件大小、输入流
			apkLeagth = connection.getContentLength();
			return inputStream;
		}
		connection.disconnect();
		return null;
	}
	
	public HashMap<String, String> parseXml(InputStream inputStream) throws XmlPullParserException, IOException {
		boolean flag = true;
		HashMap<String, String> xmlHashMap = null;
		
		//创建xml PULL的解析器
		XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
		XmlPullParser parser = factory.newPullParser();
		parser.setInput(inputStream, "UTF-8");
		int eventType = parser.getEventType();
		
		while (eventType != XmlPullParser.END_DOCUMENT) {
			switch (eventType) {
				case XmlPullParser.START_DOCUMENT:
					xmlHashMap = new HashMap<String, String>();
					break;
				case XmlPullParser.START_TAG:
					String tagName = parser.getName();
					if ((tagName.equalsIgnoreCase("VERSIONCODE")) && (true == flag)) {
						xmlHashMap.put("versionCode", parser.nextText().trim());
					}
					else if ((tagName.equalsIgnoreCase("APKURL")) && (true == flag)) {
						xmlHashMap.put("apkDownLoadUrl", parser.nextText().trim());
					}
					else if ((tagName.equalsIgnoreCase("FILENAME")) && (true == flag)) {
						xmlHashMap.put("fileName", parser.nextText().trim());
					}
					else if ((tagName.equalsIgnoreCase("VERSIONNAME")) && (true == flag)) {
						xmlHashMap.put("versionName", parser.nextText().trim());
					}
					else if ((tagName.equalsIgnoreCase("UPGRADEREASON")) && (true == flag)) {
						xmlHashMap.put("upgradeReason", parser.nextText().trim());
					}
					else if ((tagName.equalsIgnoreCase("APKLEAGTH")) && (true == flag)) {
						xmlHashMap.put("apkLeagth", parser.nextText().trim());
					}
					break;
				case XmlPullParser.END_TAG:
					flag = false;
					break;
			}
			eventType = parser.next();
		}
		
		return xmlHashMap;
	}
	
	/**
	 * 连接服务器检查应用软件是否需要升级
	 * @return 返回true:有新版本  false:没有新版本 
	 * @throws Exception
	 */
	private boolean isNeedUpgradeAPK() throws Exception {
		
		InputStream inputStream = httpRequestNetWork(Value.ApkXmlPath);
		if (inputStream != null) {
			int localVersionCode = Tools.getVersionCode(mContext);
			
			//解析从服务器上读取的xml文件
			xmlHashMap = parseXml(inputStream);
			
			//比较服务器与本地APK的软件版本
			if (null != xmlHashMap) {
				int serverVersionCode = Integer.valueOf(xmlHashMap.get("versionCode"));
				if (serverVersionCode > localVersionCode)
					return true;
				return false;
			}
		}
		//关闭输入流
		inputStream.close();
		return false;
	}
	
	/**
	 * 下载APK应用软件的线程
	 */
	public class downloadingApkThread extends Thread {

		@Override
		public void run() {
			// TODO Auto-generated method stub
			super.run();
			if (Utils.checkSDCard()) {
				apkSavePath = Environment.getExternalStorageDirectory() + File.separator + "KaerVideo" + File.separator + "cache";
				try {
					//Http请求连接网络
					InputStream inputStream = httpRequestNetWork(xmlHashMap.get("apkDownLoadUrl"));
					if (inputStream != null) {
						//确定文件目录的存在，没有创建目录
						File file = new File(apkSavePath);
						System.out.println("MyDebug: "+"APK下载目录 : "+file.toString());
						if (!file.exists()) {
							file.mkdir();
						}
						File apkFile = new File(apkSavePath, apkName);
						FileOutputStream fileOutputStream = new FileOutputStream(apkFile);
						byte[] buf = new byte[2*1024];
						int count = 0;
						do {
							//下载apk数据
							int readCount = inputStream.read(buf);
							sleep(5);
							//处理下载界面的显示
							count += readCount;
							int percent = (int)(((float)count/apkLeagth)*100);
							sendHandlerMsg(DOWNLOAD_PERCENT, percent);
							if (readCount < 0) {
								downLoadDialog.dismiss();
								sendHandlerMsg(APK_DOWNLOAD_FINISH);
							}
							//将下载的数据保存在SD卡上
							fileOutputStream.write(buf, 0, readCount);
						} while(!isCancelUpgrade);
						//关闭输入、输出流
						inputStream.close();
						fileOutputStream.close();
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			} else {
				Toast.makeText(mContext, "您手机的SD卡不可用！", Toast.LENGTH_SHORT).show();
			}
		}
	}
	
	/** 
     * 安装应用程序
     */ 
	public void installAPK() {  
		File apkfile = new File(apkSavePath, apkName); //xmlHashMap.get("fileName")+
        if (!apkfile.exists()) 
        	return ; 
        Intent intent = new Intent(Intent.ACTION_VIEW);  
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);  
        intent.setDataAndType(Uri.parse("file://" + apkfile.toString()), "application/vnd.android.package-archive");  
        mContext.startActivity(intent);  
        android.os.Process.killProcess(android.os.Process.myPid());
    }
}
