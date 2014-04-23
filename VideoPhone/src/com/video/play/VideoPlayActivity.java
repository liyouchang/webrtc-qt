package com.video.play;

import android.app.Activity;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.view.KeyEvent;
import android.view.WindowManager;

public class VideoPlayActivity  extends Activity {

	private VideoPlayView view = null; //视频对象
	private AudioPlayThread audioThread = null; //音频对象
	private WakeLock wakeLock = null; //锁屏对象
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		view = new VideoPlayView(this);
		setContentView(view);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		view.playVideo();
		
		audioThread = new AudioPlayThread();
		audioThread.start();
		
		PowerManager pm = (PowerManager)getSystemService(POWER_SERVICE);
		wakeLock = pm.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP|PowerManager.FULL_WAKE_LOCK, "WakeLock");
		wakeLock.acquire(); //设置屏幕保持唤醒
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
		}
		return super.onKeyDown(keyCode, event);
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		view.stopVideo();
		audioThread.stopAudioPlay();
		wakeLock.release(); //解除屏幕保持唤醒
		wakeLock = null;
	}
}
