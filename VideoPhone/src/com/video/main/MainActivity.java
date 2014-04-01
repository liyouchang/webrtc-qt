package com.video.main;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.ViewPager.LayoutParams;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.TabHost;
import android.widget.TabWidget;
import android.widget.TextView;

import com.video.R;
import com.video.service.BackstageService;
import com.video.socket.HandlerApplication;
import com.video.socket.ZmqCtrl;
import com.video.utils.PopupWindowAdapter;
import com.video.utils.TabFactory;
import com.video.utils.Utils;

public class MainActivity extends FragmentActivity {
	
	private Context mContext;
	private TabHost mTabHost;
	private PopupWindow mPopupWindow;
	private boolean isOpenPopupWindow = false;
	
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
		Intent intent = new Intent(HandlerApplication.getInstance(), BackstageService.class);
    	HandlerApplication.getInstance().stopService(intent);
    	System.out.println("MyDebug: 【退出应用软件】");
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			
		} else if (keyCode == KeyEvent.KEYCODE_MENU) {
			if (isOpenPopupWindow == false) {
				isOpenPopupWindow = true;
				showPopupWindow(mTabHost);
			} else {
				isOpenPopupWindow = false;
				if (mPopupWindow.isShowing()) {
					mPopupWindow.dismiss();
				}
			}
		}
		return super.onKeyDown(keyCode, event);
	}	
	
	private void showPopupWindow(View view) {
		LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View pop_view = inflater.inflate(R.layout.pop_exit_main, null);
		ListView pop_listView = (ListView)pop_view.findViewById(R.id.pop_list);
		
		List<String> item_list = new ArrayList<String>();
		item_list.add("退出应用");
		PopupWindowAdapter popAdapter = new PopupWindowAdapter(mContext, item_list);
		pop_listView.setAdapter(popAdapter);
		
		mPopupWindow = new PopupWindow(pop_view, LayoutParams.FILL_PARENT, 190, true);
		mPopupWindow.setHeight(LayoutParams.WRAP_CONTENT); 
		mPopupWindow.setBackgroundDrawable(new BitmapDrawable());
		mPopupWindow.setOutsideTouchable(true);
		
		mPopupWindow.setAnimationStyle(R.style.PopupAnimationBottom);
		mPopupWindow.showAtLocation(view, Gravity.BOTTOM, 0, 0);
		mPopupWindow.update();

		pop_listView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				switch (position) {
					case 0:
						if (mPopupWindow.isShowing()) {
							mPopupWindow.dismiss();
						}
						Intent intent = new Intent(HandlerApplication.getInstance(), BackstageService.class);
				    	HandlerApplication.getInstance().stopService(intent);
						break;
					default : break;
				}
			}
		});
	}
}

