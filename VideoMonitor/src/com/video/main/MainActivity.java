package com.video.main;

import java.util.ArrayList;

import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TabHost;
import android.widget.TabWidget;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.main.FragmentTabAdapter.OnMyTabChangedListener;
import com.video.service.MainApplication;
import com.video.socket.ZmqCtrl;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.user.LoginActivity;
import com.video.utils.OkCancelDialog;
import com.video.utils.TabFactory;
import com.video.utils.UpdateAPK;
import com.video.utils.Utils;

public class MainActivity extends FragmentActivity {
	
	public ArrayList<Fragment> fragments = new ArrayList<Fragment>();
	
	public static Context mContext;
	private PreferData preferData = null;
	private TabHost mTabHost;
	private TextView app_exit;
	
	private String userName = "";
	private String userPwd = "";
	private boolean isAppFirstTime = true;
	private boolean isAutoLogin = false;
	private boolean isActivityShow = false;
	private boolean isStartupThreadRun = false;
	public static boolean isPlayAlarmMusic = true;
	private boolean isTextViewShow = false;
	private static TextView tv_alarm_msg = null;
	private static String currentTab = "";
	
	private static Dialog mDialog = null;
	private int loginTimes = 0;
	
	private final static int IS_LOGINNING = 1;
	private final static int LOGIN_TIMEOUT = 2;
	private final int LOGIN_AGAIN = 3;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        mContext = MainActivity.this;
        preferData = new PreferData(mContext);
        ZmqCtrl.getInstance().init();
        
        if (!Value.isLoginSuccess) {
        	setFullScreen();
			setContentView(R.layout.startup);
			ZmqHandler.mHandler = handler;
			if (preferData.isExist("AppFirstTime")) {
				isAppFirstTime = preferData.readBoolean("AppFirstTime");
			}
			if (preferData.isExist("AutoLogin")) {
				isAutoLogin = preferData.readBoolean("AutoLogin");
			}
			if (!preferData.isExist("UserName")) {
				isAutoLogin = false;
			}
			if (!preferData.isExist("UserPwd")) {
				isAutoLogin = false;
			}
			new Handler().postDelayed(new Runnable(){   
			    public void run() {
			    	isStartupThreadRun = true;
			    	quitFullScreen();
			    	if (isAppFirstTime) {
			    		// 第一次使用该软件的帮助图片
			    		isAppFirstTime = true;
			    		preferData.writeData("AppFirstTime", isAppFirstTime);
			    		startActivity(new Intent(mContext, HelpActivity.class));
			    		MainActivity.this.finish();
			    	} else {
						if (isAutoLogin) {
							// 自动登录，不进入登录界面
							if (Utils.isNetworkAvailable(mContext)) {
								if (preferData.isExist("UserName")) {
									userName = preferData.readString("UserName");
								}
								if (preferData.isExist("UserPwd")) {
									userPwd = preferData.readString("UserPwd");
								}
								String data = MainApplication.getInstance().generateRealmJson();
								sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
								sendHandlerMsg(IS_LOGINNING);
								sendHandlerMsg(LOGIN_TIMEOUT, Value.REQ_TIME_6S);
							} else {
								// 无可用网络
								Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
								startActivity(new Intent(mContext, LoginActivity.class));
								MainActivity.this.finish();
							}
						} else {
							// 非自动登录
							startActivity(new Intent(mContext, LoginActivity.class));
							MainActivity.this.finish();
						}
			    	}
			    }
			 }, 3000); 
		} else {
			isStartupThreadRun = false;
			//已登录成功
			setContentView(R.layout.main);
			initData();
			initView();
		}
    }

	@Override
	protected void onStart() {
		// TODO Auto-generated method stub
		super.onStart();
		isActivityShow = true;
	}

	/**
	 * 设置全屏
	 */
	private void setFullScreen() {
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
	}
	
	/**
	 * 退出全屏
	 */
	private void quitFullScreen() {
		final WindowManager.LayoutParams attrs = getWindow().getAttributes();
		attrs.flags &= (~WindowManager.LayoutParams.FLAG_FULLSCREEN);
		getWindow().setAttributes(attrs);
		getWindow().clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
	}
	
	private void initData() {
		DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        Utils.screenWidth = dm.widthPixels;
        Utils.screenHeight = dm.heightPixels;
        
        if (preferData.isExist("PlayAlarmMusic")) {
			isPlayAlarmMusic = preferData.readBoolean("PlayAlarmMusic");
		}
        
        app_exit = (TextView) this.findViewById(R.id.tv_exit_application);
        app_exit.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				exitVideoMonitorApp();
			}
		});
	}
	
	private void initView() {
		mTabHost = (TabHost)findViewById(android.R.id.tabhost);
		mTabHost.setup();
		
		TabHost.TabSpec tabSpec = mTabHost.newTabSpec("tab1");
		tabSpec.setIndicator("实时",
				getResources().getDrawable(R.drawable.tab_own_xml))
		.setContent(new TabFactory(mContext));
		mTabHost.addTab(tabSpec);
		
		tabSpec = mTabHost.newTabSpec("tab2");
		tabSpec.setIndicator("历史",
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
		mTabHost.getCurrentTab();
		
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
        if (preferData.isExist("AlarmCount")) {
        	setAlarmIconAndText(preferData.readInt("AlarmCount"));
        } else {
        	setAlarmIconAndText(0);
        }
        
        fragments.add(new OwnFragment());
        fragments.add(new LocalFragment());
        fragments.add(new MsgFragment());
        fragments.add(new MoreFragment());
    
        final FragmentTabAdapter tabAdapter = new FragmentTabAdapter(this, fragments, android.R.id.tabcontent, mTabHost);
        tabAdapter.setOnMyTabChangedListener(new OnMyTabChangedListener() {
			@Override
			public void MyTabChanged(FragmentTabAdapter adapter) {
				// TODO Auto-generated method stub
				updateTab(mTabHost);
			}
		});
	}
	
	@Override
	protected void onSaveInstanceState(Bundle outState) {
		// TODO Auto-generated method stub
		super.onSaveInstanceState(outState);
	}

	private void updateTab(final TabHost _TabHost) {
		
		currentTab = mTabHost.getCurrentTabTag();
		
		int msgCount = 0;
		if (preferData.isExist("AlarmCount")) {
			msgCount = preferData.readInt("AlarmCount");
		} else {
			msgCount = 0;
		}
        
		for (int i = 0; i < _TabHost.getTabWidget().getChildCount(); i++) {
			TextView tv = (TextView) _TabHost.getTabWidget().getChildAt(i).findViewById(android.R.id.title);
    		if (_TabHost.getCurrentTab() == i) {
    			if (i == 2) {
    				if (msgCount > 0) {
    	        		tv.setTextColor(mContext.getResources().getColorStateList(R.color.red));
    	        	} else {
        				tv.setTextColor(this.getResources().getColorStateList(R.color.tab_text_color));
        			}
    			} else {
    				tv.setTextColor(this.getResources().getColorStateList(R.color.tab_text_color));
    			}
			} else {
				if (i == 2) {
    				if (msgCount > 0) {
    	        		tv.setTextColor(mContext.getResources().getColorStateList(R.color.red));
    	        	} else {
    	        		tv.setTextColor(this.getResources().getColorStateList(R.color.white));
        			}
    			} else {
    				tv.setTextColor(this.getResources().getColorStateList(R.color.white));
    			}
			}
			if (i ==2) {
				tv_alarm_msg = (TextView) _TabHost.getTabWidget().getChildAt(i).findViewById(android.R.id.title);
			}
		}
	}
	
	/**
	 * 判断是否在当前的Tab页
	 * @param tabNum 1、2、3、4
	 * @return 是返回true 否返回false
	 */
	public static boolean isCurrentTab(int tabNum) {
		if (currentTab.equals("tab"+tabNum)) {
			return true;
		}
		return false;
	}
	
	/**
	 * 设置消息tab的报警显示
	 * @param msg 多少条消息
	 */
	public static void setAlarmIconAndText(int msg) {
		if (tv_alarm_msg != null) {
			if (msg <= 0) {
				tv_alarm_msg.setText("消息");
				if (isCurrentTab(3)) {
					tv_alarm_msg.setTextColor(mContext.getResources().getColorStateList(R.color.tab_text_color));
				} else {
					tv_alarm_msg.setTextColor(mContext.getResources().getColorStateList(R.color.white));
				}
			} else {
				if (msg < 100) {
					tv_alarm_msg.setText("消息("+msg+")");
					tv_alarm_msg.setTextColor(mContext.getResources().getColorStateList(R.color.red));
				} else {
					tv_alarm_msg.setText("消息99+");
					tv_alarm_msg.setTextColor(mContext.getResources().getColorStateList(R.color.red));
				}
			}
		}
	}
	
	public static Handler mainHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case 0:
					setAlarmIconAndText(msg.arg1);
					
					try {
						//播放报警语音
						if (!Value.isPlayMp3 && isPlayAlarmMusic) {
							Value.isPlayMp3 = true;
							MediaPlayer mediaPlayer = null;
							if (mediaPlayer == null) {
								mediaPlayer = MediaPlayer.create(MainApplication.getInstance(), R.raw.alarm);
								mediaPlayer.stop();
							}
							mediaPlayer.setOnCompletionListener(new OnCompletionListener() {
								@Override
								public void onCompletion(MediaPlayer mp) {
									mp.release();
									mp = null;
									Value.isPlayMp3 = false;
								}
							});
							try {
								mediaPlayer.prepare();
								mediaPlayer.start();
							} catch (Exception e) {
								e.printStackTrace();
							}
						}
					} catch (Exception e) {
						System.out.println("MyDebug: MediaPlayer.create()异常！");
						e.printStackTrace();
					}
					break;
			}
		}
	};
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				//正在登录
				case IS_LOGINNING:
					if (isActivityShow) {
						mDialog = Utils.createLoadingDialog(mContext, "正在登录...");
						mDialog.show();
					}
					break;
				//登录超时
				case LOGIN_TIMEOUT:
					loginTimes ++;
					if (handler.hasMessages(LOGIN_TIMEOUT)) {
						handler.removeMessages(LOGIN_TIMEOUT);
					}
					// 终止主程序和服务广播
					MainApplication.getInstance().stopActivityandService();
					sendHandlerMsg(LOGIN_AGAIN, 3000);
					break;
				//重新登录
				case LOGIN_AGAIN:
					if (loginTimes > 1) {
						loginTimes = 0;
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						Toast.makeText(mContext, "登录超时，请重试！", Toast.LENGTH_SHORT).show();
						startActivity(new Intent(mContext, LoginActivity.class));
		    			MainActivity.this.finish();
					} else {
						ZmqCtrl.getInstance().init();
						String data = MainApplication.getInstance().generateLoginJson(userName, userPwd);
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
						sendHandlerMsg(LOGIN_TIMEOUT, Value.REQ_TIME_6S);
					}
					break;
				// 获得realm
				case R.id.get_realm_id:
					if (msg.arg1 == 0) {
						String data = MainApplication.getInstance().generateLoginJson(userName, userPwd);
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
						System.out.println("MyDebug: 自动登录操作：获得realm成功！");
					} else {
						System.out.println("MyDebug: 自动登录操作：获得realm失败！");
						String data = MainApplication.getInstance().generateRealmJson();
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
					}
					break;
				//接收登录响应
				case R.id.login_id:
					if (handler.hasMessages(LOGIN_TIMEOUT)) {
						handler.removeMessages(LOGIN_TIMEOUT);
						if (isStartupThreadRun) {
							if ((mDialog != null) && (mDialog.isShowing())) {
								mDialog.dismiss();
								mDialog = null;
							}
							if (msg.arg1 == 0) {
								Value.isLoginSuccess = true;
								MainApplication.getInstance().userName = userName;
								MainApplication.getInstance().userPwd = userPwd;
								setContentView(R.layout.main);
						        initData();
						        initView();
						        new UpdateAPK(mContext).startCheckUpgadeThread();
							} else {
								Toast.makeText(mContext, "登录失败，"+Utils.getErrorReason(msg.arg1), Toast.LENGTH_SHORT).show();
								startActivity(new Intent(mContext, LoginActivity.class));
				    			MainActivity.this.finish();
							}
						}
					} else {
						handler.removeMessages(R.id.login_id);
					}
					break;
			}
		}
	};	
	
	/**
	 * 发送Handler消息
	 */
	private void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	private void sendHandlerMsg(int what, int timeout) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessageDelayed(msg, timeout);
	}
	private void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	/**
	 * 退出微视界
	 */
	private void exitVideoMonitorApp() {
		final OkCancelDialog myDialog=new OkCancelDialog(mContext);
		myDialog.setTitle("温馨提示");
		myDialog.setMessage("确认退出视界通？");
		myDialog.setPositiveButton("确认", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
				MainApplication.getInstance().stopActivityandService();
				finish();
			}
		});
		myDialog.setNegativeButton("取消", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
				hideExitView();
			}
		});
	}
	
	private void showExitView() {
		isTextViewShow = true;
		app_exit.setVisibility(View.VISIBLE);
		Animation animation = AnimationUtils.loadAnimation(mContext, R.anim.popupwindow_menu_in);  
		app_exit.startAnimation(animation);
	}
	
	private void hideExitView() {
		isTextViewShow = false;
		app_exit.setVisibility(View.INVISIBLE);
		Animation animation = AnimationUtils.loadAnimation(mContext, R.anim.popupwindow_menu_out);  
		app_exit.startAnimation(animation);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if ((mDialog != null) && (mDialog.isShowing())) {
			mDialog.dismiss();
			mDialog = null;
		}
		if (handler.hasMessages(LOGIN_TIMEOUT)) {
			handler.removeMessages(LOGIN_TIMEOUT);
		}
		if (handler.hasMessages(R.id.login_id)) {
			handler.removeMessages(R.id.login_id);
		}
		if (keyCode == KeyEvent.KEYCODE_MENU  && event.getRepeatCount() == 0) {
			if (isTextViewShow == false) {
				showExitView();
			} else {
				hideExitView();
			}
		} else if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			if ((!isStartupThreadRun) && (!Value.isLoginSuccess)) {
				return false;
			}
			else if (isTextViewShow) {
				hideExitView();
				return false;
			}
		}
		return super.onKeyDown(keyCode, event);
	}
	
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		if ((mDialog != null) && (mDialog.isShowing())) {
			mDialog.dismiss();
			mDialog = null;
		}
		isActivityShow = false;
		isStartupThreadRun = false;
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		Value.isManulLogout = false;
	}
}

