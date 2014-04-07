package com.video.main;

import java.util.ArrayList;
import java.util.List;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import com.video.R;
import com.video.play.TunnelCommunication;
import com.video.user.LoginActivity;
import com.video.utils.ViewPagerAdapter;

public class MsgFragment extends Fragment implements OnClickListener, OnPageChangeListener  {

	private FragmentActivity mActivity;
	private View mView;
	
	private TextView viewpage_alert;
	private TextView viewpage_system;
	
	private ViewPager mViewPager;
	private List<View> pageList;
	private View alert_page;
	private View system_page;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		return inflater.inflate(R.layout.msg, container, false);
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		
		mActivity = getActivity();
		mView = getView();
		
		initViewPageView();
		initView();
		initData();
	}
	
	/**
	 * 初始化该界面下要滑动的页面
	 */
	private void initViewPageView() {
		mViewPager = (ViewPager)mView.findViewById(R.id.msg_viewpager);
		mViewPager.setOnPageChangeListener(this);
		LayoutInflater inflater = LayoutInflater.from(mActivity);
		alert_page = inflater.inflate(R.layout.msg_alert, null);
		system_page = inflater.inflate(R.layout.msg_system, null);
		pageList = new ArrayList<View>();
		pageList.add(alert_page);
		pageList.add(system_page);
		mViewPager.setAdapter(new ViewPagerAdapter(pageList));
	}
	
	private void initView() {
		viewpage_alert = (TextView)mView.findViewById(R.id.tv_vp_alert);
		viewpage_system = (TextView)mView.findViewById(R.id.tv_vp_system);
		viewpage_alert.setOnClickListener(this);
		viewpage_system.setOnClickListener(this);
		
		Button test1 = (Button)mView.findViewById(R.id.btn_test1);
		test1.setOnClickListener(this);
		
		Button test2 = (Button)mView.findViewById(R.id.btn_test2);
		test2.setOnClickListener(this);
		
		Button test3 = (Button)mView.findViewById(R.id.btn_test3);
		test3.setOnClickListener(this);
	}
	
	private void initData() {
		
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.tv_vp_alert:
				viewpage_alert.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_system.setBackgroundResource(R.drawable.viewpage_unselected);
				mViewPager.setCurrentItem(0);
				break;
			case R.id.tv_vp_system:
				viewpage_system.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_alert.setBackgroundResource(R.drawable.viewpage_unselected);
				mViewPager.setCurrentItem(1);
				break;
			case R.id.btn_test1:
				startActivity(new Intent(mActivity, LoginActivity.class));
				break;
			case R.id.btn_test2:
				TunnelCommunication.tunnelInitialize("com/video/play/TunnelCommunication");
				break;
			case R.id.btn_test3:
				TunnelCommunication.openTunnel("123456");
//				TunnelCommunication.askMediaData("123456");
//				TunnelCommunication.closeTunnel("123456");
//				TunnelCommunication.tunnelTerminate();	
				break;
			case R.id.btn_test4:
				
				break;
		}
	}
	
	@Override
	public void onPageScrollStateChanged(int arg0) {
		// TODO Auto-generated method stub
	}
	@Override
	public void onPageScrolled(int arg0, float arg1, int arg2) {
		// TODO Auto-generated method stub
	}
	@Override
	public void onPageSelected(int arg0) {
		// TODO Auto-generated method stub
		switch (arg0) {
			case 0:
				viewpage_alert.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_system.setBackgroundResource(R.drawable.viewpage_unselected);
				break;
			case 1:
				viewpage_system.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_alert.setBackgroundResource(R.drawable.viewpage_unselected);
				break;
		}
	}
}
