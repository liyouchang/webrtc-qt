package com.video.main;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TabHost;
import android.widget.TabWidget;
import android.widget.TextView;

import com.video.R;
import com.video.service.BackstageService;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqCtrl;
import com.video.utils.TabFactory;
import com.video.utils.Utils;

public class MainActivity extends FragmentActivity {
	
	private Context mContext;
	private TabHost mTabHost;
	private TextView app_exit;
	private boolean isTextViewShow = false;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        initData();
        initView();
    }
	
	private void initData() {
		mContext = MainActivity.this;
		DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        Utils.screenWidth = dm.widthPixels;
        Utils.screenHeight = dm.heightPixels;
        
        System.out.println("MyDebug: 【进入应用软件】");
        ZmqCtrl.getInstance().init();
        
        app_exit = (TextView) this.findViewById(R.id.tv_exit_application);
        app_exit.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Intent intent = new Intent(HandlerApplication.getInstance(), BackstageService.class);
		    	HandlerApplication.getInstance().stopService(intent);
		    	System.out.println("MyDebug: 【退出应用软件】");
		    	finish();
			}
		});
	}
	
	private void initView() {
		mTabHost = (TabHost)findViewById(android.R.id.tabhost);
		mTabHost.setup();
		mTabHost.setOnTabChangedListener(tabChangeListener);
		
		TabHost.TabSpec tabSpec = mTabHost.newTabSpec("tab1");
		tabSpec.setIndicator("拥有",
				getResources().getDrawable(R.drawable.tab_own_xml))
		.setContent(new TabFactory(mContext));
		mTabHost.addTab(tabSpec);
		
		tabSpec = mTabHost.newTabSpec("tab2");
		tabSpec.setIndicator("本地",
				getResources().getDrawable(R.drawable.tab_video_xml));
		tabSpec.setContent(new TabFactory(mContext));
		mTabHost.addTab(tabSpec);
		
		tabSpec = mTabHost.newTabSpec("tab3");
		tabSpec.setIndicator("消息",
				getResources().getDrawable(R.drawable.tab_msg_xml));
		tabSpec.setContent(new TabFactory(mContext));
		mTabHost.addTab(tabSpec);
		
		tabSpec = mTabHost.newTabSpec("tab4");
		tabSpec.setIndicator("更多",
				getResources().getDrawable(R.drawable.tab_more_xml));
		tabSpec.setContent(new TabFactory(mContext));
		mTabHost.addTab(tabSpec);
		
		mTabHost.setCurrentTab(0);
		
		TabWidget tabWidget = mTabHost.getTabWidget();  
		tabWidget.setStripEnabled(false);
        for (int i = 0; i < tabWidget.getChildCount(); i++) { 
        	tabWidget.getChildAt(i).setBackgroundDrawable(getResources().getDrawable(R.color.title_bg_black));
            TextView tv = (TextView) tabWidget.getChildAt(i).findViewById(android.R.id.title);  
            ImageView iv = (ImageView) tabWidget.getChildAt(i).findViewById(android.R.id.icon);  
  
            RelativeLayout.LayoutParams paramsImage = new RelativeLayout.LayoutParams(  
                    RelativeLayout.LayoutParams.WRAP_CONTENT,  
                    RelativeLayout.LayoutParams.WRAP_CONTENT);  
            paramsImage.addRule(RelativeLayout.CENTER_HORIZONTAL);  
            paramsImage.addRule(RelativeLayout.ALIGN_PARENT_TOP, RelativeLayout.TRUE);  
            iv.setLayoutParams(paramsImage);  
              
            RelativeLayout.LayoutParams paramsText = new RelativeLayout.LayoutParams(  
                    RelativeLayout.LayoutParams.WRAP_CONTENT,  
                    RelativeLayout.LayoutParams.WRAP_CONTENT);  
            paramsText.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);  
            paramsText.addRule(RelativeLayout.CENTER_HORIZONTAL, RelativeLayout.TRUE);  
            tv.setLayoutParams(paramsText);  
            tv.setTextColor(Color.WHITE); 
            tv.setTextSize(16);
        }  
        updateTab(mTabHost);
	}
	
	private TabHost.OnTabChangeListener tabChangeListener = new TabHost.OnTabChangeListener() {
		
		@Override
		public void onTabChanged(String tabId) {
			// TODO Auto-generated method stub
			
			FragmentManager fm = getSupportFragmentManager();
			FragmentTransaction ft = fm.beginTransaction();
			
			if (tabId.equals("tab1")) {
				OwnFragment fragment = new OwnFragment();
				ft.replace(android.R.id.tabcontent, fragment);
				ft.commit();
			}
			else if (tabId.equals("tab2")) {
				LocalFragment fragment = new LocalFragment();
				ft.replace(android.R.id.tabcontent, fragment);
				ft.commit();
			}
			else if (tabId.equals("tab3")) {
				MsgFragment fragment = new MsgFragment();
				ft.replace(android.R.id.tabcontent, fragment);
				ft.commit();
			}
			else if (tabId.equals("tab4")) {
				MoreFragment fragment = new MoreFragment();
				ft.replace(android.R.id.tabcontent, fragment);
				ft.commit();
			}
			updateTab(mTabHost);
		}
	};
	
	private void updateTab(final TabHost _TabHost) {
		
		for (int i = 0; i < _TabHost.getTabWidget().getChildCount(); i++) {
			TextView tv = (TextView) _TabHost.getTabWidget().getChildAt(i).findViewById(android.R.id.title);
			if (_TabHost.getCurrentTab() == i) {				
				tv.setTextColor(this.getResources().getColorStateList(R.color.tab_text_color));
			} else {
				tv.setTextColor(this.getResources().getColorStateList(R.color.white));
			}
		}
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}	

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_MENU  && event.getRepeatCount() == 0) {
			if (isTextViewShow == false) {
				isTextViewShow = true;
				app_exit.setVisibility(View.VISIBLE);
				Animation animation = AnimationUtils.loadAnimation(mContext, R.anim.popupwindow_menu_in);  
				app_exit.startAnimation(animation); 
			} else {
				isTextViewShow = false;
				app_exit.setVisibility(View.INVISIBLE);
				Animation animation = AnimationUtils.loadAnimation(mContext, R.anim.popupwindow_menu_out);  
				app_exit.startAnimation(animation); 
			}
		} else if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			if (isTextViewShow) {
				isTextViewShow = false;
				app_exit.setVisibility(View.INVISIBLE);
				Animation animation = AnimationUtils.loadAnimation(mContext, R.anim.popupwindow_menu_out);  
				app_exit.startAnimation(animation); 
				return true;
			}
		}
		return super.onKeyDown(keyCode, event);
	}
}

