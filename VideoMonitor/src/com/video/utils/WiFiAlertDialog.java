package com.video.utils;

import java.util.ArrayList;
import java.util.HashMap;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.video.R;
import com.video.main.WiFiActivity;
import com.video.play.PlayerActivity;

public class WiFiAlertDialog {
	
	public Context mContext;
	private AlertDialog alertDialog;
	
	private TextView titleView;
	private ListView wifiList;

	public WiFiAlertDialog(Context context) {
		this.mContext = context;
		
		alertDialog = new AlertDialog.Builder(context).create();
		alertDialog.show();
		Window window = alertDialog.getWindow();
		window.setContentView(R.layout.wifi_alertdialog);
		
		titleView = (TextView) window.findViewById(R.id.title);
		wifiList = (ListView) window.findViewById(R.id.wifi_list);
	}
	
	public boolean isShowing() {
		return alertDialog.isShowing();
	}
	
	public void setCancelable(boolean isCancelable) {
		alertDialog.setCancelable(isCancelable);
	}

	public void setTitle(int resId) {
		titleView.setText(resId);
	}

	public void setTitle(String title) {
		titleView.setText(title);
	}

	public void setAdapter(ListAdapter adapter) {
		wifiList.setAdapter(adapter);
	}
	
	public void setOnItemClickListenerEditText(final View et_ssid, final ArrayList<HashMap<String, Object>> list) {
		final EditText editText = (EditText) et_ssid;
		wifiList.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				editText.setText((CharSequence) list.get(position).get("WiFiSSID"));
				WiFiActivity.selectedWiFi = list.get(position);
				dismiss();
			}
		});
	}
	
	public void setOnItemClickListenerLocalDevice(final View et_ssid, final ArrayList<String> list) {
		final EditText editText = (EditText) et_ssid;
		wifiList.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				editText.setText((CharSequence) list.get(position));
				dismiss();
			}
		});
	}
	
	public void setOnItemClickListenerLocalDevice(final ArrayList<HashMap<String, String>> list) {
		wifiList.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				//实时视频
				Intent intent = new Intent(mContext, PlayerActivity.class);
				intent.putExtra("deviceName", list.get(position).get("MAC"));
				intent.putExtra("dealerName", list.get(position).get("MAC"));
				intent.putExtra("isLocalDevice", true);
				intent.putExtra("localDeviceIPandPort", list.get(position).get("IP")+":"+list.get(position).get("Port"));
				mContext.startActivity(intent);
				dismiss();
			}
		});
	}
	
	public void setOnItemClickListenerButton(final View tv_name, final ArrayList<HashMap<String, String>> list) {
		final Button button = (Button) tv_name;
		wifiList.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				button.setText((CharSequence) list.get(position).get("deviceName"));
				WiFiActivity.onlineDealerName = ""+list.get(position).get("dealerName");
				dismiss();
			}
		});
	}

	public void dismiss() {
		alertDialog.dismiss();
	}

}
