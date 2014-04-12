package com.video.local;

import android.content.Context;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Gallery;

import com.video.R;
import com.video.utils.Utils;

public class DefineGallery extends Gallery {
	
	private GestureDetector gestureDetector;
	private DefineImageView handleImage;

	public DefineGallery(Context context) {
		super(context);
	}

	public DefineGallery(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	public DefineGallery(Context context, AttributeSet attrs) {
		super(context, attrs);
		gestureDetector = new GestureDetector(new MySimpleGesture());
		this.setOnTouchListener(new MyTouchListener());
	}
	
	//缩放事件
	private class MyTouchListener implements OnTouchListener {
		float baseValue;
		float originalScale;

		@Override
		public boolean onTouch(View v, MotionEvent event) {
			// TODO Auto-generated method stub
			gestureDetector.onTouchEvent(event);
			handleImage = (DefineImageView)DefineGallery.this.getSelectedView();
			switch (event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					baseValue = 0;
					originalScale = handleImage.getScale();
					break;
				case MotionEvent.ACTION_MOVE:
					if (event.getPointerCount() == 2) {
						float x = event.getX(0) - event.getX(1);
						float y = event.getY(0) - event.getY(1);
						float value = (float) Math.sqrt(x * x + y * y);
						if (baseValue == 0) {
							baseValue = value;
						} else {
							float scale = value / baseValue;
							handleImage.zoomTo(originalScale * scale, x + event.getX(1), y + event.getY(1));
						}
					}
					break;
			}
			return false;
		}
	}
	
	//缩放图片后移动图片，否则Gallery滑动
	@Override
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
		handleImage = (DefineImageView) DefineGallery.this.getSelectedView();
		float v[] = new float[9];
		Matrix matrix = handleImage.getImageMatrix();
		matrix.getValues(v);
		float left, right;
		float width, height;
		width = handleImage.getScale() * handleImage.getImageWidth();
		height = handleImage.getScale() * handleImage.getImageHeight();
		
		if ((int) width <= Utils.screenWidth && (int) height <= Utils.screenHeight) {
			super.onScroll(e1, e2, distanceX, distanceY);
		} else {
			left = v[Matrix.MTRANS_X];
			right = left + width;
			Rect r = new Rect();
			handleImage.getGlobalVisibleRect(r);
			if (distanceX > 0) {//图片放大后向左滑动
				if (r.left > 0) {
					super.onScroll(e1, e2, distanceX, distanceY);
				} else if (right < Utils.screenWidth) {
					super.onScroll(e1, e2, distanceX, distanceY);
				} else {
					handleImage.postTranslate(-distanceX, -distanceY);
				}
			} else if (distanceX < 0) {//图片放大后向右滑动
				if (r.right < Utils.screenWidth) {
					super.onScroll(e1, e2, distanceX, distanceY);
				} else if (left > 0) {
					super.onScroll(e1, e2, distanceX, distanceY);
				} else {
					handleImage.postTranslate(-distanceX, -distanceY);
				}
			}
		}
		return false;
	}
	
	@Override
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
		return false;
	}

	private class MySimpleGesture extends SimpleOnGestureListener {
		
		//单击事件
		@Override
		public boolean onSingleTapConfirmed(MotionEvent e) {
			// TODO Auto-generated method stub
			if (ViewLocalImageActivity.isVisible) {
				ViewLocalImageActivity.isVisible = false;
				ViewLocalImageActivity.image_title.setVisibility(View.INVISIBLE);
				Animation animation = AnimationUtils.loadAnimation(ViewLocalImageActivity.mContext, R.anim.popupwindow_exit);  
				ViewLocalImageActivity.image_title.startAnimation(animation); 
			} else {
				ViewLocalImageActivity.isVisible = true;
				ViewLocalImageActivity.image_title.setVisibility(View.VISIBLE);
				Animation animation = AnimationUtils.loadAnimation(ViewLocalImageActivity.mContext, R.anim.popupwindow_enter);  
				ViewLocalImageActivity.image_title.startAnimation(animation); 
			}
			return true;
		}
		
		//双击事件
		public boolean onDoubleTap(MotionEvent e) {
			handleImage = (DefineImageView) DefineGallery.this.getSelectedView();
			if (handleImage.getScale() > handleImage.getScaleRate()) {
				handleImage.zoomTo(handleImage.getScaleRate(), Utils.screenWidth / 2, Utils.screenHeight / 2, 200f);
			} else {
				handleImage.zoomTo(1.0f, Utils.screenWidth / 2, Utils.screenHeight / 2, 200f);
			}
			return true;
		}
	}
}
