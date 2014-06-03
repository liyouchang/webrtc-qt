package com.video.utils;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.widget.ProgressBar;

import com.video.R;

public class TextProgressBar extends ProgressBar {
	Paint paint;
	String text;

	public TextProgressBar(Context context) {
		super(context);
		initText();
	}

	public TextProgressBar(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		initText();
	}

	public TextProgressBar(Context context, AttributeSet attrs) {
		super(context, attrs);
		initText();
	}

	@Override
	public synchronized void setProgress(int progress) {
		super.setProgress(progress);
	}

	@Override
	protected synchronized void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		Rect rect = new Rect();
		this.paint.getTextBounds(this.text, 0, this.text.length(), rect);
		int x = (getWidth() / 2) - rect.centerX();
		int y = (getHeight() / 2) - rect.centerY();
		canvas.drawText(this.text, x, y, this.paint);
	}

	private void initText() {
		paint = new Paint();
		paint.setColor(R.color.light_black);
		paint.setTextSize(20);
	}

	public synchronized void setText(String text) {
		this.text = text;
	}
}
