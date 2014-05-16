package com.video.utils;

import android.app.AlertDialog;
import android.content.Context;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import com.video.R;

public class OkOnlyDialog {
	
	public Context mContext;
	private AlertDialog alertDialog;
	
	private TextView titleView;
	private TextView messageView;
	private Button button_ok;

	public OkOnlyDialog(Context context) {
		this.mContext = context;
		
		alertDialog = new AlertDialog.Builder(context).create();
		alertDialog.show();
		Window window = alertDialog.getWindow();
		window.setContentView(R.layout.ok_only_dialog);
		
		titleView = (TextView) window.findViewById(R.id.title);
		messageView = (TextView) window.findViewById(R.id.message);
		button_ok = (Button) window.findViewById(R.id.btn_dialog_ok);
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

	public void setPositiveButton(String text, View.OnClickListener listener) {
		button_ok.setText(text);
		button_ok.setOnClickListener(listener);
	}

	public void dismiss() {
		alertDialog.dismiss();
	}

}
