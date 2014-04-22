package com.video.play;

import java.nio.ByteBuffer;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.util.Log;
import android.view.View;

public class VideoPlayView extends View {

    private static int width = 1280; 
    private static int height = 720;
    
    private static byte[] decodeData = new byte[width*height*3];
    private static ByteBuffer videoBuffer = ByteBuffer.wrap(decodeData);
    private static Bitmap videoBmp = Bitmap.createBitmap(width, height, Config.RGB_565);
    
    private Rect srcRect = new Rect(0, 0, 0, 0) ;
    private Rect dstRect = new Rect(0, 0, 0, 0) ;
    private int drawWide=0;
    private int drawHeight=0;  
    private int videoType = 2;
    private int bitWide=352;
    private int bitHeight=288;
    
    private PlayVideoThread playVideoThread = null; //播放视频线程
    private boolean runFlag = false;
    
  //视频解码库JNI接口
    private native int initDecoder(int width, int height); 
    private native int uninitDecoder();
    private native int decodeNalu(byte[] in, int insize, byte[] out);
    
    static 
	{
        System.loadLibrary("h264");
    }
    
	public VideoPlayView(Context context) {
		super(context);
	}
	
	public void initView() {
		initDecoder(width, height);
		System.out.println("MyDebug: 【InitDecoder()】");
		if (TunnelCommunication.videoDataCache != null) {
			TunnelCommunication.videoDataCache.clearBuffer();
		}
	}
	
	public void uninitView() {
		TunnelCommunication.videoDataCache.clearBuffer();
		uninitDecoder();
		System.out.println("MyDebug: 【UninitDecoder()】");
	}
	
	/**
	 * 播放视频
	 */
	public void playVideo() {
		if (playVideoThread == null) {
			playVideoThread = new PlayVideoThread(true);
			playVideoThread.start();
			System.out.println("MyDebug: 【播放视频】");
		}
	}
	
	/**
	 * 停止播放
	 */
	public void stopVideo() {
		if (playVideoThread != null) {
			runFlag = false;
			playVideoThread = null;
			System.out.println("MyDebug: 【停止播放】");
		}
	}
	
	/**
	 * 播放视频线程
	 */
	private class PlayVideoThread extends Thread {
		
		public PlayVideoThread(boolean flag) {
			runFlag = flag;
		}

		public void run() {
			
			initView();
			int decodeLen = 0;
			byte[] naluData = new byte[100*1024];
			int naluDataLen = 0;
			
			while (runFlag) {
				try{
					naluDataLen = TunnelCommunication.videoDataCache.pop(naluData);
					if (naluDataLen > 0) {
						decodeLen = decodeNalu(naluData, naluDataLen, decodeData);
						if (decodeLen >= 0) {
							postInvalidate();
						}
					} else {
						sleep(10);
					}
				}catch(Exception ex){
					TunnelCommunication.videoDataCache.clearBuffer();
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
			drawWide = canvas.getWidth(); 
			drawHeight = canvas.getHeight();
			videoBuffer.rewind();
			videoBmp.copyPixelsFromBuffer(videoBuffer); // bmp从缓冲区获得数据

			videoType = (byte) (TunnelCommunication.videoFrameType & 0x5F);

			if (videoType == 0) {// D1
				bitWide = 704;
				bitHeight = 576;
			} else if (videoType == 1) {// QCF
				bitWide = 176;
				bitHeight = 144;
			} else if (videoType == 2) {// CIF
				bitWide = 352;
				bitHeight = 288;
			} else if (videoType == 3) {// HD1
				bitWide = 704;
				bitHeight = 288;
			} else if (videoType == 7) {// VGA
				bitWide = 640;
				bitHeight = 480;
			} else if (videoType == 10) {// 720P
				bitWide = 1280;
				bitHeight = 720;
			} else if (videoType == 4) {// QVGA
				bitWide = 320;
				bitHeight = 240;
			}

			srcRect.left = 0;
			srcRect.top = 0;
			srcRect.right = bitWide;
			srcRect.bottom = bitHeight;
			
			if (drawWide <= drawHeight) {
				dstRect.left = 0;
				dstRect.top = (int) ((drawHeight - drawWide / 1.2) / 4);
				dstRect.right = drawWide;
				dstRect.bottom = (int) (drawWide / 1.2 + (drawHeight - drawWide / 1.2) / 4);
			} else {
				dstRect.left = 0;
				dstRect.top = 0;
				dstRect.right = drawWide;
				dstRect.bottom = drawHeight;
			}

			// 绘制图像
			canvas.drawBitmap(videoBmp, srcRect, dstRect, null);
		} catch (Exception e) {
			e.printStackTrace();
			Log.d("MYDEBUG", "OnDraw exception");
		}
	}
}
