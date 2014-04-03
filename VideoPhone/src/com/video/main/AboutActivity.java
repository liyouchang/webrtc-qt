package com.video.main;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;

import com.video.R;

public class AboutActivity extends Activity implements OnClickListener {

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
}
