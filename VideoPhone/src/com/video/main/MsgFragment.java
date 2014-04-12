package com.video.main;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
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
import com.video.play.VideoPlayActivity;
import com.video.user.LoginActivity;
import com.video.utils.MessageItemAdapter;
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
	
	private static ArrayList<HashMap<String, String>> msgList = null;
	private MessageItemAdapter msgAdapter = null;
	private ListView lv_list;
	private PullToRefreshView mPullToRefreshView;
	private String msg_refresh_time = null;
	private String msg_refresh_terminal = null;
	
	private File imageCache = null;
	
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
	 * 初始化告警消息和系统消息的界面
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
		
		Button test5 = (Button)mView.findViewById(R.id.btn_test5);
		test5.setOnClickListener(this);
	}
	
	private void initData() {
		preferData = new PreferData(mActivity);
		String SD_path = Environment.getExternalStorageDirectory().getAbsolutePath();
		String filePath = SD_path + File.separator + "KaerVideo" + File.separator + "cache";
		imageCache = new File(filePath);
		if(!imageCache.exists()){
			imageCache.mkdirs();
		}
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
		//告警消息
		testMsgView();
	}
	
	private void testMsgView() {
		msgList = new ArrayList<HashMap<String, String>>();
		HashMap<String, String> item = null;
		item = getMsgItem(true, "发生移动侦测报警", "0090B0000021", "2014-04-08 15:32:10");
		msgList.add(item);
		item = getMsgItem(false, "发生人体感应报警", "0090B0000022", "2014-04-08 16:10:15");
		msgList.add(item);
		item = getMsgItem(false, "发生人体感应报警", "0090B0000023", "2014-04-08 18:46:24");
		msgList.add(item);
		if (msgList != null) {
			msgAdapter = new MessageItemAdapter(mActivity, imageCache, msgList);
			lv_list.setAdapter(msgAdapter);
		}
	}
	
	private HashMap<String, String> getMsgItem(boolean isReaded, String event, String mac, String time) {
		
		HashMap<String, String> item = new HashMap<String, String>();
		String state = "false";
		if (isReaded) {
			state = "true";
		} else {
			state = "false";
		}
		item.put("isReaded", state);
		item.put("msgEvent", "事件: "+event);
		item.put("msgMAC", "来自: "+mac);
		item.put("msgTime", time);
		return item;
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
				preferData.writeData("msgRefreshTime", msg_refresh_time);
				preferData.writeData("msgRefreshTerminal", msg_refresh_terminal);
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
				System.out.println("MyDebug: 【打开通道】");
				TunnelCommunication.getInstance().tunnelInitialize("com/video/play/TunnelCommunication");
				TunnelCommunication.getInstance().openTunnel("123456");
				break;
			case R.id.btn_test3:
				System.out.println("MyDebug: 【播放视频】");
				TunnelCommunication.getInstance().askMediaData("123456");
				startActivity(new Intent(mActivity, VideoPlayActivity.class));
				break;
			case R.id.btn_test4:
				System.out.println("MyDebug: 【关闭通道】");
				TunnelCommunication.getInstance().closeTunnel("123456");
//				TunnelCommunication.getInstance().tunnelTerminate();
				break;
			case R.id.btn_test5:
				System.out.println("MyDebug: ");
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

	@Override
	public void onDestroyView() {
		// TODO Auto-generated method stub
		super.onDestroyView();
		 //清空缓存
//        File[] files = imageCache.listFiles();
//        for(File file :files){
//            file.delete();
//        }
//        imageCache.delete();
	}

}
