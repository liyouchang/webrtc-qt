package com.video.utils;

import android.app.Activity;
import android.view.Window;

import com.video.R;

public class TitleSet {
	
	private static Activity mActivity; 
	
	public static void getOwnTitleBar(Activity activity) {  
        mActivity = activity;  
        mActivity.requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);  
        mActivity.setContentView(R.layout.titlebar_own_layout);  
        mActivity.getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.titlebar_own_layout);
    }  
}
