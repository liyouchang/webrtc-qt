package com.video.utils;

import android.app.AlertDialog;
import android.content.Context;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.video.R;

public class DownloadAlertDialogProgressBar {
	
	public Context mContext;
	private AlertDialog alertDialog;
	
	private TextView titleView;
	private TextView messageView;
	
	private ProgressBar pb_percent_view;
    private TextView tv_percent_view;
    
	private Button button_cancel;

	public DownloadAlertDialogProgressBar(Context context) {
		this.mContext = context;
		
		alertDialog = new AlertDialog.Builder(context).create();
		alertDialog.show();
		Window window = alertDialog.getWindow();
		window.setContentView(R.layout.download_alert_dialog_progressbar);
		
		titleView = (TextView) window.findViewById(R.id.title);
		messageView = (TextView) window.findViewById(R.id.message);
		button_cancel = (Button) window.findViewById(R.id.btn_dialog_cancel);
		
		pb_percent_view = (ProgressBar) window.findViewById(R.id.pb_percent);
		tv_percent_view = (TextView) window.findViewById(R.id.tv_percent);
	}

	public void setTitle(int resId) {
		titleView.setText(resId);
	}

	public void setTitle(String title) {
		titleView.setText(title);
	}

	public void setMessage(int resId) {
		messageView.setText(resId);
	}

	public void setMessage(String message) {
		messageView.setText(message);
	}
	
	public void setProgressPercent(int percent) {
		pb_percent_view.setProgress(percent);
		tv_percent_view.setText(percent+"%");
	}

	public void setNegativeButton(String text, View.OnClickListener listener) {
		button_cancel.setText(text);
		button_cancel.setOnClickListener(listener);
	}

	public void dismiss() {
		alertDialog.dismiss();
	}

}
