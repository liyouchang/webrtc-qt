package com.video.utils;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface.OnCancelListener;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import com.video.R;

public class SetAPDialog {
	
	public Context mContext;
	private AlertDialog alertDialog;
	
	private TextView titleView;
	private Button button_ok;
	private Button button_cancel;

	public SetAPDialog(Context context) {
		this.mContext = context;
		
		alertDialog = new AlertDialog.Builder(context).create();
		alertDialog.show();
		Window window = alertDialog.getWindow();
		window.setContentView(R.layout.set_ap_dialog);
		
		titleView = (TextView) window.findViewById(R.id.title);
		button_ok = (Button) window.findViewById(R.id.btn_dialog_ok);
		button_cancel = (Button) window.findViewById(R.id.btn_dialog_cancel);
	}

	public void setTitle(int resId) {
		titleView.setText(resId);
	}

	public void setTitle(String title) {
		titleView.setText(title);
	}
	
	public boolean isShowing() {
		return alertDialog.isShowing();
	}
	
	public void setCanceledOnTouchOutside(boolean isCancelable) {
		alertDialog.setCanceledOnTouchOutside(isCancelable);
	}
	
	public void setOnCancelListener(OnCancelListener listener) {
		alertDialog.setOnCancelListener(listener);
	}

	public void setPositiveButton(String text, View.OnClickListener listener) {
		button_ok.setText(text);
		button_ok.setOnClickListener(listener);
	}
	
	public void setPositiveButtonEnabled(boolean enabled) {
		button_ok.setEnabled(enabled);
	}

	public void setNegativeButton(String text, View.OnClickListener listener) {
		button_cancel.setText(text);
		button_cancel.setOnClickListener(listener);
	}
	
	public void setNegativeButtonText(String text) {
		button_cancel.setText(text);
	}
	
	public void setNegativeButtonEnabled(boolean enabled) {
		button_cancel.setEnabled(enabled);
	}

	public void dismiss() {
		alertDialog.dismiss();
	}

}
