package com.video.main;

import com.video.R;
import com.video.data.PreferData;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;

public class SettingsActivity extends Activity implements OnClickListener  {

	private Context mContext;
	private PreferData preferData = null;
	private ImageButton playMusic;
	
	private boolean isPlayAlarmMusic = true;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.settings);
		
		initView();
		initData();
	}
	
	private void initView() {
		ImageButton back = (ImageButton) this.findViewById(R.id.ib_settings_back);
		back.setOnClickListener(this);
	
		playMusic = (ImageButton) this.findViewById(R.id.ib_play_music_switch);
		playMusic.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = SettingsActivity.this;
		preferData = new PreferData(mContext);
		if (preferData.isExist("PlayAlarmMusic")) {
			isPlayAlarmMusic = preferData.readBoolean("PlayAlarmMusic");
		}
		if (isPlayAlarmMusic) {
			playMusic.setBackgroundResource(R.drawable.icon_set_on);
		} else {
			playMusic.setBackgroundResource(R.drawable.icon_set_off);
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.ib_settings_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
				break;
			case R.id.ib_play_music_switch:
				if (isPlayAlarmMusic) {
					isPlayAlarmMusic = false;
					preferData.writeData("PlayAlarmMusic", isPlayAlarmMusic);
					playMusic.setBackgroundResource(R.drawable.icon_set_off);
				} else {
					isPlayAlarmMusic = true;
					preferData.writeData("PlayAlarmMusic", isPlayAlarmMusic);
					playMusic.setBackgroundResource(R.drawable.icon_set_on);
				}
				break;
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
		}
		return super.onKeyDown(keyCode, event);
	}
}
