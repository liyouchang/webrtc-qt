package com.video.terminal.player;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Environment;
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
import com.video.play.AudioCache;
import com.video.play.TunnelCommunication;
import com.video.play.VideoCache;
import com.video.play.VideoView;
import com.video.utils.Tools;
import com.video.utils.Utils;

public class TerminalPlayerActivity  extends Activity implements OnClickListener  {

	private static Context mContext;
	
	private static VideoView videoView = null; //视频对象
//	private static AudioThread audioThread = null; //音频对象
	private WakeLock wakeLock = null; //锁屏对象
	
	public static int requestPlayerTimes = 0;
	private PlayerReceiver playerReceiver; 
	public static final String TUNNEL_REQUEST_ACTION = "com.video.play.PlayerActivity.TunnelRequest";
	public static final String REQUEST_TERMINAL_VIDEO_ACTION = "com.video.terminal.player.TerminalPlayerActivity.request";

	public File videoSavePath = null; // 文件夹的缓存路径
	private ArrayList<HashMap<String, String>> fileList = null;
	private HashMap<String, String> currentFile = null;
	private FileInfo fileInfo = null;
	private int playSize = 1024*512;
	private int jniCallbackParam = -1;
	private boolean isLocalFile = false;
	private static String dealerName = null;
	private static Dialog mDialog = null;
	
	private ReadTerminalVideoFileThread readFileThread = null;
	
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
	private TimeCountThread timeCountThread = null; // 按秒更新SeekBar进度
	
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
//		if (audioThread != null) {
//			audioThread.start();
//		}
		
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
		seekBar.setMax((int)fileInfo.currentFileSize);
		durationTextView.setText("总大小: "+fileInfo.currentFileSize+"B");
	}
	
	@SuppressWarnings("unchecked")
	private void initData() {
		mContext = TerminalPlayerActivity.this;
		Value.playTerminalVideoFileFlag = true;
		getScreenSize();
		
		//注册广播
		playerReceiver = new PlayerReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(REQUEST_TERMINAL_VIDEO_ACTION);
		filter.addAction(TUNNEL_REQUEST_ACTION);
		registerReceiver(playerReceiver, filter);
		
		//获得终端录像的信息		
		Intent intent = this.getIntent();
		if (intent != null) {
			fileInfo = new FileInfo();
			dealerName = (String) intent.getCharSequenceExtra("dealerName");
			fileList = (ArrayList<HashMap<String, String>>) intent.getSerializableExtra("fileList");
			fileInfo.currentFileIndex = intent.getIntExtra("fileIndex", 0);
			currentFile = fileList.get(fileInfo.currentFileIndex);
			
			Value.TerminalDealerName = dealerName;
			fileInfo.currentFileDate = currentFile.get("fileDate");
			fileInfo.currentFileName = currentFile.get("fileName");
			fileInfo.currentFileSize = Integer.parseInt(currentFile.get("fileSizeInt"));
			
			if (Utils.checkSDCard()) {
				String SDPath = Environment.getExternalStorageDirectory().getAbsolutePath();
				String videoPath1 = SDPath + File.separator + "KaerVideo";
				File videoFilePath1 = new File(videoPath1);
				if(!videoFilePath1.exists()){
					videoFilePath1.mkdir();
				} 
				String videoPath2 = videoPath1 + File.separator + "cache";
				videoSavePath = new File(videoPath2);
				if(!videoSavePath.exists()){
					videoSavePath.mkdir();
				}
				fileInfo.fileCachePath = videoPath2 + File.separator + fileInfo.currentFileDate + ".h264";
				
				//【打开通道】
				TunnelCommunication.getInstance().openTunnel(dealerName);
				
				mDialog = createLoadingDialog("正在请求终端录像...");
				mDialog.show();
				sendHandlerMsg(REQUEST_TIME_OUT, 60000); // 60S
			} else {
				Toast.makeText(mContext, "未发现手机SD卡，无法缓存文件！", Toast.LENGTH_LONG).show();
				Value.isTunnelOpened = false;
				closePlayer();
				TerminalPlayerActivity.this.finish();
			}
		}
	}
	
	private class FileInfo {
		public int currentFileIndex = 0; // 当前文件在文件列表中的位置
		public String currentFileName = null; // 文件名(终端SD卡上的路径)
		public String currentFileDate = null; // 文件日期
		public long currentFileSize = 0; // 文件大小
		public int downloadSize = 0; // 文件下载大小
		public int readFileSize = 0; // 读取文件总的字节数
		public String fileCachePath = null; // 当前文件的缓存路径
		public int fileReadSpeed = 0; // 读取文件的速度
	}
	
	public class ReadTerminalVideoFileThread extends Thread {
		
		private RandomAccessFile randomFile = null;
		private boolean runFlag = false;
		
		public ReadTerminalVideoFileThread(boolean isRun) {
			runFlag = isRun;
			if (timeCountThread == null) {
				timeCountThread = new TimeCountThread(true);
			}
			if (TunnelCommunication.videoDataCache == null) {
				TunnelCommunication.videoDataCache = new VideoCache(1024*1024);
				TunnelCommunication.videoDataCache.clearBuffer();
			}
			if (TunnelCommunication.audioDataCache == null) {
				TunnelCommunication.audioDataCache = new AudioCache(1024*1024);
				TunnelCommunication.audioDataCache.clearBuffer();
			}
		}
		
		/**
		 * 播放本地的前一个或后一个文件
		 */
		public void readNewFile() {
			pauseTerminalVideoFile();
			TunnelCommunication.videoDataCache.clearBuffer();
			TunnelCommunication.audioDataCache.clearBuffer();
			try {
				// 关闭当前的终端录像文件
				if (randomFile != null) {
					randomFile.close();
					randomFile = null; 
				}
				// 打开新的终端录像文件
				randomFile = new RandomAccessFile(fileInfo.fileCachePath, "r");
				randomFile.seek(0);
				// 文件总大小
				fileInfo.currentFileSize = randomFile.length();
				sendHandlerMsg(handler, SEEKBAR_FILE_SIZE, (int)fileInfo.currentFileSize);
				// 进度条从0更新
				fileInfo.readFileSize = 0;
				sendHandlerMsg(handler, SEEKBAR_REFRESH, 0);
				playTerminalVideoFile();
				System.out.println("MyDebug: 正在播放本地的前一个或后一个文件...");
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		/**
		 * 停止读文件
		 */
		public void stopRead() {
			runFlag = false;
			if (readFileThread != null) {
				readFileThread.interrupt();
				readFileThread = null;
			}
			
			try {
				if (randomFile != null) {
					randomFile.close();
					randomFile = null; 
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		
		@Override
		public void run() {
			
			int readCount = 1024*100;
			int readSize = 0;
			byte frameType = (byte) 0xFF;
			byte[] fileData = new byte[readCount];
			long filePointer = 0;
			
			byte[] naluData = new byte[TunnelCommunication.width*TunnelCommunication.height*3];
			int naluDataLen = 4;
			
			int frameNo = 0, lastFrameNo = 0;
			int frameTime = 0, lastFrameTime = 0, spaceTime = 0;
			
			if (timeCountThread != null) {
				timeCountThread.start();
			}
			
			while (runFlag) {
				try {
					
					if (!videoView.isPlayVideo) {
						// 暂停不向下处理
						sleep(100);
						if (randomFile != null) {
							// 文件下载大小
							if ((jniCallbackParam != 2) || (jniCallbackParam != 3) || (jniCallbackParam != 5)) {
								fileInfo.downloadSize = (int) randomFile.length();
							}
							// 处理SeekBar拖拽
							filePointer = randomFile.getFilePointer();
							if ((dragSeekBarProgress > 0) && (dragSeekBarProgress < randomFile.length())) {
								randomFile.seek(dragSeekBarProgress);
								dragSeekBarProgress = 0;
								fileInfo.readFileSize = (int) randomFile.getFilePointer();
								if (!filePointerMoveToValidPosition(randomFile, 0)) {
									if (!filePointerMoveToValidPosition(randomFile, 1)) {
										randomFile.seek(filePointer);
									}
								}
								System.out.println("MyDebug: 【结束拖拽后指针位置】"+randomFile.getFilePointer());
							}
						}
						continue;
					}
					
					if (randomFile == null) {
						// 打开终端录像文件
						randomFile = new RandomAccessFile(fileInfo.fileCachePath, "r");
						randomFile.seek(0);
					} 
					
					if (randomFile != null) {
						
						filePointer = randomFile.getFilePointer();
						readSize = randomFile.read(fileData, 0, 12);
						fileInfo.readFileSize += readSize;
						
						// 文件下载大小
						if ((jniCallbackParam != 2) || (jniCallbackParam != 3) || (jniCallbackParam != 5)) {
							fileInfo.downloadSize = (int) randomFile.length();
						}
						
						// 文件读取完毕
						if (fileInfo.readFileSize >= fileInfo.currentFileSize) {
							System.out.println("MyDebug: 【文件读取完毕】");
							sendHandlerMsg(READ_FILE_OVER);
							if (randomFile != null) {
								randomFile.close();
								randomFile = null;
							}
						}
						
						frameNo = Tools.getWordValue(fileData, 0);
						frameType = fileData[9];
						if ((frameType & 0x5F) < 30) {
							// 【视频数据】
							TunnelCommunication.videoFrameType = frameType;
							int frameLen = Tools.getWordValue(fileData, 10);
							frameTime = Tools.getWordValue(fileData, 4)*100+ fileData[8];
							
							filePointer = randomFile.getFilePointer();
							readSize = randomFile.read(fileData, 0, frameLen);
							if ((readSize <= 0) || (readSize > readCount)) {
								// 读错数据重新读取
								Thread.sleep(100);
								if (!filePointerMoveToValidPosition(randomFile, 0)) {
									if (!filePointerMoveToValidPosition(randomFile, 1)) {
										randomFile.seek(filePointer);
										fileInfo.readFileSize = (int) randomFile.getFilePointer();
									}
								}
								continue;
							} else {
								fileInfo.readFileSize += readSize;
							}
							
							// 快进、慢进
							if (lastFrameNo != frameNo) {
								spaceTime = (frameTime - lastFrameTime) * 10;
								lastFrameNo = frameNo;
								lastFrameTime = frameTime;
								double fX = 0, fY = 0, fZ = 0;
								
								if (spaceTime <= 0 || spaceTime > 1000) {
									if (spaceTime > 10000) {
										spaceTime = 40;
									} else {
										spaceTime = 1000;
									}
								}
								fY = fileInfo.fileReadSpeed;
								if (fY < 0) {
									double betweenTime = (double) spaceTime;
									fY = -fY;
									fX = Math.pow((double) 2, (double) (fY));
									fZ = 1 / fX;
									betweenTime = betweenTime / fZ;
									Thread.sleep((int)Math.ceil(betweenTime));
								} else {
									spaceTime = spaceTime / (int) Math.pow((double) 2, (double) fY);
									Thread.sleep(spaceTime);
								}
							}
							
							// 抛帧播放视频
							switch (fileInfo.fileReadSpeed) {
								case 1:
									if (frameNo%2 == 0) {
										continue;
									}
									break;
								case 2:
									if ((frameNo%2 == 0) || (frameNo%3 == 0)) {
										continue;
									}
									break;
								case 3:
									if ((frameNo%2 == 0) || (frameNo%3 == 0) || (frameNo%5 == 0)) {
										continue;
									}
									break;
							}
							 
							int pushPosition = 0;
							if ((byte)(frameType & 0x80) != 0) {
								pushPosition = 4;
							} else {
								if(naluDataLen > 4 ){
									Tools.setIntValue(naluData, 0, naluDataLen-4);
									if (TunnelCommunication.videoDataCache.push(naluData, naluDataLen) != 0) {
										TunnelCommunication.videoDataCache.clearBuffer();
									}
									naluDataLen = 4;
								}
							}
							int naluLen = readSize - pushPosition;
							Tools.CopyByteArray(naluData, naluDataLen, fileData, pushPosition, naluLen);
							naluDataLen += naluLen;
						} else {
							// 【音频数据】
							int frameLen = Tools.getWordValue(fileData, 10);
							filePointer = randomFile.getFilePointer();
							readSize = randomFile.read(fileData, 4, frameLen);
							
							// 抛帧播放音频
							switch (fileInfo.fileReadSpeed) {
								case 1:
									if (frameNo%2 == 0) {
										continue;
									}
									break;
								case 2:
									if ((frameNo%2 == 0) || (frameNo%3 == 0)) {
										continue;
									}
									break;
								case 3:
									if ((frameNo%2 == 0) || (frameNo%3 == 0) || (frameNo%5 == 0)) {
										continue;
									}
									break;
							}
							
							if ((readSize <= 0) || (readSize > readCount)) {
								// 读错数据重新读取
								Thread.sleep(100);
								if (!filePointerMoveToValidPosition(randomFile, 0)) {
									if (!filePointerMoveToValidPosition(randomFile, 1)) {
										randomFile.seek(filePointer);
										fileInfo.readFileSize = (int) randomFile.getFilePointer();
									}
								}
								continue;
							} else {
								fileInfo.readFileSize += readSize;
							}
//							TunnelCommunication.audioDataCache.push(fileData, 0, readSize);
						}
					}
				} catch (Exception e) {
					e.printStackTrace();
					try {
						if (randomFile != null) {
							randomFile.close();
							randomFile = null; 
						}
					} catch (IOException ex) {
						ex.printStackTrace();
					}
				}
			}
			
			try {
				if (randomFile != null) {
					randomFile.close();
					randomFile = null; 
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		
		/**
		 * 将文件指针移动到有效的位置
		 * @param randomFile 随机读取文件
		 * @param type 0:向前移动 1:向后移动
		 * @return
		 */
		private boolean filePointerMoveToValidPosition(RandomAccessFile randomFile, int type) {
			boolean result = false;
			int readCount = 1024*512;
			byte[] byDataBuffer = new byte[readCount];
			long lFilePos = 0, lFileSrcPos = 0;
			int iDatalen, iValue, iReadLen = 0;
			try {
				lFilePos = randomFile.getFilePointer();
				lFileSrcPos = lFilePos;
				if (type == 0) {
					// 向前移动
					while (true) {
						if (result == true || lFilePos == 0) {
							break;
						}

						lFilePos -= 2048;
						if (lFilePos < 0) {
							lFilePos = 0;
						}
						randomFile.seek(lFilePos);
						iDatalen = (int) (lFileSrcPos - lFilePos);
						if (iDatalen > readCount) {
							break;
						}
						iReadLen = randomFile.read(byDataBuffer, 0, iDatalen);
						for (int i = 0; i < iReadLen; i++) {
							iValue = Tools.getIntValue(byDataBuffer, i);
							if ((iValue == 0x01000000)
									&& (byDataBuffer[i + 4] == 0x67)
									&& (byDataBuffer[i + 5] == 0x42) && i >= 12) {
								result = true;
								randomFile.seek(lFilePos + i - 12);
								System.out.println("MyDebug: 【向前修正成功】"+randomFile.getFilePointer());
								break;
							}
						}
					}
				} else {
					// 向后移动
					while (true) {
						iReadLen = 512 * 1024;
						if ((result == true) || (iReadLen < 512 * 1024)) {
							break;
						}
						iReadLen = randomFile.read(byDataBuffer, 0, readCount);
						for (int i = 0; i < iReadLen; i++) {
							iValue = Tools.getIntValue(byDataBuffer, i);
							if ((iValue == 0x01000000)
									&& ((byDataBuffer[i + 4] & 0x1f) == 7)
									&& i >= 12) {
								result = true;
								randomFile.seek(lFilePos + i - 12);
								System.out.println("MyDebug: 【向后修正成功】"+randomFile.getFilePointer());
								break;
							}
						}
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
			return result;
		}
	}
	
	class TimeCountThread extends Thread {
		
		private boolean runFlag = false;
		private boolean isPaused = false;
		
		public TimeCountThread(boolean isRun) {
			runFlag = isRun;
		}
		
		/**
		 * 开始计数
		 */
		public void startCount() {
			isPaused = false;
		}
		
		/**
		 * 暂停计数
		 */
		public void pauseCount() {
			isPaused = true;
		}
		
		public void run() {
			int timeCount = 0;

			while (runFlag) {
				try {
					Thread.sleep(100);
					if (isPaused) {
						continue;
					}
					timeCount ++;
					if (timeCount >= 10) {
						timeCount = 0;
						sendHandlerMsg(handler, SEEKBAR_REFRESH, fileInfo.readFileSize, fileInfo.downloadSize);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
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
	 * 获得前一个列表文件信息
	 * @return 1:前面没有文件  2:该文件已下载完成  3:从终端获取  4:前一个文件没有下载
	 */
	private int readFrontTerminalVideoFile() {
		fileInfo.currentFileIndex --;
		if (fileInfo.currentFileIndex < 0) {
			fileInfo.currentFileIndex ++;
			toastNotify(mContext, "前面没有文件了", Toast.LENGTH_SHORT);
			return 1;
		} else {
			currentFile = fileList.get(fileInfo.currentFileIndex);
			fileInfo.currentFileDate = currentFile.get("fileDate");
			fileInfo.currentFileName = currentFile.get("fileName");
			fileInfo.currentFileSize = Integer.parseInt(currentFile.get("fileSizeInt"));
			fileInfo.fileCachePath = videoSavePath.getPath() + File.separator + fileInfo.currentFileDate + ".h264";
			File file = new File(fileInfo.fileCachePath);
			if (file.exists()) {
				if (file.length() > (fileInfo.currentFileSize/4)) {
					readFileThread.readNewFile();
					isLocalFile = true;
					tv_download.setVisibility(View.INVISIBLE);
					return 2;
				} else {
					file.delete();
					toastNotify(mContext, "前一个文件还没有下载", Toast.LENGTH_SHORT);
					return 4;
				}
			} else {
				if ((jniCallbackParam == 3) && (isLocalFile)) {
					pauseTerminalVideoFile();
					isLocalFile = false;
					// 文件总大小
					sendHandlerMsg(handler, SEEKBAR_FILE_SIZE, (int)fileInfo.currentFileSize);
					// 进度条从0更新
					fileInfo.readFileSize = 0;
					sendHandlerMsg(handler, SEEKBAR_REFRESH, 0);
					if (Value.isTunnelOpened) {
						//下载下一个终端录像文件
						if ((fileInfo.fileCachePath != null) || (dealerName != null)) {
							System.out.println("MyDebug: 【开始下载前一个录像文件】");
							TunnelCommunication.getInstance().downloadRemoteFile(dealerName,
											fileInfo.currentFileName,
											fileInfo.fileCachePath,
											playSize);
						}
					} else {
						//【打开通道】
						TunnelCommunication.getInstance().openTunnel(dealerName);
					}
					mDialog = createLoadingDialog("正在请求前一个终端录像...");
					mDialog.show();
					sendHandlerMsg(REQUEST_TIME_OUT, 60000); // 60S
					return 3;
				} else {
					toastNotify(mContext, "前一个文件还没有下载", Toast.LENGTH_SHORT);
					return 4;
				}
			}
		}
	}
	
	/**
	 * 获得后一个列表文件信息
	 * @return 1:后面没有文件  2:该文件已下载完成  3:从终端获取  4:后一个文件没有下载
	 */
	private int readBackTerminalVideoFile() {
		fileInfo.currentFileIndex ++;
		if (fileInfo.currentFileIndex >= fileList.size()) {
			fileInfo.currentFileIndex --;
			toastNotify(mContext, "后面没有文件了", Toast.LENGTH_SHORT);
			return 1;
		} else {
			currentFile = fileList.get(fileInfo.currentFileIndex);
			fileInfo.currentFileDate = currentFile.get("fileDate");
			fileInfo.currentFileName = currentFile.get("fileName");
			fileInfo.currentFileSize = Integer.parseInt(currentFile.get("fileSizeInt"));
			fileInfo.fileCachePath = videoSavePath.getPath() + File.separator + fileInfo.currentFileDate + ".h264";
			File file = new File(fileInfo.fileCachePath);
			if (file.exists()) {
				if (file.length() > (fileInfo.currentFileSize/4)) {
					readFileThread.readNewFile();
					isLocalFile = true;
					tv_download.setVisibility(View.INVISIBLE);
					return 2;
				} else {
					file.delete();
					toastNotify(mContext, "后一个文件还没有下载", Toast.LENGTH_SHORT);
					return 4;
				}
			} else {
				if ((jniCallbackParam == 3) || (isLocalFile)) {
					isLocalFile = false;
					pauseTerminalVideoFile();
					// 文件总大小
					sendHandlerMsg(handler, SEEKBAR_FILE_SIZE, (int)fileInfo.currentFileSize);
					// 进度条从0更新
					fileInfo.readFileSize = 0;
					sendHandlerMsg(handler, SEEKBAR_REFRESH, 0);
					if (Value.isTunnelOpened) {
						//下载下一个终端录像文件
						if ((fileInfo.fileCachePath != null) && (dealerName != null)) {
							System.out.println("MyDebug: 【开始下载后一个录像文件】");
							TunnelCommunication.getInstance().downloadRemoteFile(dealerName,
											fileInfo.currentFileName,
											fileInfo.fileCachePath,
											playSize);
						}
					} else {
						//【打开通道】
						TunnelCommunication.getInstance().openTunnel(dealerName);
					}
					mDialog = createLoadingDialog("正在请求后一个终端录像...");
					mDialog.show();
					sendHandlerMsg(REQUEST_TIME_OUT, 60000); // 60S
					return 3;
				} else {
					toastNotify(mContext, "后一个文件还没有下载", Toast.LENGTH_SHORT);
					return 4;
				}
			}
		}
	}
	
	/**
	 * 播放终端录像
	 */
	private void playTerminalVideoFile() {
		videoView.isPlayVideo = true;
		timeCountThread.startCount();
		button_play_pause.setImageResource(R.drawable.local_player_pause);
		tv_info.setText(getReadSpeedValue());
	}
	
	/**
	 * 暂停终端录像
	 */
	private void pauseTerminalVideoFile() {
		videoView.isPlayVideo = false;
		timeCountThread.pauseCount();
		button_play_pause.setImageResource(R.drawable.local_player_play);
		tv_info.setText("已暂停");
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			// 前一个文件
			case R.id.ib_front_file:
				readFrontTerminalVideoFile();
				break;
			// 慢进
			case R.id.ib_play_slow:
				fileInfo.fileReadSpeed --;
				if (fileInfo.fileReadSpeed < -3) {
					fileInfo.fileReadSpeed = -3;
				}
				playTerminalVideoFile();
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
				fileInfo.fileReadSpeed ++;
				if (fileInfo.fileReadSpeed > 3) {
					fileInfo.fileReadSpeed = 3;
				}
				playTerminalVideoFile();
				break;
			// 后一个文件
			case R.id.ib_back_file:
				readBackTerminalVideoFile();
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
			closePlayer();
			finish();
		}
		return super.onKeyDown(keyCode, event);
	}
	
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
					playedTextView.setText("已播放: "+msg.arg1+"B");
					if (isLocalFile) {
						tv_download.setVisibility(View.INVISIBLE);
					} else {
						tv_download.setVisibility(View.VISIBLE);
						if ((jniCallbackParam == 0) || (jniCallbackParam == 4))
							tv_download.setText("已缓冲: "+msg.arg2+"B");
						else if (jniCallbackParam == 3) {
							tv_download.setText("已缓冲完成");
						} else {
							tv_download.setVisibility(View.INVISIBLE);
						}
					}
					break;
				// 文件大小进度条
				case SEEKBAR_FILE_SIZE:
					seekBar.setMax(msg.arg1);
					durationTextView.setText("总大小: "+msg.arg1+"B");
					break;
				// 读取文件结束
				case READ_FILE_OVER:
					pauseTerminalVideoFile();
					playedTextView.setText("已播放: "+fileInfo.currentFileSize+"B");
					readBackTerminalVideoFile();
					break;
			}
		}
	};
	
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
		wakeLock.release(); 
		wakeLock = null;
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
		Value.playTerminalVideoFileFlag = false;
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
				if (readFileThread != null) {
					readFileThread.stopRead();
				}
			} catch (Exception e) {
				System.out.println("MyDebug: 关闭终端录像音视频对讲异常！");
				e.printStackTrace();
			}
			
			//关闭通道
			if (Value.isTunnelOpened) {
				TunnelCommunication.getInstance().closeTunnel(dealerName);
			}
			Value.TerminalDealerName = null;
			
			//删除缓存文件
			Utils.deleteFile(videoSavePath);
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
		switch (fileInfo.fileReadSpeed) {
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
			
			if (action.equals(REQUEST_TERMINAL_VIDEO_ACTION)) {
				
				int recordStatus = intent.getIntExtra("RecordStatus", -1);
				switch (recordStatus) {
					case 0: // 请求录像成功
						jniCallbackParam = 0;
						break;
					case 1: // 保存录像失败
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						jniCallbackParam = 1;
						break;
					case 2: // 请求文件错误
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						jniCallbackParam = 2;
						break;
					case 3: // 下载录像完成
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						jniCallbackParam = 3;
						isLocalFile = true;
						toastNotify(mContext, "录像下载完成", Toast.LENGTH_SHORT);
						break;
					case 4: // 达到播放阀值
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						if (handler.hasMessages(REQUEST_TIME_OUT)) {
							handler.removeMessages(REQUEST_TIME_OUT);
						}
						jniCallbackParam = 4;
						//播放终端录像文件
						if (readFileThread == null) {
							readFileThread = new ReadTerminalVideoFileThread(true);
							readFileThread.start();
						} else {
							playTerminalVideoFile();
						}
						videoView.playVideo();
						break;
					case 5: // 请求消息错误
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						jniCallbackParam = 5;
						break;
				}
			}
			else if (action.equals(TUNNEL_REQUEST_ACTION)) {
				int TunnelEvent = intent.getIntExtra("TunnelEvent", 1);
				switch (TunnelEvent) {
					// 通道打开
					case 0:
						Value.isTunnelOpened = true;
						String peerId = (String) intent.getCharSequenceExtra("PeerId");
						if (peerId.equals(dealerName)) {
							try {
								//下载终端录像文件
								if ((fileInfo.fileCachePath != null) && (dealerName != null)) {
									System.out.println("MyDebug: 【开始下载录像文件】");
									TunnelCommunication.getInstance().downloadRemoteFile(dealerName,
													fileInfo.currentFileName,
													fileInfo.fileCachePath,
													playSize);
								}
							} catch (Exception e) {
								e.printStackTrace();
							}
						}
						break;
					// 通道关闭
					case 1:
						if (mDialog.isShowing()) {
							mDialog.dismiss();
						}
						if (handler.hasMessages(REQUEST_TIME_OUT)) {
							handler.removeMessages(REQUEST_TIME_OUT);
						}
						if (!isLocalFile) {
							if (!Value.isTunnelOpened) {
								toastNotify(mContext, "请求终端录像超时，请重试！", Toast.LENGTH_SHORT);
							}
							closePlayer();
							TerminalPlayerActivity.this.finish();
						}
						Value.isTunnelOpened = false;
						break;
				}
			}
		}
	}
	
}
