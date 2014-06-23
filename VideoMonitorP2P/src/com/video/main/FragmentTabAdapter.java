package com.video.main;

import java.util.List;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;

public class FragmentTabAdapter implements OnTabChangeListener {

	private List<Fragment> fragments;
	private TabHost mTabHost;
	private FragmentActivity fragmentActivity;
	private int fragmentId;
	private int currentTab;

	private OnMyTabChangedListener onMyTabChangedListener;

	public FragmentTabAdapter(FragmentActivity activity, List<Fragment> list, int id, TabHost tabHost) {
		
		this.fragmentActivity = activity;
		this.fragments = list;
		this.fragmentId = id;
		this.mTabHost = tabHost;
		
		mTabHost.setOnTabChangedListener(this);

		// 默认显示第一个tab页
		FragmentTransaction ft = activity.getSupportFragmentManager().beginTransaction();
		ft.add(fragmentId, fragments.get(0));
		ft.commit();
	}

	/**
	 * 切换显示tab
	 */
	private void showTab(int index) {
		for (int i=0; i<fragments.size(); i++) {
			Fragment fragment = fragments.get(i);
			FragmentTransaction ft = obtainFragmentTransaction(index);
			if (index == i) {
				ft.show(fragment);
			} else {
				ft.hide(fragment);
			}
			ft.commit();
		}
		currentTab = index;
	}

	/**
	 * 获取一个FragmentTransaction
	 */
	private FragmentTransaction obtainFragmentTransaction(int index) {
		return fragmentActivity.getSupportFragmentManager().beginTransaction();
	}

	public int getCurrentTab() {
		return currentTab;
	}

	public Fragment getCurrentFragment() {
		return fragments.get(currentTab);
	}

	public void setOnMyTabChangedListener(
			OnMyTabChangedListener onMyTabChangedListener) {
		this.onMyTabChangedListener = onMyTabChangedListener;
	}

	public interface OnMyTabChangedListener {
		public void MyTabChanged(FragmentTabAdapter fragmentTabAdapter);
	}
	
	private int getIntId(String tabId) {
		int result = 0;
		if (tabId.equals("tab1")) {
			result = 0;
		} else if (tabId.equals("tab2")) {
			result = 1;
		} else if (tabId.equals("tab3")) {
			result = 2;
		} else if (tabId.equals("tab4")) {
			result = 3;
		}
		return result;
	}

	@Override
	public void onTabChanged(String tabId) {
		// TODO Auto-generated method stub
		if (onMyTabChangedListener != null) {
			onMyTabChangedListener.MyTabChanged(this);
		}
		for (int i=0; i<fragments.size(); i++) {
			if (getIntId(tabId) == i) {
				Fragment fragment = fragments.get(i);
				FragmentTransaction ft = obtainFragmentTransaction(i);

				getCurrentFragment().onPause(); // 暂停当前tab
				// getCurrentFragment().onStop(); // 停止当前tab

				if (fragment.isAdded()) {
					//fragment.onStart(); // 启动目标tab的onStart()
					fragment.onResume(); // 启动目标tab的onResume()
				} else {
					ft.add(fragmentId, fragment);
				}
				showTab(i); // 显示目标tab
				ft.commit();
			}
		}
	}
}