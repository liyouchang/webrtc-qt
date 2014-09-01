package com.video.play;

import java.util.HashMap;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.MessageQueue.IdleHandler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.support.v4.view.ViewPager.LayoutParams;
import android.view.Display;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;
import com.video.service.BackstageService;

@SuppressLint({ "Wakelock", "HandlerLeak" })
public class RemoteFilePlayerActivity  extends Activity implements OnClickListener  {

	private static Context mContext;
	
	private static VideoView videoView = null; //视频对象
//	private static AudioThread audioThread = null; //音频对象
	private WakeLock wakeLock = null; //锁屏对象
	
	public static int requestPlayerTimes = 0;
	private PlayerReceiver playerReceiver; 
	public static final String REQUEST_REMOTE_FILE_PLAYER_ACTION = "RemoteFilePlayerActivity.requestRemoteFilePlayerStatus";

	private FileInfo fileInfo = null;
	private int jniCallbackParam = -1;
	private static String dealerName = null;
	private static Dialog mDialog = null;
	
	private int screenWidth = 0;
	private int screenHeight = 0;
	private int bottomHeight = 100;
	
	private boolean isPopupWindowShow = false;
	private final int SHOW_TIME_MS = 10000;
	private final int HIDE_POPUPWINDOW = 1;
	private final int REQUEST_TIME_OUT = 2;
	private final int SEEKBAR_REFRESH = 3;
	private final int SEEKBAR_FILE_SIZE = 4;
	private final int READ_FILE_OVER = 5;
	
	private GestureDetector mGestureDetector = null; //手势识别
	private int dragSeekBarProgress = 0; // 当前播放进度
	
	private View controlerView = null; //底部视图
	private static PopupWindow controlerPopupWindow = null; //底部弹出框
	private View infoView = null; // 播放信息视图
	private static PopupWindow infoPopupWindow = null; // 播放信息弹出框

	private SeekBar seekBar = null; // 可拖拽的进度条
	private TextView tv_info = null; // 播放信息
	private TextView tv_download = null; // 下载大小
	private TextView durationTextView = null; // 视频的总时间
	private TextView playedTextView = null; // 播放时间
	private ImageButton button_front = null; // 上一个
	private ImageButton button_slow = null; // 慢进
	private ImageButton button_play_pause = null; // 播放、暂停
	private ImageButton button_fast = null; // 快进
	private ImageButton button_back = null; // 下一个
	
	public class FileInfo {
		public String fileName = "Sample.avi"; // 文件名
		public long fileSize = 100; // 文件大小
		public int playSpeed = 0; // 播放速度
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		PowerManager pm = (PowerManager)getSystemService(POWER_SERVICE);
		wakeLock = pm.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP|PowerManager.FULL_WAKE_LOCK, "WakeLock");
		wakeLock.acquire(); // 设置屏幕保持唤醒
		
		// 视频
		videoView = new VideoView(this);
		setContentView(videoView);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		// 音频
//		audioThread = new AudioThread();
		
		initData(); // 初始化数据
		initView(); // 初始化视图
		
		// 空闲的队列
		Looper.myQueue().addIdleHandler(new IdleHandler() {
			@Override
			public boolean queueIdle() {
				isPopupWindowShow = true;
				if (controlerPopupWindow != null && videoView.isShown()) {
					showBottomPopupWindow();
					showRecordPopupWindow();
				}
				hidePopupWindowDelay();
				return false;
			}
		});
		
		mGestureDetector = new GestureDetector(new SimpleOnGestureListener(){
			//单击屏幕
			@Override
			public boolean onSingleTapUp(MotionEvent e) {
				// TODO Auto-generated method stub
				if (isPopupWindowShow) {
					hidePopupWindow();
				} else {
					showPopupWindow();
					hidePopupWindowDelay();
				}
				return false;
			}
		});
	}
	
	private void initView() {
		// 视频底部弹出框
		controlerView = getLayoutInflater().inflate(R.layout.terminal_player_controler, null);
		controlerPopupWindow = new PopupWindow(controlerView, screenWidth, bottomHeight, false);
		
		seekBar = (SeekBar) controlerView.findViewById(R.id.seekbar);
		seekBar.setOnSeekBarChangeListener(new onSeekBarChangeListenerImpl());
		playedTextView = (TextView) controlerView.findViewById(R.id.has_played);
		durationTextView = (TextView) controlerView.findViewById(R.id.duration);
		
		button_front = (ImageButton) controlerView.findViewById(R.id.ib_front_file);
		button_slow = (ImageButton) controlerView.findViewById(R.id.ib_play_slow);
		button_play_pause = (ImageButton) controlerView.findViewById(R.id.ib_play_pause);
		button_fast = (ImageButton) controlerView.findViewById(R.id.ib_play_fast);
		button_back = (ImageButton) controlerView.findViewById(R.id.ib_back_file);
		
		button_front.setOnClickListener(this);
		button_slow.setOnClickListener(this);
		button_play_pause.setOnClickListener(this);
		button_fast.setOnClickListener(this);
		button_back.setOnClickListener(this);
		
		button_front.setAlpha(0xBB);
		button_slow.setAlpha(0xBB);
		button_play_pause.setAlpha(0xBB);
		button_fast.setAlpha(0xBB);
		button_back.setAlpha(0xBB);
		
		// 录像弹出框
		infoView = getLayoutInflater().inflate(R.layout.terminal_player_info, null);
		infoPopupWindow = new PopupWindow(infoView, screenWidth, bottomHeight, false);
		tv_info = (TextView) infoView.findViewById(R.id.tv_info_content);
		tv_info.setText("正常播放");
		tv_download = (TextView) infoView.findViewById(R.id.tv_info_download);
		
		// 处理显示
		seekBar.setMax((int)fileInfo.fileSize);
		durationTextView.setText("总大小: "+fileInfo.fileSize);
	}
	
	private void initData() {
		mContext = RemoteFilePlayerActivity.this;
		getScreenSize();
		
		//注册广播
		playerReceiver = new PlayerReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(REQUEST_REMOTE_FILE_PLAYER_ACTION);
		filter.addAction(BackstageService.TUNNEL_REQUEST_ACTION);
		registerReceiver(playerReceiver, filter);
		
		//获得终端录像的信息		
		Intent intent = this.getIntent();
		if (intent != null) {
			fileInfo = new FileInfo();
			dealerName = (String) intent.getCharSequenceExtra("dealerName");
			//【请求远程录像】
			TunnelCommunication.getInstance().playRemoteFile(dealerName, fileInfo.fileName);
			
			if ((mDialog == null) || (!mDialog.isShowing())) {
				mDialog = createLoadingDialog("正在请求远程录像...");
				mDialog.show();
			}
			sendHandlerMsg(REQUEST_TIME_OUT, 30000); // 30S
		}
	}
	
	private class onSeekBarChangeListenerImpl implements OnSeekBarChangeListener {
		@Override
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			// TODO Auto-generated method stub
			if (fromUser) {
				dragSeekBarProgress = progress;
				videoView.isPlayVideo = false;
			}
		}
		
		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
			// TODO Auto-generated method stub
			videoView.isPlayVideo = false;
		}
		
		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
			// TODO Auto-generated method stub
			System.out.println("MyDebug: 【拖拽指针位置】"+dragSeekBarProgress);
			if (TunnelCommunication.videoDataCache != null) {
				TunnelCommunication.videoDataCache.clearBuffer();
			}
			if (TunnelCommunication.audioDataCache != null) {
				TunnelCommunication.audioDataCache.clearBuffer();
			}
			playTerminalVideoFile();
			hidePopupWindowDelay();
		}
	}
	
	
	/**
	 * 播放终端录像
	 */
	private void playTerminalVideoFile() {
		videoView.isPlayVideo = true;
		button_play_pause.setImageResource(R.drawable.local_player_pause);
		tv_info.setText(getReadSpeedValue());
	}
	
	/**
	 * 暂停终端录像
	 */
	private void pauseTerminalVideoFile() {
		videoView.isPlayVideo = false;
		button_play_pause.setImageResource(R.drawable.local_player_play);
		tv_info.setText("已暂停");
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			// 前一个文件
			case R.id.ib_front_file:
				break;
			// 慢进
			case R.id.ib_play_slow:
				break;
				// 播放、暂停
			case R.id.ib_play_pause:
				if (videoView.isPlayVideo) {
					pauseTerminalVideoFile();
				} else {
					playTerminalVideoFile();
				}
				break;
			// 快进
			case R.id.ib_play_fast:
				break;
			// 后一个文件
			case R.id.ib_back_file:
				break;
		}
		hidePopupWindowDelay();
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {// 实现该方法来处理触屏事件
		// TODO Auto-generated method stub
		boolean result = mGestureDetector.onTouchEvent(event);
		if (!result) {
			result = super.onTouchEvent(event);
		}
		return result;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			if (mDialog.isShowing()) {
				mDialog.dismiss();
			}
			if (handler.hasMessages(REQUEST_TIME_OUT)) {
				handler.removeMessages(REQUEST_TIME_OUT);
			}
			//关闭通道
			if (Value.isTunnelOpened) {
				TunnelCommunication.getInstance().closeTunnel(dealerName);
			}
		}
		return super.onKeyDown(keyCode, event);
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				// 隐藏弹出框
				case HIDE_POPUPWINDOW:
					hidePopupWindow();
					break;
				// 请求下载录像文件超时
				case REQUEST_TIME_OUT:
					if (mDialog.isShowing()) {
						mDialog.dismiss();
					}
					toastNotify(mContext, "请求终端录像超时，请重试！", Toast.LENGTH_SHORT);
					break;
				// 更新播放进度条
				case SEEKBAR_REFRESH:
					seekBar.setProgress(msg.arg1);
					playedTextView.setText("已播放: "+msg.arg1);
					tv_download.setVisibility(View.INVISIBLE);
					break;
				// 文件大小进度条
				case SEEKBAR_FILE_SIZE:
					seekBar.setMax(msg.arg1);
					durationTextView.setText("总大小: "+msg.arg1);
					break;
				// 读取文件结束
				case READ_FILE_OVER:
					pauseTerminalVideoFile();
					playedTextView.setText("已播放: "+fileInfo.fileSize);
					break;
			}
		}
	};
	
	/**
	 * 发送Handler消息
	 */
	public void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	private void sendHandlerMsg(int what, int timeout) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessageDelayed(msg, timeout);
	}
	public void sendHandlerMsg(Handler handler, int what, int arg1) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, int arg1, int arg2) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(Handler handler, int what, HashMap<String, String> obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	/**
	 * 显示PopupWindow
	 */
	private void showPopupWindow() {
		isPopupWindowShow = true;
		cancelDelayHide();
		showBottomPopupWindow();
	}
	
	/**
	 * 显示底部PopupWindow
	 */
	private void showBottomPopupWindow() {
		controlerPopupWindow.setHeight(bottomHeight); 
		controlerPopupWindow.setBackgroundDrawable(new BitmapDrawable());
		controlerPopupWindow.setOutsideTouchable(true);
		
		controlerPopupWindow.setAnimationStyle(R.style.PopupAnimationBottom);
		controlerPopupWindow.showAtLocation(videoView, Gravity.BOTTOM, 0, 0);
		controlerPopupWindow.update();
		controlerPopupWindow.showAtLocation(videoView, Gravity.BOTTOM, 0, 0);
	}
	
	/**
	 * 显示播放信息PopupWindow
	 */
	private void showRecordPopupWindow() {
		if (infoPopupWindow != null && videoView.isShown()) {
			infoPopupWindow.setWidth(LayoutParams.WRAP_CONTENT);
			infoPopupWindow.setHeight(LayoutParams.WRAP_CONTENT); 
			infoPopupWindow.setBackgroundDrawable(new BitmapDrawable());
			infoPopupWindow.setOutsideTouchable(false);
			infoPopupWindow.setTouchable(true);
			
			infoPopupWindow.showAtLocation(videoView, Gravity.TOP | Gravity.RIGHT, 60, 60);
			infoPopupWindow.update();
		}
	}
	
	/**
	 * 隐藏PopupWindow
	 */
	private void hidePopupWindow() {
		isPopupWindowShow = false;
		cancelDelayHide();
		if (controlerPopupWindow.isShowing()) {
			controlerPopupWindow.dismiss();
		}
	}
	
	/**
	 * 延迟隐藏控制器
	 */
	private void hidePopupWindowDelay() {
		cancelDelayHide();
		handler.sendEmptyMessageDelayed(HIDE_POPUPWINDOW, SHOW_TIME_MS);
	}
	
	/**
	 * 取消延迟隐藏
	 */
	private void cancelDelayHide() {
		if (handler.hasMessages(HIDE_POPUPWINDOW)) {
			handler.removeMessages(HIDE_POPUPWINDOW);
		}
	}
	
	/**
	 * 获得屏幕尺寸大小
	 */
	private void getScreenSize() {
		Display display = getWindowManager().getDefaultDisplay();
		screenWidth = display.getWidth();
		screenHeight = display.getHeight();
		if (screenHeight > screenWidth) {
			screenWidth = screenHeight;
		}
		bottomHeight = screenHeight / 4;
	}
	
	/**
	 * 自定义Toast显示
	 */
	private static void toastNotify(Context context, String notify_text, int duration) {	
		LayoutInflater Inflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View tx_view = Inflater.inflate(R.layout.toast_layout, null);
		
		TextView textView = (TextView)tx_view.findViewById(R.id.toast_text_id);
		textView.setText(notify_text);
		
		Toast toast = new Toast(context);
		toast.setDuration(duration);
		toast.setView(tx_view);
		toast.show();
	}
	
	/**
	 * 自定义Dialog
	 */
	public Dialog createLoadingDialog(String msg) {
		LayoutInflater inflater = LayoutInflater.from(mContext);
		View v = inflater.inflate(R.layout.dialog_player, null);
		LinearLayout layout = (LinearLayout) v.findViewById(R.id.dialog_view);
		ImageView spaceshipImage = (ImageView) v.findViewById(R.id.dialog_img);
		TextView tipTextView = (TextView) v.findViewById(R.id.dialog_textView);
		Animation hyperspaceJumpAnimation = AnimationUtils.loadAnimation(mContext, R.anim.dialog_anim);
		spaceshipImage.startAnimation(hyperspaceJumpAnimation);
		tipTextView.setText(msg);
		Dialog loadingDialog = new Dialog(mContext, R.style.AppThemeFullscreen);
		loadingDialog.setCancelable(true);
		loadingDialog.setCanceledOnTouchOutside(false);
		loadingDialog.setOnCancelListener(new OnCancelListener() {
			@Override
			public void onCancel(DialogInterface dialog) {
				// TODO Auto-generated method stub
				RemoteFilePlayerActivity.this.finish();
			}
		});
		loadingDialog.setContentView(layout, new LinearLayout.LayoutParams(
				LinearLayout.LayoutParams.FILL_PARENT,
				LinearLayout.LayoutParams.FILL_PARENT));
		return loadingDialog;
	}
	
	@Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        getScreenSize();
        if (isPopupWindowShow) {
			showPopupWindow();
			hidePopupWindowDelay();
		} else {
			hidePopupWindow();
		}
	}
	
	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
		closePlayer();
		finish();
	}

	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		destroyDialogView();
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		//注销广播
		unregisterReceiver(playerReceiver);
		//解除屏幕保持唤醒
		if ((wakeLock != null) && (wakeLock.isHeld())) {
			wakeLock.release(); 
			wakeLock = null;
		}
	}

	/**
	 * 销毁弹出框
	 */
	private void destroyDialogView() {
		if (mDialog.isShowing()) {
			mDialog.dismiss();
		}
		if (controlerPopupWindow.isShowing()) {
			controlerPopupWindow.dismiss();
		}
		if (infoPopupWindow.isShowing()) {
			infoPopupWindow.dismiss();
		}
	}

	/**
	 * 关闭播放器
	 */
	private void closePlayer() {
		try {
			destroyDialogView();
			
			//关闭实时音视频
			try {
				videoView.stopVideo();
//				audioThread.stopAudioThread();
			} catch (Exception e) {
				System.out.println("MyDebug: 关闭终端录像音视频对讲异常！");
				e.printStackTrace();
			}
		} catch (Exception e) {
			System.out.println("MyDebug: 关闭终端录像播放器异常！");
			e.printStackTrace();
		}
	}
	
	/**
	 * 获得读文件的速度值的说明
	 */
	private String getReadSpeedValue() {
		String result = "正常播放";
		if (!videoView.isPlayVideo) {
			return result = "已暂停";
		}
		switch (fileInfo.playSpeed) {
			case -3: result = "1/8倍速播放"; break;
			case -2: result = "1/4倍速播放"; break;
			case -1: result = "1/2倍速播放"; break;
			case 0: result = "正常播放"; break;
			case 1: result = "2倍速播放"; break;
			case 2: result = "4倍速播放"; break;
			case 3: result = "8倍速播放"; break;
		}
		return result;
	}
	
	/**
	 * @author sunfusheng
	 * 播放器的广播接收
	 */
	public class PlayerReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			String action = intent.getAction();
			
			if (action.equals(REQUEST_REMOTE_FILE_PLAYER_ACTION)) {
				// status 0：请求录像播放成功  2：请求录像文件错误  3：播放结束  4：正在播放,返回播放位置和播放速度  5：返回错误的消息
				int status = intent.getIntExtra("status", 5);
				switch (status) {
					case 0: // 请求录像播放成功
						jniCallbackParam = 0;
						if ((mDialog != null) && (mDialog.isShowing())) {
							mDialog.dismiss();
							mDialog = null;
						}
						if (videoView == null) {
							videoView = new VideoView(mContext);
						}
						videoView.playVideo();
						break;
					case 1: //
						break;
					case 2: // 请求录像文件错误
						jniCallbackParam = 2;
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						
						break;
					case 3: // 播放结束
						jniCallbackParam = 3;
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						break;
					case 4: // 正在播放,返回播放位置和播放速度
						jniCallbackParam = 4;
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						if (handler.hasMessages(REQUEST_TIME_OUT)) {
							handler.removeMessages(REQUEST_TIME_OUT);
						}
						
						//播放终端录像文件
						playTerminalVideoFile();
						videoView.playVideo();
//						if (audioThread != null) {
//							audioThread.start();
//						}
						break;
					case 5: // 返回错误的消息
						jniCallbackParam = 5;
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						
						break;
				}
			}
			else if (action.equals(BackstageService.TUNNEL_REQUEST_ACTION)) {
				int TunnelEvent = intent.getIntExtra("TunnelEvent", 1);
				switch (TunnelEvent) {
					// 通道打开
					case 0:
						Value.isTunnelOpened = true;
						String peerId = (String) intent.getCharSequenceExtra("PeerId");
						if (peerId.equals(dealerName)) {
						}
						break;
					// 通道关闭
					case 1:
						if ((mDialog != null) && (mDialog.isShowing())) {
							toastNotify(mContext, "请求视频超时，请重试！", Toast.LENGTH_SHORT);
							mDialog.dismiss();
							mDialog = null;
						}
						if (handler.hasMessages(REQUEST_TIME_OUT)) {
							handler.removeMessages(REQUEST_TIME_OUT);
						}
						Value.isTunnelOpened = false;
						break;
				}
			}
		}
	}
	
}
