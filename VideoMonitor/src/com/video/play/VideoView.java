package com.video.play;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;

import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff.Mode;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.os.Environment;
import android.view.View;

import com.video.data.Value;
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
//    public boolean isFullScreen = true; // 是否全屏
    public boolean isDisplayView = false;
    
    //视频解码库JNI接口
    private native int initDecoder(int width, int height); 
    private native int uninitDecoder();
    private native int decodeNalu(byte[] in, int insize, byte[] out);
    
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
	 * 播放视频
	 */
	public void playVideo() {
		runFlag = true;
		isPlayVideo = true;
		TunnelCommunication.videoDataCache.clearBuffer();
		if (playVideoThread == null) {
			playVideoThread = new PlayVideoThread();
			playVideoThread.start();
			System.out.println("MyDebug: 【播放视频】");
		}
	}
	
	/**
	 * 停止播放
	 */
	public void stopVideo() {
		runFlag = false;
		isPlayVideo = false;
		TunnelCommunication.videoDataCache.clearBuffer();
		if (playVideoThread != null) {
			playVideoThread.interrupt();
			playVideoThread = null;
			System.out.println("MyDebug: 【停止播放】");
		}
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
			
			while (runFlag) {
				try{
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
						sleep(10);
					} else {
						sleep(20);
					}
					if (Value.playTerminalVideoFileFlag) {
						//终端录像
						
					} else {
						//实时录像
						if (!Value.isTunnelOpened) {
							PlayerActivity.requestPlayerTimes = 0;
							Intent intent = new Intent();
							intent.setAction(PlayerActivity.REQUEST_TIMES_ACTION);
							mContext.sendBroadcast(intent);
//							if (PlayerActivity.requestPlayerTimes < 3) {
//								if (!Value.isTunnelOpened) {
//									PlayerActivity.requestPlayerTimes ++;
//									Intent intent = new Intent();
//									intent.setAction(PlayerActivity.PLAYER_BROADCAST_ACTION);
//									mContext.sendBroadcast(intent);
//									sleep(10000);
//									System.out.println("MyDebug: 开始广播视频");
//								}
//							} else {
//								if (!Value.isTunnelOpened) {
//									PlayerActivity.requestPlayerTimes = 0;
//									Intent intent = new Intent();
//									intent.setAction(PlayerActivity.REQUEST_TIMES_ACTION);
//									mContext.sendBroadcast(intent);
//									System.out.println("MyDebug: 停止广播视频");
//								}
//							}
							sleep(2000);
						}
					}
				}catch(Exception ex){
					ex.printStackTrace();
					uninitView();
					initView();
				}
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

			if (videoType == 0) {// D1
				bitWidth = 704;
				bitHeight = 576;
			} else if (videoType == 1) {// QCF
				bitWidth = 176;
				bitHeight = 144;
			} else if (videoType == 2) {// CIF
				bitWidth = 352;
				bitHeight = 288;
			} else if (videoType == 3) {// HD1
				bitWidth = 704;
				bitHeight = 288;
			} else if (videoType == 7) {// VGA
				bitWidth = 640;
				bitHeight = 480;
			} else if (videoType == 10) {// 720P
				bitWidth = 1280;
				bitHeight = 720;
			} else if (videoType == 4) {// QVGA
				bitWidth = 320;
				bitHeight = 240;
			}
//			System.out.println("MyDebug: videoType: "+videoType);

			srcRect.left = 0;
			srcRect.top = 0;
			srcRect.right = bitWidth;
			srcRect.bottom = bitHeight;
			
			if (mContext.getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
				dstRect.left = 0;
				dstRect.top = 0;
				dstRect.right = drawWidth;
				dstRect.bottom = drawHeight;
			} else {
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
			if (mCanvas == null) {
				mCanvas = canvas;
			}
			if (Value.playTerminalVideoFileFlag) {
				//终端录像
				
			} else {
				// 清空画布
				if ((!runFlag) || (!Value.isTunnelOpened)) {
					Paint paint = new Paint();
					paint.setXfermode(new PorterDuffXfermode(Mode.CLEAR));
					canvas.drawBitmap(videoBmp, srcRect, dstRect, paint);
					paint.setXfermode(new PorterDuffXfermode(Mode.SRC));
				} else {
					if (!isDisplayView) {
						isDisplayView = true;
						Thread.sleep(3000);
						Paint paint = new Paint();
						paint.setXfermode(new PorterDuffXfermode(Mode.CLEAR));
						canvas.drawBitmap(videoBmp, srcRect, dstRect, paint);
						paint.setXfermode(new PorterDuffXfermode(Mode.SRC));
					}
				}
			}
		} catch (Exception e) {
			System.out.println("MyDebug: 绘制图像异常！");
			e.printStackTrace();
		}
	}
	
	/**
	 * 抓拍图片
	 */
	public boolean captureVideo() {
		if (mCanvas == null || !Utils.checkSDCard()) {
			return false;
		}
		Bitmap bm = Bitmap.createBitmap(videoBmp, 0, 0, bitWidth, bitHeight);
		mCanvas.save(Canvas.ALL_SAVE_FLAG);  
		mCanvas.restore();  
		
		String SDPath = Environment.getExternalStorageDirectory().getAbsolutePath();
		
		String filePath1 = SDPath + File.separator + "KaerVideo";
		File imageFilePath1 = new File(filePath1);
		if(!imageFilePath1.exists()){
			imageFilePath1.mkdir();
		} 
		
		String filePath2 = filePath1 + File.separator + "image";
		File imageFilePath2 = new File(filePath2);
		if(!imageFilePath2.exists()){
			imageFilePath2.mkdir();
		} 
		
		String filePath3 = filePath2 + File.separator +Utils.getNowTime("yyyy-MM-dd");
		File imageFilePath3 = new File(filePath3);
		if(!imageFilePath3.exists()){
			imageFilePath3.mkdir();
		} 
		
		String imageFile = filePath3 + File.separator +Utils.getNowTime("yyyyMMddhhmmss") + ".jpg";
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
		if (mCanvas == null || !Utils.checkSDCard()) {
			return null;
		}
		Bitmap bm = Bitmap.createBitmap(videoBmp, 0, 0, bitWidth, bitHeight);
		mCanvas.save(Canvas.ALL_SAVE_FLAG);  
		mCanvas.restore();  
		
		String SDPath = Environment.getExternalStorageDirectory().getAbsolutePath();
		
		String filePath1 = SDPath + File.separator + "KaerVideo";
		File imageFilePath1 = new File(filePath1);
		if(!imageFilePath1.exists()){
			imageFilePath1.mkdir();
		} 
		
		String filePath2 = filePath1 + File.separator + "video";
		File imageFilePath2 = new File(filePath2);
		if(!imageFilePath2.exists()){
			imageFilePath2.mkdir();
		} 
		
		String filePath3 = filePath2 + File.separator +Utils.getNowTime("yyyy-MM-dd");
		File imageFilePath3 = new File(filePath3);
		if(!imageFilePath3.exists()){
			imageFilePath3.mkdir();
		} 
		
		String filePath4 = filePath3 + File.separator +"thumbnails";
		File imageFilePath4 = new File(filePath4);
		if(!imageFilePath4.exists()){
			imageFilePath4.mkdir();
		}
		
		String thumbnailName = Utils.getNowTime("yyyyMMddhhmmss");
		String imageFile = filePath4 + File.separator + thumbnailName + ".jpg";
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
