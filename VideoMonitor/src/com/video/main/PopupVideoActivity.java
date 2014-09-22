package com.video.main;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.PowerManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.play.PlayerActivity;
import com.video.service.MainApplication;
import com.video.utils.OkCancelDialog;
import com.video.utils.Utils;

@SuppressLint("Wakelock")
public class PopupVideoActivity extends Activity implements OnClickListener {

	private Context mContext;
	private PreferData preferData;
	
	private static String mDeviceName = null;
	private static String mDeviceId = null;
	private static String mDealerName = null;
	private static String mLinkState = null;
	private static String mPlayerClarity = null;
	
	// 读取流量保护开关设置
	boolean isProtectTraffic = true;
	
	// 电源管理器
	private PowerManager mPowerManager;
	// 唤醒锁
	private PowerManager.WakeLock mWakeLock;
	// 键盘管理器
	private KeyguardManager mKeyguardManager;
	// 键盘锁
	private KeyguardLock mKeyguardLock;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.popup_video);

		initView();
		initData();
	}

	private void initView() {
		Button btn_smartdoor_ok = (Button) this.findViewById(R.id.btn_smartdoor_ok);
		btn_smartdoor_ok.setOnClickListener(this);

		Button btn_smartdoor_cancel = (Button) this.findViewById(R.id.btn_smartdoor_cancel);
		btn_smartdoor_cancel.setOnClickListener(this);
	}

	private void initData() {
		mContext = PopupVideoActivity.this;
		if (preferData == null) {
			preferData = new PreferData(mContext);
		}
		if (preferData.isExist("ProtectTraffic")) {
			isProtectTraffic = preferData.readBoolean("ProtectTraffic");
		}
		mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
		mKeyguardManager = (KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE);
		
		Intent intent = this.getIntent();
		if (intent != null) {
			mDeviceId = (String) intent.getCharSequenceExtra("deviceID");
			int position = MainApplication.getInstance().getDeviceListPositionByDeviceID(mDeviceId);
			mDeviceName = MainApplication.getInstance().deviceList.get(position).get("deviceName");
			mDealerName = MainApplication.getInstance().deviceList.get(position).get("dealerName");
			mLinkState = MainApplication.getInstance().deviceList.get(position).get("LinkState");
			mPlayerClarity = MainApplication.getInstance().deviceList.get(position).get("playerClarity");
		}
	}

	@Override
	protected void onStart() {
		// TODO Auto-generated method stub
		super.onStart();
		// 点亮亮屏
		mWakeLock = mPowerManager.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.SCREEN_BRIGHT_WAKE_LOCK, "Tag");
		mWakeLock.acquire();
		// 键盘解锁
		mKeyguardLock = mKeyguardManager.newKeyguardLock("");
		mKeyguardLock.disableKeyguard();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		// 一定要释放唤醒锁和恢复键盘
		if (mWakeLock != null) {
			mWakeLock.release();
			mWakeLock = null;
		}
		if (mKeyguardLock != null) {
			mKeyguardLock.reenableKeyguard();
			mKeyguardLock = null;
		}
	}
	
	/**
	 * 打开实时视频播放器
	 */
	private void openVideoPlayer() {
		Intent intent = new Intent(mContext, PlayerActivity.class);
		intent.putExtra("deviceID", mDeviceId);
		intent.putExtra("deviceName", mDeviceName);
		intent.putExtra("dealerName", mDealerName);
		intent.putExtra("playerClarity", mPlayerClarity);
		startActivity(intent);
	}
	
	/**
	 * 观看视频
	 */
	private void watchVideo() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (mLinkState.equals("linked")) {
				if (!isProtectTraffic) {
					openVideoPlayer();
				} else {
					if (Utils.isWiFiNetwork(mContext)) {
						openVideoPlayer();
					} else {
						final OkCancelDialog myDialog=new OkCancelDialog(mContext);
						myDialog.setTitle("");
						myDialog.setMessage(getResources().getString(R.string.the_current_network_is_not_WiFi));
						myDialog.setPositiveButton(getResources().getString(R.string.confirm), new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog.dismiss();
								openVideoPlayer();
							}
						});
						myDialog.setNegativeButton(getResources().getString(R.string.cancel), new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog.dismiss();
							}
						});
					}
				}
			} else {
				Toast.makeText(mContext, getResources().getString(R.string.not_online), Toast.LENGTH_SHORT).show();
			}
		} else {
			Toast.makeText(mContext, getResources().getString(R.string.no_available_network_connection), Toast.LENGTH_SHORT).show();
		}
	}

	@Override
	public void onClick(View view) {
		// TODO Auto-generated method stub
		switch (view.getId()) {
		case R.id.btn_smartdoor_ok:
			watchVideo();
			finish();
			break;
		case R.id.btn_smartdoor_cancel:
			finish();
			break;
		}
	}
}
