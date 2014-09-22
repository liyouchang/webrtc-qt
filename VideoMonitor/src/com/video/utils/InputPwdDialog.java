package com.video.utils;

import com.video.R;

import android.app.AlertDialog;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;


public class InputPwdDialog {
	
	public Context mContext;
	private AlertDialog alertDialog;
	
	private TextView titleView;
	private EditText et_input_pwd;
	private Button button_ok;
	private Button button_cancel;

	public InputPwdDialog(Context context) {
		this.mContext = context;
		
		LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View layout = inflater.inflate(R.layout.input_pwd_dialog, null);
		
		alertDialog = new AlertDialog.Builder(context).create();
		alertDialog.setView(layout);
		alertDialog.show();
		Window window = alertDialog.getWindow();
		window.setContentView(R.layout.input_pwd_dialog);
		
		titleView = (TextView) window.findViewById(R.id.title);
		et_input_pwd = (EditText) window.findViewById(R.id.et_input_pwd);
		button_ok = (Button) window.findViewById(R.id.btn_dialog_ok);
		button_cancel = (Button) window.findViewById(R.id.btn_dialog_cancel);
	}

	public void setTitle(int resId) {
		titleView.setText(resId);
	}

	public void setTitle(String title) {
		titleView.setText(title);
	}
	
	public void setCanceledOnTouchOutside(boolean isCancelable) {
		alertDialog.setCanceledOnTouchOutside(isCancelable);
	}

	public String getEditTextContent() {
		return et_input_pwd.getText().toString().trim();
	}
	
	public EditText getEditText() {
		return et_input_pwd;
	}

	public void setPositiveButton(String text, View.OnClickListener listener) {
		button_ok.setText(text);
		button_ok.setOnClickListener(listener);
	}

	public void setNegativeButton(String text, View.OnClickListener listener) {
		button_cancel.setText(text);
		button_cancel.setOnClickListener(listener);
	}

	public void dismiss() {
		alertDialog.dismiss();
	}

}
