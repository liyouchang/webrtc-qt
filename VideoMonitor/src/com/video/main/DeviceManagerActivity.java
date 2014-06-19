package com.video.main;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.RelativeLayout;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.data.XmlDevice;
import com.video.utils.DeviceItemAdapter;

public class DeviceManagerActivity extends Activity implements OnClickListener  {

	private Context mContext;
	private XmlDevice xmlData;
	private PreferData preferData = null;
	
	private String thumbnailsPath = null;
	private static File thumbnailsFile = null;
	private static ArrayList<HashMap<String, String>> deviceList = null;
	private static DeviceItemAdapter deviceAdapter = null;
	private static ListView lv_list;
	
	//终端列表项
	private static String mDeviceName = null;
	private static String mDeviceId = null;
	private static int listPosition = 0;
	private static int listSize = 0;
	private RelativeLayout noDeviceLayout = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.device_manager);
		
		initView();
		initData();
	}
	
	private void initView() {
		ImageButton back = (ImageButton) this.findViewById(R.id.ib_device_manager_back);
		back.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = DeviceManagerActivity.this;
		xmlData = new XmlDevice(mContext);
		preferData = new PreferData(mContext);
		
		deviceList = new ArrayList<HashMap<String, String>>();
		
		//加载本地的设备列表数据
		deviceList = xmlData.readXml();
		listSize = deviceList.size();
		
		if (listSize > 0) {
			deviceAdapter = new DeviceItemAdapter(mContext, thumbnailsFile, deviceList);
			lv_list.setAdapter(deviceAdapter);
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.ib_device_manager_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
				break;
			
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
		}
		return super.onKeyDown(keyCode, event);
	}
}
