package com.video.play;

import com.video.data.Value;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.view.KeyEvent;
import android.view.WindowManager;

public class VideoPlayActivity  extends Activity {

	private static VideoPlayView view = null; //视频对象
//	private AudioPlayThread audioThread = null; //音频对象
	private WakeLock wakeLock = null; //锁屏对象
	private static String dealerName = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		initData();
		
//		view = new VideoPlayView(this);
//		setContentView(view);
//		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
//		audioThread = new AudioPlayThread();
//		audioThread.start();
		
//		PowerManager pm = (PowerManager)getSystemService(POWER_SERVICE);
//		wakeLock = pm.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP|PowerManager.FULL_WAKE_LOCK, "WakeLock");
//		wakeLock.acquire(); //设置屏幕保持唤醒
	}
	
	private void initData() {
		Intent intent = this.getIntent();
		dealerName = (String) intent.getCharSequenceExtra("dealerName");
		Value.TerminalDealerName = dealerName;
		System.out.println("MyDebug: 列表的dealerName:"+dealerName);
		//【打开通道】
		TunnelCommunication.getInstance().tunnelInitialize("com/video/play/TunnelCommunication");
		TunnelCommunication.getInstance().openTunnel(dealerName);
		System.out.println("MyDebug: 【打开通道】");
	}
	
	public static Handler playHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			if (msg.what == 0) {
				String peerId = (String) msg.obj;
				System.out.println("MyDebug: 回调的dealerName:"+peerId);
				if (peerId.equals(dealerName)) {
					//【播放视频】
					TunnelCommunication.getInstance().askMediaData(dealerName);
					System.out.println("MyDebug: 【播放视频】");
					view.playVideo();
				}
			}
		}
	};
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			//【关闭通道】
			TunnelCommunication.getInstance().closeTunnel(dealerName);
//			TunnelCommunication.getInstance().tunnelTerminate();
			Value.TerminalDealerName = null;
			System.out.println("MyDebug: 【关闭通道】");
			finish();
		}
		return super.onKeyDown(keyCode, event);
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		view.stopVideo();
//		audioThread.stopAudioPlay();
//		wakeLock.release(); //解除屏幕保持唤醒
//		wakeLock = null;
	}
}
