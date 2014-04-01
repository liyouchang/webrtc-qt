package com.video.main;

import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.ImageButton;

import com.video.R;

public class AboutActivity extends Activity implements OnClickListener, OnTouchListener {

	private ImageButton button_back;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.about);
		
		initView();
	}

	private void initView() {
		button_back = (ImageButton) super.findViewById(R.id.btn_about_back);
		button_back.setOnClickListener(this);
		button_back.setOnTouchListener(this);
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_about_back:
				finish();
				break;
		}
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_about_back:
				if (MotionEvent.ACTION_DOWN == event.getAction()) {
					button_back.setImageResource(R.drawable.btn_title_back_selected);
				} else if (MotionEvent.ACTION_UP == event.getAction()) {
					button_back.setImageResource(R.drawable.btn_title_back_unselected);
				}
				break;
		}
		return false;
	}
}
