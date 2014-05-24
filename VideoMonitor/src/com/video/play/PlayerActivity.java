package com.video.play;

import java.io.File;
import java.util.HashMap;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.MessageQueue.IdleHandler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
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
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;
import com.video.socket.HandlerApplication;
import com.video.utils.Utils;

public class PlayerActivity  extends Activity implements OnClickListener  {

	private static Context mContext;
	
	private static VideoView videoView = null; //视频对象
	private static AudioThread audioThread = null; //音频对象
	private static TalkThread talkThread = null; //对讲对象
	private WakeLock wakeLock = null; //锁屏对象
	
	public static int requestPlayerTimes = 0;
	private PlayerReceiver playerReceiver; 
	public static final String PLAYER_BROADCAST_ACTION = "com.video.play.PlayerActivity.PlayVideo";
	public static final String REQUEST_TIMES_ACTION = "com.video.play.PlayerActivity.RequestTimes";

	private TextView tv_title = null;
	private static String deviceName = null;
	private static String dealerName = null;
	private static Dialog mDialog = null;
	
	private int screenWidth = 0;
	private int screenHeight = 0;
	private int titleHeight = 80;
	private int bottomHeight = 100;
	
	private static boolean isRecordVideo = false;
	private boolean isVoiceEnable = true;
	private boolean isTalkEnable = false;
	private boolean isPlayMusic = false;
	private boolean isPopupWindowShow = false;
	private final int SHOW_TIME_MS = 6000;
	private final int HIDE_POPUPWINDOW = 1;
	
	private GestureDetector mGestureDetector = null;//手势识别
	
	private View titleView = null;//标题视图
	private static PopupWindow titlePopupWindow = null;//标题弹出框
	private View bottomView = null;//底部视图
	private static PopupWindow bottomPopupWindow = null;//底部弹出框

	
	private Button player_capture = null;
	private Button player_record = null;
	private Button player_talkback = null;
	private Button player_sound = null;
	private Button video_clarity = null;
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		PowerManager pm = (PowerManager)getSystemService(POWER_SERVICE);
		wakeLock = pm.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP|PowerManager.FULL_WAKE_LOCK, "WakeLock");
		wakeLock.acquire(); //设置屏幕保持唤醒
		
		initData();
		
		//视频
		videoView = new VideoView(this);
		setContentView(videoView);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		//音频
		audioThread = new AudioThread();
		if (audioThread != null) {
			audioThread.start();
		}
		
		//视频标题弹出框
		titleView = getLayoutInflater().inflate(R.layout.player_title_view, null);
		titlePopupWindow = new PopupWindow(titleView);
		tv_title = (TextView) titleView.findViewById(R.id.tv_player_title);
		tv_title.setText(deviceName);
		ImageButton player_back = (ImageButton) titleView.findViewById(R.id.ib_player_back);
		player_back.setOnClickListener(this);
		
		//视频底部弹出框
		bottomView = getLayoutInflater().inflate(R.layout.player_bottom_view, null);
		bottomPopupWindow = new PopupWindow(bottomView);
		player_capture = (Button) bottomView.findViewById(R.id.btn_player_capture);
		player_capture.setOnClickListener(this);
		player_record = (Button) bottomView.findViewById(R.id.btn_player_record);
		player_record.setOnClickListener(this);
		player_talkback = (Button) bottomView.findViewById(R.id.btn_player_talkback);
		player_talkback.setOnClickListener(this);
		player_sound = (Button) bottomView.findViewById(R.id.btn_player_sound);
		player_sound.setOnClickListener(this);
		video_clarity = (Button) bottomView.findViewById(R.id.btn_player_clarity);
		video_clarity.setOnClickListener(this);
		
		//空闲的队列
		Looper.myQueue().addIdleHandler(new IdleHandler() {
			@Override
			public boolean queueIdle() {
				isPopupWindowShow = true;
				if (titlePopupWindow != null && videoView.isShown()) {
					titlePopupWindow.setAnimationStyle(R.style.PopupAnimationTop);
					titlePopupWindow.showAtLocation(videoView, Gravity.TOP, 0, 0);
					titlePopupWindow.update(0, 0, screenWidth, titleHeight);
				}
				if (bottomPopupWindow != null && videoView.isShown()) {
					bottomPopupWindow.setAnimationStyle(R.style.PopupAnimationBottom);
					bottomPopupWindow.showAtLocation(videoView, Gravity.BOTTOM, 0, 0);
					bottomPopupWindow.update(0, 0, screenWidth, bottomHeight);
				}
				hidePopupWindowDelay();
				return false;
			}
		});
		
		mGestureDetector = new GestureDetector(new SimpleOnGestureListener(){

			//单击屏幕
			@Override
			public boolean onSingleTapConfirmed(MotionEvent e) {
				// TODO Auto-generated method stub
				if (isPopupWindowShow) {
					hidePopupWindow();
					cancelDelayHide();
				} else {
					showPopupWindow();
					cancelDelayHide();
					hidePopupWindowDelay();
				}
				if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
					titlePopupWindow.update(0, 0, screenWidth, titleHeight);
				}
				return super.onSingleTapConfirmed(e);
			}
		});
	}
	
	private void initData() {
		mContext = PlayerActivity.this;
		getScreenSize();
		
		//注册广播
		playerReceiver = new PlayerReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(PLAYER_BROADCAST_ACTION);
		filter.addAction(REQUEST_TIMES_ACTION);
		registerReceiver(playerReceiver, filter);
		
		Intent intent = this.getIntent();
		deviceName = (String) intent.getCharSequenceExtra("deviceName");
		dealerName = (String) intent.getCharSequenceExtra("dealerName");
		Value.TerminalDealerName = dealerName;
		
		//【打开通道】
		TunnelCommunication.getInstance().tunnelInitialize("com/video/play/TunnelCommunication");
		TunnelCommunication.getInstance().openTunnel(dealerName);
		mDialog = createLoadingDialog("正在请求视频...");
		mDialog.show();
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case HIDE_POPUPWINDOW:
					hidePopupWindow();
					break;
			}
		}
	};
	
	public static Handler playHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case 0:
					Value.isTunnelOpened = true;
					String peerId = (String) msg.obj;
					if (peerId.equals(dealerName)) {
						//【播放视频】
						TunnelCommunication.getInstance().askMediaData(dealerName);
						videoView.playVideo();
						if (mDialog != null) {
							mDialog.dismiss();
							mDialog = null;
						}
					}
					break;
				case 1:
					if (mDialog != null) {
						mDialog.dismiss();
					}
					if (!Value.isTunnelOpened) {
						toastNotify(mContext, "请求视频超时，请重试！", Toast.LENGTH_SHORT);
					}
					Value.isTunnelOpened = false;
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
	public void sendHandlerMsg(int what, int timeout) {
		Message msg = new Message();
		msg.what = what;
		playHandler.sendMessageDelayed(msg, timeout);
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
		titlePopupWindow.update(0, 0, screenWidth, titleHeight);
		bottomPopupWindow.update(0, 0, screenWidth, bottomHeight);
	}
	
	/**
	 * 隐藏PopupWindow
	 */
	private void hidePopupWindow() {
		isPopupWindowShow = false;
		if (titlePopupWindow.isShowing()) {
			if (this.getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
				titlePopupWindow.update(0, 0, 0, 0);
			}
		}
		if (bottomPopupWindow.isShowing()) {
			bottomPopupWindow.update(0, 0, 0, 0);
		}
	}
	
	/**
	 * 延迟隐藏控制器
	 */
	private void hidePopupWindowDelay() {
		handler.sendEmptyMessageDelayed(HIDE_POPUPWINDOW, SHOW_TIME_MS);
	}
	
	/**
	 * 取消延迟隐藏
	 */
	private void cancelDelayHide() {
		handler.removeMessages(HIDE_POPUPWINDOW);
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
	 * @param context 上下文
	 * @param msg 显示的信息
	 * @return 返回Dialog
	 */
	private Dialog createLoadingDialog(String msg) {
		LayoutInflater inflater = LayoutInflater.from(mContext);
		View v = inflater.inflate(R.layout.dialog_player_layout, null);
		LinearLayout layout = (LinearLayout) v.findViewById(R.id.dialog_view);
		ImageView spaceshipImage = (ImageView) v.findViewById(R.id.dialog_img);
		TextView tipTextView = (TextView) v.findViewById(R.id.dialog_textView);
		Animation hyperspaceJumpAnimation = AnimationUtils.loadAnimation(mContext, R.anim.dialog_anim);
		spaceshipImage.startAnimation(hyperspaceJumpAnimation);
		tipTextView.setText(msg);
		Dialog loadingDialog = new Dialog(mContext, R.style.dialog_player_style);
		loadingDialog.setCancelable(false);
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
			cancelDelayHide();
			hidePopupWindowDelay();
		} else {
			hidePopupWindow();
			cancelDelayHide();
		}
        if (this.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
			titlePopupWindow.update(0, 0, screenWidth, titleHeight);
		}
	}
	
	/**
	 * 播放抓拍声音
	 */
	private void playMyMusic(int resid) {
		if (!isPlayMusic) {
			MediaPlayer mediaPlayer = null;
			if (mediaPlayer == null) {
				mediaPlayer = MediaPlayer.create(mContext, resid);
				mediaPlayer.stop();
			}
			mediaPlayer.setOnCompletionListener(new OnCompletionListener() {
				@Override
				public void onCompletion(MediaPlayer mp) {
					mp.release();
					mp = null;
					isPlayMusic = false;
				}
			});
			try {
				mediaPlayer.prepare();
				mediaPlayer.start();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			//关闭播放器
			case R.id.ib_player_back:
				closePlayer();
				finish();
				break;
			//截屏
			case R.id.btn_player_capture:
				if (videoView.captureVideo()) {
					playMyMusic(R.raw.capture);
					toastNotify(mContext, "抓拍成功", Toast.LENGTH_SHORT);
				} else {
					toastNotify(mContext, "抓拍失败", Toast.LENGTH_SHORT);
				}
				break;
			//录像
			case R.id.btn_player_record:
				try {
					if (!isRecordVideo) {
						String videoName = videoView.captureThumbnails();
						if (videoName != null) {
							//开始录视频
							isRecordVideo = true;
							playMyMusic(R.raw.record);
							player_record.setBackgroundResource(R.drawable.player_recording_xml);
							
							String SDPath = Environment.getExternalStorageDirectory().getAbsolutePath();
							String filePath1 = SDPath + File.separator + "KaerVideo";
							File videoFilePath1 = new File(filePath1);
							if(!videoFilePath1.exists()){
								videoFilePath1.mkdir();
							} 
							String filePath2 = filePath1 + File.separator + "video";
							File videoFilePath2 = new File(filePath2);
							if(!videoFilePath2.exists()){
								videoFilePath2.mkdir();
							} 
							String filePath3 = filePath2 + File.separator +Utils.getNowTime("yyyy-MM-dd");
							File videoFilePath3 = new File(filePath3);
							if(!videoFilePath3.exists()){
								videoFilePath3.mkdir();
							} 
							String videoFile = filePath3 + File.separator +videoName + ".avi";
							TunnelCommunication.getInstance().startRecordVideo(dealerName, videoFile);
						} else {
							toastNotify(mContext, "录像失败！", Toast.LENGTH_SHORT);
						}
					} else {
						//停止录视频
						isRecordVideo = false;
						TunnelCommunication.getInstance().stopRecordVideo(dealerName);
						player_record.setBackgroundResource(R.drawable.player_record_xml);
					}
				} catch (Exception e) {
					System.out.println("MyDebug: 录像异常！");
					e.printStackTrace();
				}
				break;
			//对讲
			case R.id.btn_player_talkback:
				if (Value.isSharedUser) {
					toastNotify(mContext, "您无权使用对讲功能！", Toast.LENGTH_SHORT);
				} else {
					playMyMusic(R.raw.di);
					if (isTalkEnable) {
						//停止对讲
						isTalkEnable =false;
						player_talkback.setBackgroundResource(R.drawable.player_talkstop_xml);
						if (talkThread != null) {
							talkThread.stopTalkThread();
						}
					} else {
						//开始对讲
						isTalkEnable = true;
						player_talkback.setBackgroundResource(R.drawable.player_talkback_xml);
						talkThread = new TalkThread();
						if (talkThread != null) {
							talkThread.start();
						}
					}
				}
				break;
			//声音
			case R.id.btn_player_sound:
				if (isVoiceEnable) {
					isVoiceEnable = false;
					player_sound.setBackgroundResource(R.drawable.player_sound_disable_xml);
					audioThread.closeAudioTrackVolume();
				} else {
					isVoiceEnable = true;
					player_sound.setBackgroundResource(R.drawable.player_sound_enable_xml);
					audioThread.openAudioTrackVolume();
				}
				break;
		}
		if (v.getId() != R.id.ib_player_back) {
			cancelDelayHide();
			hidePopupWindowDelay();
		}
	}
	
	/**
	 * @param order: "move_left", "move_right", "move_up", "move_down", "stop" 
	 * @return 返回生成JSON云台控制的字符串
	 */
	private String generatePtzControlJson(String order) {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "tunnel");
			jsonObj.put("command", "ptz");
			jsonObj.put("control", order);
			jsonObj.put("param", 0);
			return jsonObj.toString();
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	private void sendPtzControlOrder(String order) {
		Handler sendHandler = HandlerApplication.getInstance().getMyHandler();
		String data = generatePtzControlJson(order);
		HashMap<String, String> map = new HashMap<String, String>();
		map.put("peerId", dealerName);
		map.put("peerData", data);
		sendHandlerMsg(sendHandler, R.id.send_to_peer_id, map); 
	}
	
	private boolean isPtzControling = false;
	private long startTime = 0;
	private long endTime = 0;
	private long spaceTime = 0;
	
	private final String PTZ_UP = "move_up";
	private final String PTZ_DOWN = "move_down";
	private final String PTZ_LEFT = "move_left";
	private final String PTZ_RIGHT = "move_right";
	private final String PTZ_STOP = "stop";
	
	private float startPointX = 0.0f;
	private float startPointY = 0.0f;
	
	private float endPointX = 0.0f;
	private float endPointY = 0.0f;
	
	private float spaceX = 0.0f;
	private float spaceY = 0.0f;
	
	//处理云台事件
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		// TODO Auto-generated method stub
		boolean result = mGestureDetector.onTouchEvent(event);

		if (!result) {
			if (!Value.isSharedUser) {
				switch (event.getAction()) {
					case MotionEvent.ACTION_UP:
						if (isPtzControling) {
							//停止
							isPtzControling = false;
							sendPtzControlOrder(PTZ_STOP);
						}
						break;
					case MotionEvent.ACTION_DOWN:
						isPtzControling = false;
						startTime = System.currentTimeMillis();
						startPointX = event.getX();
						startPointY = event.getY();
						break;
					case MotionEvent.ACTION_MOVE:
						endTime = System.currentTimeMillis();
						spaceTime = endTime - startTime;
						if (!isPtzControling && (spaceTime > 100)) {
							isPtzControling = true;
							
							endPointX = event.getX();
							endPointY = event.getY();
							spaceX = endPointX - startPointX;
							spaceY = endPointY - startPointY;
							
							if (Math.abs(spaceX) >= Math.abs(spaceY)) {
								if (spaceX < -1) {
									//向左
									sendPtzControlOrder(PTZ_LEFT);
								} 
								else if (spaceX > 1) {
									//向右
									sendPtzControlOrder(PTZ_RIGHT);
								}
							} else {
								if (spaceY < -1) {
									//向上
									sendPtzControlOrder(PTZ_UP);
								} 
								else if (spaceY > 1) {
									//向下
									sendPtzControlOrder(PTZ_DOWN);
								}
							}
						}
						break;
				}
			}
			result = super.onTouchEvent(event);
		}
		return result;
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
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		//注销广播
		unregisterReceiver(playerReceiver);
		//解除屏幕保持唤醒
		wakeLock.release(); 
		wakeLock = null;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			closePlayer();
			finish();
		}
		return super.onKeyDown(keyCode, event);
	}

	/**
	 * 关闭播放器
	 */
	private void closePlayer() {
		try {
			//停止录视频
			if (isRecordVideo) {
				isRecordVideo = false;
				TunnelCommunication.getInstance().stopRecordVideo(dealerName);
			}
			//关闭实时音视频
			try {
				videoView.stopVideo();
				audioThread.stopAudioThread();
				if (talkThread != null) {
					talkThread.stopTalkThread();
				}
			} catch (Exception e) {
				System.out.println("MyDebug: 关闭音视频对讲异常！");
				e.printStackTrace();
			}
			//关闭通道
			if (Value.isTunnelOpened) {
				TunnelCommunication.getInstance().closeTunnel(dealerName);
				TunnelCommunication.getInstance().tunnelTerminate();
			}
			Value.TerminalDealerName = null;
			//销毁弹出框
			if (titlePopupWindow != null) {
				titlePopupWindow.dismiss();
			}
			if (bottomPopupWindow != null) {
				bottomPopupWindow.dismiss();
			}
		} catch (Exception e) {
			System.out.println("MyDebug: 关闭实时播放器异常！");
			e.printStackTrace();
		}
	}
	
	public class PlayerReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			String action = intent.getAction();
			if ((action.equals(PLAYER_BROADCAST_ACTION)) && (!Value.isTunnelOpened)) {
				try {
					videoView.stopVideo();
					audioThread.stopAudioThread();
					if (talkThread != null) {
						talkThread.stopTalkThread();
					}
				} catch (Exception e) {
					System.out.println("MyDebug: 关闭音视频对讲异常！");
					e.printStackTrace();
				}
				//视频
				videoView = new VideoView(mContext);
				setContentView(videoView);
				
				//音频
				audioThread = new AudioThread();
				if (audioThread != null) {
					audioThread.start();
				}
				TunnelCommunication.getInstance().tunnelInitialize("com/video/play/TunnelCommunication");
				TunnelCommunication.getInstance().openTunnel(dealerName);
				toastNotify(mContext, "正在重新请求视频...", Toast.LENGTH_LONG);
				System.out.println("MyDebug: 正在重新请求视频...");
			}
			else if (action.equals(REQUEST_TIMES_ACTION)) {
				closePlayer();
				PlayerActivity.this.finish();
			}
		}
	}
	
}
	
//	{
//		private boolean isClarityPopupWindowShow = false;
//		private Button clarity_high = null;
//		private Button clarity_normal = null;
//		private Button clarity_low = null;
//		private View clarityView = null;//底部视图
//		private PopupWindow clarityPopupWindow = null;//底部弹出框
		
		//视频质量弹出框
//		clarityView = getLayoutInflater().inflate(R.layout.player_clarity_view, null);
//		clarityPopupWindow = new PopupWindow(clarityView);
//		clarity_high = (Button) clarityView.findViewById(R.id.btn_video_clarity_high);
//		clarity_high.setOnClickListener(this);
//		clarity_normal = (Button) clarityView.findViewById(R.id.btn_video_clarity_normal);
//		clarity_normal.setOnClickListener(this);
//		clarity_low = (Button) clarityView.findViewById(R.id.btn_video_clarity_low);
//		clarity_low.setOnClickListener(this);
		
//		if (clarityPopupWindow != null) {
//			clarityPopupWindow.dismiss();
//			isClarityPopupWindowShow = false;
//		}
		
//		//视频清晰度选择
//	case R.id.btn_player_clarity:
//		if (isClarityPopupWindowShow) {
//			isClarityPopupWindowShow = false;
//			clarityPopupWindow.update(0, 0, 0, 0);
//		} else {
//			isClarityPopupWindowShow = true;
//			if (clarityPopupWindow != null && videoView != null) {
//				clarityPopupWindow.showAtLocation(videoView, Gravity.BOTTOM|Gravity.RIGHT, 0, 0);
//				clarityPopupWindow.update(10, (bottomHeight+10), LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
//			}
//		}
//		break;
//	//高清
//	case R.id.btn_video_clarity_high:
//		System.out.println("MyDebug: ---> btn_video_clarity_high");
//		if (isClarityPopupWindowShow) {
//			isClarityPopupWindowShow = false;
//			clarityPopupWindow.update(0, 0, 0, 0);
//		}
//		break;
//	//标准
//	case R.id.btn_video_clarity_normal:
//		System.out.println("MyDebug: ---> btn_video_clarity_normal");
//		if (isClarityPopupWindowShow) {
//			isClarityPopupWindowShow = false;
//			clarityPopupWindow.update(0, 0, 0, 0);
//		}
//		break;
//	//流畅
//	case R.id.btn_video_clarity_low:
//		System.out.println("MyDebug: ---> btn_video_clarity_low");
//		if (isClarityPopupWindowShow) {
//			isClarityPopupWindowShow = false;
//			clarityPopupWindow.update(0, 0, 0, 0);
//		}
//		break;
		
//		if (clarityPopupWindow != null) {
//			clarityPopupWindow.dismiss();
//		}
//	}

