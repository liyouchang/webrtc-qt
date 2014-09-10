package com.video.main;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.play.TunnelCommunication;
import com.video.service.MainApplication;
import com.video.socket.ZmqCtrl;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.user.LoginActivity;
import com.video.utils.OkCancelDialog;
import com.video.utils.UpdateAPK;
import com.video.utils.Utils;

@SuppressLint({ "CutPasteId", "HandlerLeak" })
public class MainActivity extends FragmentActivity implements OnClickListener {
	
	public static Context mContext;
	private PreferData preferData = null;
	
	private static List<Fragment> mFragmentList = new ArrayList<Fragment>();
    private FragmentManager mFragmentManager;  
    private FragmentTransaction mFragmentTransaction; 
    public static int lastFragment = 0;
    
    public static final int TAB_ONE = 0;
    public static final int TAB_TWO = 1;
    public static final int TAB_THREE = 2;
    public static final int TAB_FOUR = 3;
    
    private static TextView tab_three_number;
    private RelativeLayout tab_one_layout;
    private RelativeLayout tab_two_layout;
    private RelativeLayout tab_three_layout;
    private RelativeLayout tab_four_layout;
	
	private String userName = "";
	private String userPwd = "";
	private boolean isAppFirstTime = true;
	private boolean isAutoLogin = false;
	private boolean isActivityShow = false;
	private boolean isStartupThreadRun = false;
	public static boolean isPlayAlarmMusic = true;
	
	private static Dialog mDialog = null;
	private int loginTimes = 0;
	
	private final static int IS_LOGINNING = 1;
	private final static int LOGIN_TIMEOUT = 2;
	private final int LOGIN_AGAIN = 3;
	
	private MainReceiver mainReceiver = null;
	public static final String UNREAD_ALARM_COUNT_ACTION = "MainActivity.unread_alarm_count_action";
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = MainActivity.this;
        preferData = new PreferData(mContext);
        ZmqCtrl.getInstance().init();
        
        // 注册广播
        mainReceiver = new MainReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(UNREAD_ALARM_COUNT_ACTION);
		registerReceiver(mainReceiver, filter);
        
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
			initView();
			initData();
		}
    }

	@Override
	protected void onStart() {
		// TODO Auto-generated method stub
		super.onStart();
		isActivityShow = true;
	}

	private void initView() {
		tab_three_number = (TextView) this.findViewById(R.id.tab_three_number);
    	tab_one_layout = (RelativeLayout) this.findViewById(R.id.tab_one_layout);
    	tab_two_layout = (RelativeLayout) this.findViewById(R.id.tab_two_layout);
    	tab_three_layout = (RelativeLayout) this.findViewById(R.id.tab_three_layout);
    	tab_four_layout = (RelativeLayout) this.findViewById(R.id.tab_four_layout);
	}
	
	private void initData() {
		DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        Utils.screenWidth = dm.widthPixels;
        Utils.screenHeight = dm.heightPixels;
        
        if (preferData.isExist("PlayAlarmMusic")) {
			isPlayAlarmMusic = preferData.readBoolean("PlayAlarmMusic");
		}
        mFragmentList.clear();
        mFragmentList.add(new OwnFragment());
        mFragmentList.add(new LocalFragment());
        mFragmentList.add(new MsgFragment());
        mFragmentList.add(new MoreFragment());
        setCurrentFragment(lastFragment);
        showAlarmMsgCount(MainApplication.getInstance().unreadAlarmCount);
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
	
	@Override
	public void onClick(View v) {
		if ((mFragmentList == null) || (mFragmentList.size() != 4) || (!Value.isLoginSuccess)) {
			mFragmentList = new ArrayList<Fragment>();
			mFragmentList.clear();
	        mFragmentList.add(new OwnFragment());
	        mFragmentList.add(new LocalFragment());
	        mFragmentList.add(new MsgFragment());
	        mFragmentList.add(new MoreFragment());
		}
		switch (v.getId()) {
			case R.id.tab_one_layout:
				setCurrentFragment(TAB_ONE);
				break;
			case R.id.tab_two_layout:
				setCurrentFragment(TAB_TWO);
				break;
			case R.id.tab_three_layout:
				setCurrentFragment(TAB_THREE);
				break;
			case R.id.tab_four_layout:
				setCurrentFragment(TAB_FOUR);
				break;
		}
	}
	
	private void setCurrentFragment(int curFragment) {
		tab_one_layout.setEnabled(true);
		tab_two_layout.setEnabled(true);
		tab_three_layout.setEnabled(true);
		tab_four_layout.setEnabled(true);
		
		mFragmentManager = getSupportFragmentManager();
    	mFragmentTransaction = mFragmentManager.beginTransaction();
		if (mFragmentList.get(curFragment).isAdded()) {
			mFragmentTransaction.hide(mFragmentList.get(lastFragment));
			mFragmentTransaction.show(mFragmentList.get(curFragment));
		} else {
			mFragmentTransaction.add(R.id.tab_content, mFragmentList.get(curFragment));
		}
		
		switch (curFragment) {
			case TAB_ONE:
				tab_one_layout.setEnabled(false);
				break;
			case TAB_TWO:
				tab_two_layout.setEnabled(false);
				break;
			case TAB_THREE:
				tab_three_layout.setEnabled(false);
				break;
			case TAB_FOUR:
				tab_four_layout.setEnabled(false);
				break;
		}
		
		mFragmentTransaction.hide(mFragmentList.get(lastFragment));
		mFragmentTransaction.show(mFragmentList.get(curFragment));
		mFragmentTransaction.commit();
		lastFragment = curFragment;
	}
	
	/**
	 * 判断是否在当前的Tab页
	 * @param tabNum 1、2、3、4
	 * @return 是返回true 否返回false
	 */
	public static boolean isCurrentTab(int tabNum) {
		if (tabNum == lastFragment) {
			return true;
		}
		return false;
	}
	
	/**
	 * 设置显示报警消息的数量
	 */
	public static void showAlarmMsgCount(int alarmCount) {
		if (tab_three_number != null) {
			if ((alarmCount > 0) && (alarmCount < 100)) {
				tab_three_number.setVisibility(View.VISIBLE);
				tab_three_number.setText(""+alarmCount);
			} 
			else if (alarmCount >= 100) {
				tab_three_number.setVisibility(View.VISIBLE);
				tab_three_number.setText("99+");
			} else {
				tab_three_number.setVisibility(View.INVISIBLE);
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
					showAlarmMsgCount(msg.arg1);
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
								initView();
								initData();
						        new UpdateAPK(mContext).startCheckUpgadeThread();
						    // 初始化IceServers
							TunnelCommunication.getInstance().tunnelInitialize(MainApplication.getInstance().generateIceServersJson(Value.stun, Value.turn, userName, userPwd));
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
	 * 退出应用
	 */
	public void exitVideoMonitorApp() {
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
			}
		});
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
			super.openOptionsMenu();
			return true;
		} 
		else if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			
		}
		return super.onKeyDown(keyCode, event);
	}
	
	public boolean onCreateOptionsMenu(Menu menu) {
		super.onCreateOptionsMenu(menu);
		menu.add(Menu.NONE, 1, 1, "退出视界通").setIcon(R.drawable.icon_close);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
			case 1:
				exitVideoMonitorApp();
				break;
			default:
				return super.onOptionsItemSelected(item);
		}
		return true;
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
		if (mainReceiver != null) {
			unregisterReceiver(mainReceiver);
		}
		Value.isManulLogout = false;
	}

	public class MainReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			String action = intent.getAction();
			// 更新未读报警消息的显示
			if (action.equals(UNREAD_ALARM_COUNT_ACTION)) {
				showAlarmMsgCount(MainApplication.getInstance().unreadAlarmCount);
				if (preferData == null) {
					preferData = new PreferData(mContext);
				}
				preferData.writeData("AlarmCount", MainApplication.getInstance().unreadAlarmCount);
			}
		}
	}
}

