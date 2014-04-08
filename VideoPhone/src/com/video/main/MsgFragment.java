package com.video.main;

import java.util.ArrayList;
import java.util.List;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import com.video.R;
import com.video.data.PreferData;
import com.video.main.PullToRefreshView.OnFooterRefreshListener;
import com.video.main.PullToRefreshView.OnHeaderRefreshListener;
import com.video.play.TunnelCommunication;
import com.video.user.LoginActivity;
import com.video.utils.Utils;
import com.video.utils.ViewPagerAdapter;

public class MsgFragment extends Fragment implements OnClickListener, OnPageChangeListener, OnHeaderRefreshListener, OnFooterRefreshListener {

	private FragmentActivity mActivity;
	private View mView;
	private PreferData preferData = null;
	private TextView viewpage_alert;
	private TextView viewpage_system;
	
	private ViewPager mViewPager;
	private List<View> pageList;
	private View alert_page;
	private View system_page;
	
	private ListView lv_list;
	private PullToRefreshView mPullToRefreshView;
	private String msg_refresh_time = null;
	private String msg_refresh_terminal = null;
	
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
		
		//告警消息
		lv_list = (ListView) mView.findViewById(R.id.msg_list);
		mPullToRefreshView = (PullToRefreshView) mView.findViewById(R.id.main_pull_refresh_view);
		mPullToRefreshView.setOnHeaderRefreshListener(this);
        mPullToRefreshView.setOnFooterRefreshListener(this);
        
        //系统消息
		Button test1 = (Button)mView.findViewById(R.id.btn_test1);
		test1.setOnClickListener(this);
		
		Button test2 = (Button)mView.findViewById(R.id.btn_test2);
		test2.setOnClickListener(this);
		
		Button test3 = (Button)mView.findViewById(R.id.btn_test3);
		test3.setOnClickListener(this);
		
		Button test4 = (Button)mView.findViewById(R.id.btn_test4);
		test4.setOnClickListener(this);
	}
	
	private void initData() {
		preferData = new PreferData(mActivity);
		//初始化下拉刷新的显示
		if (preferData.isExist("msgRefreshTime")) {
			msg_refresh_time = preferData.readString("msgRefreshTime");
		}
		if (preferData.isExist("msgRefreshTerminal")) {
			msg_refresh_terminal = preferData.readString("msgRefreshTerminal");
		}
		if ((msg_refresh_time != null) && (msg_refresh_terminal != null)) {
			mPullToRefreshView.onHeaderRefreshComplete(msg_refresh_time, msg_refresh_terminal);
		}
	}
	
	/**
	 * 上拖刷新
	 */
	@Override
	public void onFooterRefresh(PullToRefreshView view) {
		// TODO Auto-generated method stub
		mPullToRefreshView.postDelayed(new Runnable() {
			@Override
			public void run() {
				mPullToRefreshView.onFooterRefreshComplete();
			}
		}, 1000);
	}
	
	/**
	 * 下拉刷新
	 */
	@Override
	public void onHeaderRefresh(PullToRefreshView view) {
		mPullToRefreshView.postDelayed(new Runnable() {
			@Override
			public void run() {
				msg_refresh_time = "上次更新于: "+Utils.getNowTime("yyyy-MM-dd hh:mm:ss");
				msg_refresh_terminal = "终端: "+Build.MODEL;
				preferData.writeData("listRefreshTime", msg_refresh_time);
				preferData.writeData("listRefreshTerminal", msg_refresh_terminal);
				mPullToRefreshView.onHeaderRefreshComplete(msg_refresh_time, msg_refresh_terminal);
			}
		}, 1500);
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
//				TunnelCommunication.tunnelTerminate();	
				break;
			case R.id.btn_test4:
				TunnelCommunication.closeTunnel("123456");				
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
