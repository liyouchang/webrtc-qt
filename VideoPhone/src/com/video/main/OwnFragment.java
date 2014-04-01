package com.video.main;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager.LayoutParams;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.PopupWindow;

import com.qrcode.view.CaptureActivity;
import com.video.R;
import com.video.user.LoginActivity;
import com.video.user.RegisterActivity;
import com.video.utils.PopupWindowAdapter;

public class OwnFragment extends Fragment implements OnClickListener, OnTouchListener {
	
	private FragmentActivity mActivity;
	private View mView;
	
	private Button button_login;
	private ImageButton button_add;
	private PopupWindow mPopupWindow;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		return inflater.inflate(R.layout.own, container, false);
	}
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		mActivity = getActivity();
		mView = getView();
		
		initView();
		initData();
		
	}
	
	private void initView () {
		button_login = (Button)mView.findViewById(R.id.btn_test_login);
		button_login.setOnClickListener(this);
		button_login.setOnTouchListener(this);
		button_add = (ImageButton)mView.findViewById(R.id.btn_add_device);
		button_add.setOnClickListener(this);
		button_add.setOnTouchListener(this);
		Button button_register = (Button)mView.findViewById(R.id.btn_test_register);
		button_register.setOnClickListener(this);
	}
	
	private void initData() {
		
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_test_login:
				startActivity(new Intent(mActivity, LoginActivity.class));
				break;
			case R.id.btn_test_register:
				startActivity(new Intent(mActivity, RegisterActivity.class));
				break;
			case R.id.btn_add_device:
				startActivity(new Intent(mActivity, AddDeviceActivity.class));
				break;
		}
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_add_device:
				if (MotionEvent.ACTION_DOWN == event.getAction()) {
					button_add.setImageResource(R.drawable.imagebutton_add_selected);
				} else if (MotionEvent.ACTION_UP == event.getAction()) {
					button_add.setImageResource(R.drawable.imagebutton_add_unselected);
				}
				break;
		}
		return false;
	}
	
	public void showPopupWindow(View view) {
		LayoutInflater inflater = (LayoutInflater) mActivity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View pop_view = inflater.inflate(R.layout.pop_main, null);
		ListView pop_listView = (ListView)pop_view.findViewById(R.id.pop_list);
		
		List<String> item_list = new ArrayList<String>();
		item_list.add("添加设备");
		item_list.add("退出");
		PopupWindowAdapter popAdapter = new PopupWindowAdapter(mActivity, item_list);
		pop_listView.setAdapter(popAdapter);
		
		mPopupWindow = new PopupWindow(pop_view, LayoutParams.FILL_PARENT, 190, true);
		mPopupWindow.setHeight(LayoutParams.WRAP_CONTENT); 
		mPopupWindow.setBackgroundDrawable(new BitmapDrawable());
		mPopupWindow.setOutsideTouchable(true);
		
		mPopupWindow.setAnimationStyle(R.style.PopupAnimationBottom);
		mPopupWindow.showAtLocation(view, Gravity.BOTTOM, 0, 0);
		mPopupWindow.update();

		pop_listView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				switch (position) {
					case 0:
						if (mPopupWindow.isShowing()) {
							mPopupWindow.dismiss();
						}
						startActivityForResult(new Intent(mActivity, CaptureActivity.class), 0);
						break;
					case 1:
						if (mPopupWindow.isShowing()) {
							mPopupWindow.dismiss();
						}
						break;
					default : break;
				}
			}
		});
	}
}
