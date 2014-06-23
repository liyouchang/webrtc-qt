package com.video.local.player;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.media.AudioManager;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import com.video.R;

public class LocalSoundView extends View{
	
	private Context mContext;
	private Bitmap bm , bm1;//可调节声音的小方格图片
	private int bitmapWidth , bitmapHeight;// 可调节声音的小方格图片宽度、高度 
	private int index;//第几个可调节声音的小方格图片
	private OnVolumeChangedListener mOnVolumeChangedListener;//声音改变监听
	
	private final static int HEIGHT = 11;//可调节声音的小方格高度 
	public final static int MY_HEIGHT = 163;//可调节声音区域整体高度
	public final static int MY_WIDTH = 44;//可调节声音区域整体宽度
	
	/**
	 * 改变声音大小(接口)
	 */
	public interface OnVolumeChangedListener{
		public void setYourVolume(int index);
	}
	
	public void setOnVolumeChangeListener(OnVolumeChangedListener l){
		mOnVolumeChangedListener = l;
	}
	
	public LocalSoundView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		mContext = context;
		// TODO Auto-generated constructor stub
		init();
	}

	public LocalSoundView(Context context, AttributeSet attrs) {
		super(context, attrs);
		mContext = context;
		// TODO Auto-generated constructor stub
		init();
	}

	public LocalSoundView(Context context) {
		super(context);
		mContext = context;
		// TODO Auto-generated constructor stub
		init();
	}

	private void init(){//初始化
		bm = BitmapFactory.decodeResource(mContext.getResources(), R.drawable.sound_line);
		bm1 = BitmapFactory.decodeResource(mContext.getResources(), R.drawable.sound_line1);
		bitmapWidth = bm.getWidth();
		bitmapHeight = bm.getHeight();
		//setIndex(5);
		AudioManager am = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
		setIndex(am.getStreamVolume(AudioManager.STREAM_MUSIC));//设置当前音量
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {//实现该方法来处理触屏事件
		// TODO Auto-generated method stub
		
		int y = (int) event.getY();
		int n = y * 15 / MY_HEIGHT;
		setIndex(15-n);
		
		return true;
	}

	@Override
	protected void onDraw(Canvas canvas) {//绘制
		// TODO Auto-generated method stub

		int reverseIndex = 15 - index;
		for(int i = 0;i!=reverseIndex;++i){
			canvas.drawBitmap(bm1, new Rect(0,0,bitmapWidth,bitmapHeight), 
					new Rect(0,i*HEIGHT,bitmapWidth,i*HEIGHT+bitmapHeight), null);
		}
		for(int i = reverseIndex;i!=15;++i){
			canvas.drawBitmap(bm, new Rect(0,0,bitmapWidth,bitmapHeight), 
					new Rect(0,i*HEIGHT,bitmapWidth,i*HEIGHT+bitmapHeight), null);
		}
		
		super.onDraw(canvas);  
	}

	private void setIndex(int n){//设置音量
		if(n>15){
			n = 15;
		}
		else if(n<0){
			n = 0;
		}
		if(index!=n){
			index = n;
			if(mOnVolumeChangedListener!=null){
				mOnVolumeChangedListener.setYourVolume(n);
			}
		}
		invalidate();//View本身调用迫使view重画
	}
	
}
