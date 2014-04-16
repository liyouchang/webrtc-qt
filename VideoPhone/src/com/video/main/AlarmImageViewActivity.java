package com.video.main;

import java.io.File;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.ImageView;

import com.video.R;

public class AlarmImageViewActivity extends Activity implements OnClickListener {
	
	private ImageView view;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.alarm_imageview);
		
		initView();
		initData();
	}
	
	private void initView() {
		view = (ImageView) findViewById(R.id.alarm_imageView);
		view.setOnClickListener(this);
		ImageButton button_back = (ImageButton) this.findViewById(R.id.ib_alarm_back);
		button_back.setOnClickListener(this);
	}
	
	private void initData() {
		Intent intent = this.getIntent();
		File file = new File((String) intent.getCharSequenceExtra("imagePath"));
		System.out.println("MyDebug: Í¼Æ¬µÄÂ·¾¶£º"+intent.getCharSequenceExtra("imagePath"));
		Uri UriPath = Uri.fromFile(file);
		view.setImageURI(UriPath);
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.ib_alarm_back:
				finish();
				break;
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
		}
		return false;
	}
}