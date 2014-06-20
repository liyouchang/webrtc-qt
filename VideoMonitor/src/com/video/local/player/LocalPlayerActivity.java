package com.video.local.player;

import java.io.File;
import java.io.FileFilter;
import java.util.LinkedList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.MessageQueue.IdleHandler;
import android.view.Display;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.PopupWindow;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.local.player.LocalSoundView.OnVolumeChangedListener;
import com.video.local.player.LocalVideoView.MySizeChangeLinstener;

public class LocalPlayerActivity extends Activity implements OnClickListener {

	private Context mContext;
	private boolean isOnline = false; // 是否在线播放
	private boolean isChangedVideo = false;// 是否改变视频

	public static LinkedList<MovieInfo> playList = new LinkedList<MovieInfo>();// 视频信息集合

	public class MovieInfo {
		String displayName;// 视频名称
		String path;// 视频路径
	}

	private static int position;// 定位
	private int playedTime;// 已播放时间

	public static LocalVideoView videoView = null; // 视频视图
	private SeekBar seekBar = null;// 可拖拽的进度条
	private TextView durationTextView = null;// 视频的总时间
	private TextView playedTextView = null;// 播放时间
	private GestureDetector mGestureDetector = null;// 手势识别
	private AudioManager mAudioManager = null; // 音频管理

	private int maxVolume = 0;// 最大声音
	private int currentVolume = 0;// 当前声音

	private ImageButton button_screen = null;// 全屏、标准
	private ImageButton button_front = null;// 上一个
	private ImageButton button_play_pause = null;// 播放、暂停
	private ImageButton button_back = null;// 下一个
	private ImageButton button_voice = null;// 声音

	private View controlView = null;// 控制器视图
	private PopupWindow controler = null;// 控制器

	private LocalSoundView mSoundView = null;// 声音视图
	private PopupWindow mSoundWindow = null;// 声音控制器

	private static int screenWidth = 0;// 屏幕宽度
	private static int screenHeight = 0;// 屏幕高度
	private static int controlHeight = 0; // 控制器高度

	private final static int TIME = 6868;// 控制器显示持续时间(毫秒)

	private boolean isControllerShow = true;// 是否显示控制器
	public static boolean isPaused = false;// 是否暂停
	private boolean isFullScreen = false;// 是否全屏
	private boolean isSilent = false;// 是否静音
	private boolean isSoundShow = false;// 是否显示声音

	private String videoFile = null;
	private String videoPath = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.local_player);

		initView();
		initData();

		/**
		 * 空闲的队列
		 */
		Looper.myQueue().addIdleHandler(new IdleHandler() {
			@Override
			public boolean queueIdle() {
				if (controler != null && videoView.isShown()) {
					controler.showAtLocation(videoView, Gravity.BOTTOM, 0, 0);
					controler.update(0, 0, screenWidth, controlHeight);
				}
				return false;
			}
		});
	}

	private void initView() {
		controlView = getLayoutInflater().inflate(R.layout.local_player_controler, null);
		controler = new PopupWindow(controlView);
		durationTextView = (TextView) controlView.findViewById(R.id.duration);
		playedTextView = (TextView) controlView.findViewById(R.id.has_played);
		
		button_screen = (ImageButton) controlView.findViewById(R.id.btn_full_screen);
		button_front = (ImageButton) controlView.findViewById(R.id.ib_front_file);
		button_play_pause = (ImageButton) controlView.findViewById(R.id.ib_play_pause);
		button_back = (ImageButton) controlView.findViewById(R.id.ib_back_file);
		button_voice = (ImageButton) controlView.findViewById(R.id.ib_voice);
		
		button_screen.setOnClickListener(this);
		button_front.setOnClickListener(this);
		button_play_pause.setOnClickListener(this);
		button_back.setOnClickListener(this);
		button_voice.setOnClickListener(this);
		
		button_screen.setAlpha(0xBB);
		button_front.setAlpha(0xBB);
		button_play_pause.setAlpha(0xBB);
		button_back.setAlpha(0xBB);

		videoView = (LocalVideoView) findViewById(R.id.vv);
		
		mSoundView = new LocalSoundView(this);
		mSoundView.setOnVolumeChangeListener(new OnVolumeChangedListener() {
			@Override
			public void setYourVolume(int index) {
				cancelDelayHide();// 取消隐藏延迟
				updateVolume(index);// 更新音量
				hideControllerDelay();// 延迟隐藏控制器
			}
		});
		mSoundWindow = new PopupWindow(mSoundView);

		mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
		maxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
		currentVolume = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
		button_voice.setAlpha(findAlphaFromSound());// 设置声音按键透明度
		button_voice.setOnLongClickListener(new OnLongClickListener() {
			@Override
			public boolean onLongClick(View arg0) {
				// TODO Auto-generated method stub
				if (isSilent) {
					button_voice.setImageResource(R.drawable.soundcontrol);
				} else {
					button_voice.setImageResource(R.drawable.soundmute);
				}
				isSilent = !isSilent;
				updateVolume(currentVolume);
				cancelDelayHide();// 取消隐藏延迟
				hideControllerDelay();// 延迟隐藏控制器
				return true;
			}
		});

		seekBar = (SeekBar) controlView.findViewById(R.id.seekbar);
		seekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			@Override
			public void onProgressChanged(SeekBar seekbar, int progress,
					boolean fromUser) {
				// TODO Auto-generated method stub
				if (fromUser) {
					videoView.seekTo(progress);// 设置播放位置
				}
			}

			@Override
			public void onStartTrackingTouch(SeekBar arg0) {
				// TODO Auto-generated method stub
				myHandler.removeMessages(HIDE_CONTROLER);
			}

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				myHandler.sendEmptyMessageDelayed(HIDE_CONTROLER, TIME);
			}
		});
	}

	private void initData() {
		getScreenSize();
		mContext = LocalPlayerActivity.this;
		
		Intent intent = this.getIntent();
		videoFile = (String) intent.getCharSequenceExtra("videoFile");
		videoPath = (String) intent.getCharSequenceExtra("videoPath");
		getVideoFile(playList, new File(videoFile + File.separator));// 获得视频文件
		position = -1;
		
		if (videoPath != null) {
			videoView.stopPlayback();// 停止视频播放
			videoView.setVideoPath(videoPath); // 设置视频文件Path
			isOnline = true;
			button_play_pause.setImageResource(R.drawable.local_player_pause);
			for (int i = 0; i < playList.size(); i++) {
				if (playList.get(i).path.equals(videoPath)) {
					position = i;
					break;
				}
			}
		} else {
			button_play_pause.setImageResource(R.drawable.local_player_play);
		}

		videoView.setMySizeChangeLinstener(new MySizeChangeLinstener() {
			@Override
			public void doMyThings() {
				// TODO Auto-generated method stub
				setVideoScale(SCREEN_DEFAULT);// 设置视频显示尺寸
			}
		});
		
		videoView.setOnErrorListener(new OnErrorListener() {
			@Override
			public boolean onError(MediaPlayer mp, int what, int extra) {

				videoView.stopPlayback();// 停止视频播放
				isOnline = false;

				new AlertDialog.Builder(mContext)
				.setTitle("对不起")
				.setMessage("您所播的视频格式不正确，播放已停止。")
				.setPositiveButton("知道了",
					new AlertDialog.OnClickListener() {
						@Override
						public void onClick(DialogInterface dialog, int which) {
							videoView.stopPlayback();
						}
					}).setCancelable(false).show();
				return false;
			}

		});

		mGestureDetector = new GestureDetector(new SimpleOnGestureListener() {

			@Override
			public boolean onDoubleTap(MotionEvent e) {
				// TODO Auto-generated method stub
				if (isFullScreen) {
					setVideoScale(SCREEN_DEFAULT);// 设置视频显示尺寸
				} else {
					setVideoScale(SCREEN_FULL);// 设置视频显示尺寸
				}
				isFullScreen = !isFullScreen;

				if (isControllerShow) {
					showController();// 显示控制器
				}
				// return super.onDoubleTap(e);
				return true;
			}

			@Override
			public boolean onSingleTapConfirmed(MotionEvent e) {// 轻击屏幕
				// TODO Auto-generated method stub
				if (!isControllerShow) {// 是否显示控制器
					showController();// 显示控制器
					hideControllerDelay();// 延迟隐藏控制器
				} else {
					cancelDelayHide();// 取消隐藏延迟
					hideController();// 隐藏控制器
				}
				// return super.onSingleTapConfirmed(e);
				return true;
			}

			@Override
			public void onLongPress(MotionEvent e) {// 长按屏幕
				// TODO Auto-generated method stub
				if (isPaused) {
					videoView.start();
					button_play_pause
							.setImageResource(R.drawable.local_player_pause);
					cancelDelayHide();// 取消隐藏延迟
					hideControllerDelay();// 延迟隐藏控制器
				} else {
					videoView.pause();
					button_play_pause
							.setImageResource(R.drawable.local_player_play);
					cancelDelayHide();// 取消隐藏延迟
					showController();// 显示控制器
				}
				isPaused = !isPaused;
				// super.onLongPress(e);
			}
		});

		videoView.setOnPreparedListener(new OnPreparedListener() {// 注册在媒体文件加载完毕，可以播放时调用的回调函数

			@Override
			public void onPrepared(MediaPlayer arg0) {// 加载
				// TODO Auto-generated method stub

				setVideoScale(SCREEN_DEFAULT);
				isFullScreen = false;
				if (isControllerShow) {
					showController();// 显示控制器
				}

				int i = videoView.getDuration();// 获得所播放视频的总时间
				seekBar.setMax(i);
				i /= 1000;
				int minute = i / 60;
				int hour = minute / 60;
				int second = i % 60;
				minute %= 60;
				durationTextView.setText(String.format("%02d:%02d:%02d", hour, minute, second));

				videoView.start();
				button_play_pause.setImageResource(R.drawable.local_player_pause);
				hideControllerDelay();// 延迟隐藏控制器
				myHandler.sendEmptyMessage(PROGRESS_CHANGED);
			}
		});

		videoView.setOnCompletionListener(new OnCompletionListener() {// 注册在媒体文件播放完毕时调用的回调函数

			@Override
			public void onCompletion(MediaPlayer arg0) {
				// TODO Auto-generated method stub
				int n = playList.size();
				isOnline = false;
				if (++position < n) {
					videoView.setVideoPath(playList.get(position).path);
				} else {
					videoView.stopPlayback();
					LocalPlayerActivity.this.finish();
				}
			}
		});
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch(v.getId()) {
			case R.id.btn_full_screen: // 设置视频显示尺寸
				if (isFullScreen) {
					setVideoScale(SCREEN_DEFAULT);
				} else {
					setVideoScale(SCREEN_FULL);
				}
				isFullScreen = !isFullScreen;

				if (isControllerShow) {
					showController();
				}
				hideControllerDelay();
				break;
			case R.id.ib_front_file: // 前一个视频文件
				isOnline = false;
				position--;
				if (position >= 0) {
					videoView.setVideoPath(playList.get(position).path);
					cancelDelayHide();
					hideControllerDelay();
				} else {
					position++;
					Toast.makeText(mContext, "前面没有了！", Toast.LENGTH_LONG).show();
				}
				break;
			case R.id.ib_play_pause: // 播放、暂停
				if (isPaused) {
					videoView.start();
					button_play_pause.setImageResource(R.drawable.local_player_pause);
					hideControllerDelay();
				} else {
					videoView.pause();
					button_play_pause.setImageResource(R.drawable.local_player_play);
				}
				isPaused = !isPaused;
				cancelDelayHide();
				break;
			case R.id.ib_back_file: // 后一个视频文件
				int n = playList.size();
				isOnline = false;
				position++;
				if (position < n) {
					videoView.setVideoPath(playList.get(position).path);
					cancelDelayHide();
					hideControllerDelay();
				} else {
					position--;
					Toast.makeText(mContext, "后面没有了！", Toast.LENGTH_LONG).show();
				}
				break;
			case R.id.ib_voice: // 视频声音
				if (isSoundShow) {
					mSoundWindow.dismiss();
				} else {
					if (mSoundWindow.isShowing()) {
						mSoundWindow.update(15, 0, LocalSoundView.MY_WIDTH, LocalSoundView.MY_HEIGHT);
					} else {
						mSoundWindow.showAtLocation(videoView, Gravity.RIGHT | Gravity.CENTER_VERTICAL, 15, 0);
						mSoundWindow.update(15, 0, LocalSoundView.MY_WIDTH, LocalSoundView.MY_HEIGHT);
					}
				}
				isSoundShow = !isSoundShow;
				cancelDelayHide();
				hideControllerDelay();
				break;
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		if (requestCode == 0 && resultCode == Activity.RESULT_OK) {

			videoView.stopPlayback();// 停止视频播放

			int result = data.getIntExtra("CHOOSE", -1);
			if (result != -1) {
				isOnline = false;
				isChangedVideo = true;
				videoView.setVideoPath(playList.get(result).path);
				position = result;
			} else {
				String url = data.getStringExtra("CHOOSE_URL");
				if (url != null) {
					videoView.setVideoPath(url);// 设置视频文件路径
					isOnline = true;
					isChangedVideo = true;
				}
			}

			return;
		}
		super.onActivityResult(requestCode, resultCode, data);
	}

	private final static int PROGRESS_CHANGED = 0;
	private final static int HIDE_CONTROLER = 1;

	Handler myHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			switch (msg.what) {
				case PROGRESS_CHANGED:// 进度改变
					int i = videoView.getCurrentPosition();
					seekBar.setProgress(i);
		
					if (isOnline) {
						int j = videoView.getBufferPercentage();
						seekBar.setSecondaryProgress(j * seekBar.getMax() / 100);
					} else {
						seekBar.setSecondaryProgress(0);
					}
		
					i /= 1000;
					int minute = i / 60;
					int hour = minute / 60;
					int second = i % 60;
					minute %= 60;
					playedTextView.setText(String.format("%02d:%02d:%02d", hour, minute, second));
					sendEmptyMessageDelayed(PROGRESS_CHANGED, 1000);
					break;
				case HIDE_CONTROLER:// 隐藏控制器
					hideController();// 隐藏控制器
					break;
			}

			super.handleMessage(msg);
		}
	};

	@Override
	public boolean onTouchEvent(MotionEvent event) {// 实现该方法来处理触屏事件
		// TODO Auto-generated method stub

		boolean result = mGestureDetector.onTouchEvent(event);

		if (!result) {
			if (event.getAction() == MotionEvent.ACTION_UP) {

				/*
				 * if(!isControllerShow){ showController();
				 * hideControllerDelay(); }else { cancelDelayHide();
				 * hideController(); }
				 */
			}
			result = super.onTouchEvent(event);
		}

		return result;
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		// TODO Auto-generated method stub

		getScreenSize();// 获得屏幕尺寸大小
		if (isControllerShow) {

			cancelDelayHide();// 取消隐藏延迟
			hideController();// 隐藏控制器
			showController();// 显示控制器
			hideControllerDelay();// 延迟隐藏控制器
		}

		super.onConfigurationChanged(newConfig);
	}

	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		playedTime = videoView.getCurrentPosition();
		videoView.pause();
		button_play_pause.setImageResource(R.drawable.local_player_play);
		super.onPause();
	}

	@Override
	protected void onResume() {// 恢复挂起的播放器
		// TODO Auto-generated method stub
		if (!isChangedVideo) {
			if (isPaused) {
				videoView.pause();
			} else {
				videoView.seekTo(playedTime);// 设置播放位置 playedTime已播放时间
				videoView.start();
			}
		} else {
			isChangedVideo = false;
		}
		
		if (isPaused) {
			videoView.pause();
		} else {
			videoView.start();
		}

		if (videoView.isPlaying()) {
			button_play_pause.setImageResource(R.drawable.local_player_pause);
			hideControllerDelay();// 延迟隐藏控制器
		}
		super.onResume();
	}

	@Override
	protected void onDestroy() {// 销毁
		// TODO Auto-generated method stub

		if (controler.isShowing()) {
			controler.dismiss();// 控制器销毁(释放资源)
		}
		if (mSoundWindow.isShowing()) {
			mSoundWindow.dismiss();// 声音控制器销毁(释放资源)
		}

		myHandler.removeMessages(PROGRESS_CHANGED);// 进度改变 释放
		myHandler.removeMessages(HIDE_CONTROLER);// 隐藏控制器 释放

		if (videoView.isPlaying()) {
			videoView.stopPlayback();// 停止视频播放
		}

		playList.clear();// 视频信息集合 清空

		super.onDestroy();
	}

	private void getScreenSize() {// 获得屏幕尺寸大小
		Display display = getWindowManager().getDefaultDisplay();
		screenHeight = display.getHeight();
		screenWidth = display.getWidth();
		controlHeight = screenHeight / 4;
	}

	private void hideController() {// 隐藏控制器
		if (controler.isShowing()) {
			controler.update(0, 0, 0, 0);
			isControllerShow = false;
		}
		if (mSoundWindow.isShowing()) {
			mSoundWindow.dismiss();
			isSoundShow = false;
		}
	}

	private void hideControllerDelay() {// 延迟隐藏控制器
		if (myHandler.hasMessages(HIDE_CONTROLER)) {
			myHandler.removeMessages(HIDE_CONTROLER);
		}
		myHandler.sendEmptyMessageDelayed(HIDE_CONTROLER, TIME);
	}

	private void showController() {// 显示控制器
		controler.update(0, 0, screenWidth, controlHeight);
		isControllerShow = true;
	}

	private void cancelDelayHide() {// 取消隐藏延迟
		if (myHandler.hasMessages(HIDE_CONTROLER)) {
			myHandler.removeMessages(HIDE_CONTROLER);
		}
	}

	private final static int SCREEN_FULL = 0;
	private final static int SCREEN_DEFAULT = 1;

	private void setVideoScale(int flag) {// 设置视频显示尺寸

		switch (flag) {
			case SCREEN_FULL:// 全屏
				videoView.setVideoScale(screenWidth, screenHeight);
				getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
				break;
			case SCREEN_DEFAULT:// 标准
				int videoWidth = videoView.getVideoWidth();
				int videoHeight = videoView.getVideoHeight();
				int mWidth = screenWidth;
				int mHeight = screenHeight - 25;
	
				if (videoWidth > 0 && videoHeight > 0) {
					if (videoWidth * mHeight > mWidth * videoHeight) {
						mHeight = mWidth * videoHeight / videoWidth;
					} else if (videoWidth * mHeight < mWidth * videoHeight) {
						mWidth = mHeight * videoWidth / videoHeight;
					} else {
					}
				}
	
				videoView.setVideoScale(mWidth, mHeight);
				getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
				break;
		}
		videoView.start();
		button_play_pause.setImageResource(R.drawable.local_player_pause);
		isPaused = false;
	}

	private int findAlphaFromSound() {// 设置声音按键透明度
		if (mAudioManager != null) {
			int alpha = currentVolume * (0xCC - 0x55) / maxVolume + 0x55;
			return alpha;
		} else {
			return 0xCC;
		}
	}

	private void updateVolume(int index) {// 更新音量
		if (mAudioManager != null) {
			if (isSilent) {// 是否静音
				mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, 0, 0);
			} else {
				mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, index, 0);
			}
			currentVolume = index;
			button_voice.setAlpha(findAlphaFromSound());
		}
	}

	private void getVideoFile(final LinkedList<MovieInfo> list, File file) {// 获得视频文件

		file.listFiles(new FileFilter() {

			@Override
			public boolean accept(File file) {
				// TODO Auto-generated method stub
				String name = file.getName();
				int i = name.indexOf('.');
				if (i != -1) {
					name = name.substring(i);
					if (name.equalsIgnoreCase(".avi")
							|| name.equalsIgnoreCase(".mp4")
							|| name.equalsIgnoreCase(".3gp")
							|| name.equalsIgnoreCase(".wmv")
							|| name.equalsIgnoreCase(".flv")) {

						MovieInfo mi = new MovieInfo();
						mi.displayName = file.getName();
						mi.path = file.getAbsolutePath();
						list.add(mi);
						return true;
					}
				}
				return false;
			}
		});
	}
}