package com.video.socket;

import android.app.Application;
import android.os.Handler;

public class HandlerApplication extends Application {

	private static HandlerApplication AppInstance;
	private Handler mHandler;
	

	public void setMyHandler(Handler mHandler) {
		this.mHandler = mHandler;
	}

	public Handler getMyHandler() {
		return mHandler;
	}
	
	public static HandlerApplication getInstance() {  
        return AppInstance;  
    } 
	
	@Override  
    public void onCreate() {  
        // TODO Auto-generated method stub  
        super.onCreate();  
        AppInstance = this;
    }
}
