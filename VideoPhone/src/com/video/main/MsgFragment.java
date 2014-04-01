package com.video.main;

import java.util.ArrayList;
import java.util.List;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.TextView;

import com.video.R;
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
		
		initView();
		initData();
		initViewPageView();
	}
	
	private void initView() {
		viewpage_alert = (TextView)mView.findViewById(R.id.tv_vp_alert);
		viewpage_system = (TextView)mView.findViewById(R.id.tv_vp_system);
		mViewPager = (ViewPager)mView.findViewById(R.id.msg_viewpager);
		
		viewpage_alert.setOnClickListener(this);
		viewpage_system.setOnClickListener(this);
		mViewPager.setOnPageChangeListener(this);
	}
	
	private void initData() {
		pageList = new ArrayList<View>();
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
		}
	}
	
	/**
	 * 初始化该界面下要滑动的页面
	 */
	private void initViewPageView() {
		LayoutInflater inflater = LayoutInflater.from(mActivity);
		alert_page = inflater.inflate(R.layout.msg_alert, null);
		system_page = inflater.inflate(R.layout.msg_system, null);
		pageList.add(alert_page);
		pageList.add(system_page);
		mViewPager.setAdapter(new ViewPagerAdapter(pageList));
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
