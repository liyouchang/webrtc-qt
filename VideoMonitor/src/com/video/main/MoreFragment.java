package com.video.main;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;

import com.video.R;
import com.video.data.PreferData;
import com.video.service.MainApplication;
import com.video.user.LoginActivity;
import com.video.user.ModifyPwdActivity;
import com.video.utils.OkCancelDialog;

public class MoreFragment extends Fragment implements OnClickListener {

	private View rootView;
	private FragmentActivity mActivity;
	
	private Button button_logout;
	private PreferData preferData = null;
	
	private boolean isRememberPwd = true;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		if (rootView == null) {
			rootView = inflater.inflate(R.layout.more, null);
		}
		container = (ViewGroup) rootView.getParent();
		if (container != null) {
			container.removeView(rootView);
		}
		return rootView;
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		mActivity = getActivity();
		
		initView();
		initData();
	}
	
	private void initView() {
		Button button_modify_pwd = (Button)rootView.findViewById(R.id.btn_modify_pwd);
		button_modify_pwd.setOnClickListener(this);
		
		Button button_setting = (Button)rootView.findViewById(R.id.btn_setting);
		button_setting.setOnClickListener(this);
		
//		Button button_device_manager = (Button)mView.findViewById(R.id.btn_device_manager);
//		button_device_manager.setOnClickListener(this);
		
		Button button_wifi = (Button)rootView.findViewById(R.id.btn_wifi);
		button_wifi.setOnClickListener(this);
		
		Button button_help = (Button)rootView.findViewById(R.id.btn_help);
		button_help.setOnClickListener(this);
		
		Button button_about = (Button)rootView.findViewById(R.id.btn_about);
		button_about.setOnClickListener(this);
		
		button_logout = (Button)rootView.findViewById(R.id.btn_logout);
		button_logout.setOnClickListener(this);
 	}
	
	private void initData() {
		if (preferData == null) {
			preferData = new PreferData(mActivity);
		}
	}
	
	/**
	 * 显示操作的提示
	 */
	private void showHandleDialog() {
		final OkCancelDialog myDialog=new OkCancelDialog(mActivity);
		myDialog.setTitle("温馨提示");
		myDialog.setMessage("确认退出当前账号的登录？");
		myDialog.setPositiveButton("确定", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
				exitActivityandService();
			}
		});
		myDialog.setNegativeButton("取消", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
			}
		});
	}
	
	/**
	 * 退出当前账号登录的处理
	 */
	private void exitActivityandService() {
		if (preferData.isExist("RememberPwd")) {
			isRememberPwd = preferData.readBoolean("RememberPwd");
		}
		if (!isRememberPwd) {
			if (preferData.isExist("UserPwd")) {
				preferData.deleteItem("UserPwd");
			}
			if (preferData.isExist("AutoLogin")) {
				preferData.deleteItem("AutoLogin");
			}
		}
		
		// 发送注销数据
		MainApplication.getInstance().sendLogoutData();
		
		// 终止主程序和服务广播
		MainApplication.getInstance().stopActivityandService();
		
		// 启动登录界面
		startActivity(new Intent(mActivity, LoginActivity.class));
		mActivity.finish();
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_modify_pwd:
				startActivityForResult(new Intent(mActivity, ModifyPwdActivity.class), 1);
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_setting:
				startActivity(new Intent(mActivity, SettingsActivity.class));
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
//			case R.id.btn_device_manager:
//				startActivity(new Intent(mActivity, DeviceManagerActivity.class));
//				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
//				break;
			case R.id.btn_wifi:
				startActivity(new Intent(mActivity, WiFiActivity.class));
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_help:
				startActivity(new Intent(mActivity, HelpActivity.class));
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_about:
				startActivity(new Intent(mActivity, AboutActivity.class));
				mActivity.overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
				break;
			case R.id.btn_logout:
				showHandleDialog();
				break;
		}
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		super.onActivityResult(requestCode, resultCode, data);
		if (resultCode == 1) {
			// 修改密码返回处理
			if (preferData == null) {
				preferData = new PreferData(mActivity);
			}
			if (preferData.isExist("UserPwd")) {
				preferData.deleteItem("UserPwd");
			}
			if (preferData.isExist("AutoLogin")) {
				preferData.deleteItem("AutoLogin");
			}
			// 终止主程序和服务广播
			MainApplication.getInstance().stopActivityandService();
			// 启动登录界面
			startActivity(new Intent(mActivity, LoginActivity.class));
			mActivity.finish();
		}
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
}

