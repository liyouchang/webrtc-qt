package com.video.main;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.RelativeLayout;

import com.video.R;
import com.video.data.XmlDevice;
import com.video.utils.DeviceManagerItemAdapter;

public class DeviceManagerActivity extends Activity implements OnClickListener  {

	private Context mContext;
	private XmlDevice xmlData;
	
	private String thumbnailsPath = null;
	private File thumbnailsFile = null;
	private ArrayList<HashMap<String, String>> deviceList = null;
	private DeviceManagerItemAdapter deviceAdapter = null;
	private ListView lv_list = null;
	private int listSize = 0;
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
		
		lv_list = (ListView) this.findViewById(R.id.lv_device_list);
		lv_list.setOnItemClickListener(new OnItemClickListenerImpl());
		
		noDeviceLayout = (RelativeLayout) this.findViewById(R.id.rl_no_device_manager_list);
	}
	
	private void initData() {
		mContext = DeviceManagerActivity.this;
		xmlData = new XmlDevice(mContext);
		
		// 缩略图保存路径
		String SD_path = Environment.getExternalStorageDirectory().getAbsolutePath();
		thumbnailsPath = SD_path + File.separator + "KaerVideo" + File.separator + "thumbnails";
		thumbnailsFile = new File(thumbnailsPath);
		if(!thumbnailsFile.exists()){
			thumbnailsFile.mkdirs();
		}
		
		//加载本地的设备列表数据
		deviceList = new ArrayList<HashMap<String, String>>();
		deviceList = xmlData.readXml();
		listSize = deviceList.size();
		if (listSize > 0) {
			noDeviceLayout.setVisibility(View.INVISIBLE);
			deviceAdapter = new DeviceManagerItemAdapter(mContext, thumbnailsFile, deviceList);
			lv_list.setAdapter(deviceAdapter);
		} else {
			noDeviceLayout.setVisibility(View.VISIBLE);
		}
	}
	
	/**
	 * 设备项ListView的点击事件
	 */
	private class OnItemClickListenerImpl implements OnItemClickListener {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, final int position, long id) {
			// TODO Auto-generated method stub
			HashMap<String, String> item = deviceList.get(position);
			String mDeviceName = item.get("deviceName");
			String mDeviceId = item.get("deviceID");
			String mDealerName = item.get("dealerName");
			
			Intent intent = new Intent(mContext, DeviceOperationActivity.class);
			intent.putExtra("deviceName", mDeviceName);
			intent.putExtra("deviceID", mDeviceId);
			if (mDealerName != null) {
				intent.putExtra("dealerName", mDealerName);
			}
			startActivityForResult(intent, 0);
			overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
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
