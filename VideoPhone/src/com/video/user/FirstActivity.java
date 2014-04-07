package com.video.user;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;

import com.video.R;
import com.video.data.PreferData;
import com.video.main.HelpActivity;
import com.video.socket.ZmqCtrl;

public class FirstActivity extends Activity {

	private Context mContext;
	private PreferData preferData = null;
	
	private boolean appFirstTime = true;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.first);
		
		mContext = FirstActivity.this;
        ZmqCtrl.getInstance().init();
		preferData = new PreferData(mContext);
		System.out.println("MyDebug: 【进入应用软件】");
		
		if (preferData.isExist("AppFirstTime")) {
			appFirstTime = preferData.readBoolean("AppFirstTime");
		}
		
		new Handler().postDelayed(new Runnable(){   
		    public void run() {   
		    	if (appFirstTime) {
		    		appFirstTime = true;
		    		preferData.writeData("AppFirstTime", appFirstTime);
		    		startActivity(new Intent(mContext, HelpActivity.class));
		    	} else {
		    		startActivity(new Intent(mContext, LoginActivity.class));
		    	}
		    	FirstActivity.this.finish();
		    } 
		 }, 1000); 
	}
}
