package com.video.play;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;

public class VideoPlayActivity  extends Activity {

	private VideoPlayView view = null; //视频图像类
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		view = new VideoPlayView(this);
		setContentView(view);
		view.playVideo();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			view.stopVideo();
		}
		return super.onKeyDown(keyCode, event);
	}
}
