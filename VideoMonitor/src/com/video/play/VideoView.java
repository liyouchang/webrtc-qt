package com.video.play;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.view.View;
import android.view.ViewGroup.LayoutParams;

import com.video.service.MainApplication;
import com.video.utils.Utils;

public class VideoView extends View {

	private Context mContext;
	private static int width = 1280; 
	private static int height = 720;

	private static byte[] decodeData = new byte[width*height*3];
	private static ByteBuffer videoBuffer = ByteBuffer.wrap(decodeData);
	private static Bitmap videoBmp = Bitmap.createBitmap(width, height, Config.RGB_565);

	private Rect srcRect = new Rect(0, 0, 0, 0) ;
	private Rect dstRect = new Rect(0, 0, 0, 0) ;
	private int drawWidth = 0;
	private int drawHeight = 0;  
	private int videoType = 0;
	private int bitWidth = 640;
	private int bitHeight = 480;

	private Canvas mCanvas = null;

	private PlayVideoThread playVideoThread = null; //播放视频线程
	private boolean runFlag = false; // 运行标志
	public boolean isPlayVideo = false; // 是否播放
	public boolean isFullScreen = true; // 是否全屏
	public boolean isDisplayView = false;

	//视频解码库JNI接口
	public native int initDecoder(int width, int height); 
	public native int uninitDecoder();
	public native int decodeNalu(byte[] in, int insize, byte[] out);

	public VideoView(Context context) {
		super(context);
		this.mContext = context;
	}

	public void initView() {
		initDecoder(width, height);
		if (TunnelCommunication.videoDataCache != null) {
			TunnelCommunication.videoDataCache.clearBuffer();
		}
	}

	public void uninitView() {
		TunnelCommunication.videoDataCache.clearBuffer();
		uninitDecoder();
	}

	/**
	 * 设置视频宽、高
	 * @param width
	 * @param height
	 */
	public void setVideoScale(int width , int height){
		LayoutParams lp = getLayoutParams();
		lp.height = height;
		lp.width = width;
		setLayoutParams(lp);
	}

	/**
	 * 播放视频
	 */
	public void playVideo() {
		runFlag = true;
		isPlayVideo = true;
		TunnelCommunication.videoDataCache.clearBuffer();
		if (playVideoThread == null) {
			playVideoThread = new PlayVideoThread();
			playVideoThread.start();
			Utils.log("【播放视频】");
		}
	}

	/**
	 * 停止播放
	 */
	public void stopVideo() {
		runFlag = false;
		isPlayVideo = false;
		TunnelCommunication.videoDataCache.clearBuffer();
		try {
			if (playVideoThread != null) {
				playVideoThread.join();
			}
			Utils.log("【停止视频】");
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		playVideoThread = null;
	}

	/**
	 * 播放视频线程
	 */
	private class PlayVideoThread extends Thread {

		public void run() {
			initView();
			int decodeLen = 0;
			byte[] naluData = new byte[100*1024];
			int naluDataLen = 0;
			try{
				while (runFlag) {
					// 播放、暂停
					if (!isPlayVideo) {
						sleep(100);
						continue;
					}
					naluDataLen = TunnelCommunication.videoDataCache.pop(naluData);
					if (naluDataLen > 0) {
						decodeLen = decodeNalu(naluData, naluDataLen, decodeData);
						if (decodeLen >= 0) {
							postInvalidate();
						}
						sleep(5);
					} else {
						sleep(10);
					}

				}
			}catch(Exception ex){
				ex.printStackTrace();
			}
			uninitView();
		}
	}

	@Override
	protected void onDraw(Canvas canvas) {
		try {
			super.onDraw(canvas);
			drawWidth = canvas.getWidth(); 
			drawHeight = canvas.getHeight();
			videoBuffer.rewind();
			videoBmp.copyPixelsFromBuffer(videoBuffer); // bmp从缓冲区获得数据

			videoType = (byte) (TunnelCommunication.videoFrameType & 0x5F);
			
			switch (videoType) {
				case 0:// D1
					bitWidth = 704;
					bitHeight = 576;
					break;
				case 1:// QCF
					bitWidth = 176;
					bitHeight = 144;
					break;
				case 2:// CIF
					bitWidth = 352;
					bitHeight = 288;
					break;
				case 3:// HD1
					bitWidth = 704;
					bitHeight = 288;
					break;
				case 4:// QVGA
					bitWidth = 320;
					bitHeight = 240;
					break;		
				case 7:// VGA
					bitWidth = 640;
					bitHeight = 480;
					break;
				case 10:// 720P
					bitWidth = 1280;
					bitHeight = 720;
					break;
			}
			
			srcRect.left = 0;
			srcRect.top = 0;
			srcRect.right = bitWidth;
			srcRect.bottom = bitHeight;

			if (mContext.getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
				// 横屏
				if (isFullScreen) {
					dstRect.left = 0;
					dstRect.top = 0;
					dstRect.right = drawWidth;
					dstRect.bottom = drawHeight;
				} else {
					int widthOffset = (drawWidth - drawHeight*bitWidth/bitHeight)/2;
					if (widthOffset < 0) {
						widthOffset = -widthOffset;
					}
					dstRect.left = widthOffset;
					dstRect.top = 0;
					dstRect.right = drawWidth - widthOffset;
					dstRect.bottom = drawHeight;
				}
			} else {
				// 竖屏
				if (drawWidth <= drawHeight) {
					dstRect.left = 0;
					dstRect.top = (int) ((drawHeight - drawWidth / 1.2) / 4);
					dstRect.right = drawWidth;
					dstRect.bottom = (int) (drawWidth / 1.2 + (drawHeight - drawWidth / 1.2) / 4);
				} else {
					dstRect.left = 0;
					dstRect.top = 0;
					dstRect.right = drawWidth;
					dstRect.bottom = drawHeight;
				}
			}

			// 绘制图像
			canvas.drawBitmap(videoBmp, srcRect, dstRect, null);
//			Utils.log("srcRect: "+srcRect.toString()+"  dstRect: "+dstRect.toString());
			if (mCanvas == null) {
				mCanvas = canvas;
			}
		} catch (Exception e) {
			Utils.log(" 绘制图像异常！");
			e.printStackTrace();
		}
	}

	/**
	 * 抓拍图片
	 */
	public boolean captureVideo() {
		if (mCanvas == null || !MainApplication.isSDExist) {
			return false;
		}
		Bitmap bm = Bitmap.createBitmap(videoBmp, 0, 0, bitWidth, bitHeight);
		mCanvas.save(Canvas.ALL_SAVE_FLAG);  
		mCanvas.restore();  

		String filePath1 = MainApplication.getInstance().imagePath + File.separator +Utils.getNowTime("yyyy-MM-dd");
		File imageFilePath1 = new File(filePath1);
		if(!imageFilePath1.exists()){
			imageFilePath1.mkdir();
		} 

		String imageFile = filePath1 + File.separator +Utils.getNowTime("yyyyMMddhhmmss") + ".jpg";
		File file = new File(imageFile);
		FileOutputStream fos = null;  
		try {  
			fos = new FileOutputStream(file);
			bm.compress(Bitmap.CompressFormat.JPEG, 100, fos);  
		} catch (FileNotFoundException e) {  
			e.printStackTrace(); 
			return false;
		} 
		return true;
	}

	/**
	 * 抓拍录像的缩略图
	 */
	public String captureThumbnails() {
		if (mCanvas == null || !MainApplication.isSDExist) {
			return null;
		}
		Bitmap bm = Bitmap.createBitmap(videoBmp, 0, 0, bitWidth, bitHeight);
		mCanvas.save(Canvas.ALL_SAVE_FLAG);  
		mCanvas.restore();  

		String filePath1 = MainApplication.getInstance().videoPath + File.separator +Utils.getNowTime("yyyy-MM-dd");
		File imageFilePath1 = new File(filePath1);
		if(!imageFilePath1.exists()){
			imageFilePath1.mkdir();
		} 

		String filePath2 = filePath1 + File.separator +"thumbnails";
		File imageFilePath2 = new File(filePath2);
		if(!imageFilePath2.exists()){
			imageFilePath2.mkdir();
		}

		String thumbnailName = Utils.getNowTime("yyyyMMddhhmmss");
		String imageFile = filePath2 + File.separator + thumbnailName + ".jpg";
		File file = new File(imageFile);
		FileOutputStream fos = null;  
		try {  
			fos = new FileOutputStream(file);
			bm.compress(Bitmap.CompressFormat.JPEG, 100, fos);  
		} catch (FileNotFoundException e) {  
			e.printStackTrace(); 
			return null;
		} 
		return thumbnailName;
	}
}
