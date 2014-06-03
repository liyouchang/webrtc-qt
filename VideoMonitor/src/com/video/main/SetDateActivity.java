package com.video.main;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.DatePicker.OnDateChangedListener;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

public class SetDateActivity extends Activity implements OnClickListener {

	private Context mContext;
	private EditText start_date;
	private EditText end_date;
	
	private DatePicker datePicker = null;
	private TimePicker timePicker = null;
	private MyDateClass startDate = null;
	private MyDateClass endDate = null;
	private Dialog mDialog = null;
	
	private String startDateJson = "";
	private String endDateJson = "";
	private String dealerName = "";
	
	private final int IS_REQUSTING = 1;
	private final int REQUST_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.set_date);
		
		initView();
		initData();
	}
	
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		ZmqHandler.mHandler = handler;
	}



	private void initView() {
		start_date = (EditText) this.findViewById(R.id.et_start_date);
		end_date = (EditText) this.findViewById(R.id.et_end_date);
		
		ImageButton back = (ImageButton) this.findViewById(R.id.ib_set_date_back);
		back.setOnClickListener(this);
		
		Button set_start_date = (Button) this.findViewById(R.id.btn_set_start_date);
		set_start_date.setOnClickListener(this);
		
		Button set_end_date = (Button) this.findViewById(R.id.btn_set_end_date);
		set_end_date.setOnClickListener(this);
		
		Button submit = (Button) this.findViewById(R.id.btn_set_date_submit);
		submit.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = SetDateActivity.this;
		
		Intent intent = this.getIntent();
		if (intent != null) {
			dealerName = (String) intent.getCharSequenceExtra("dealerName");
		}
	}
	
	private class MyDateClass {
		public int mYear = 0;
		public int mDay = 0;
		public int mMonth = 0;
		public int mHour = 0;
		public int mMinute = 0;
	}
	
	/**
	 * 设置显示的日期和时间
	 */
	private void showDateTimeDialog(String dateTitle, final EditText dateView, final MyDateClass dateClass) {
		final AlertDialog dateDialog = new AlertDialog.Builder(mContext).create();
		dateDialog.show();
		Window datewindow = dateDialog.getWindow();
		datewindow.setContentView(R.layout.date_time_self);
		
		TextView title = (TextView) datewindow.findViewById(R.id.set_date_title);
		title.setText(dateTitle);
		datePicker = (DatePicker) datewindow.findViewById(R.id.datepicker);
		timePicker = (TimePicker) datewindow.findViewById(R.id.timepicker);
		
		Calendar calendar = Calendar.getInstance();
		MyDateClass initDate = new MyDateClass();
		initDate.mYear = calendar.get(Calendar.YEAR);
		initDate.mMonth = calendar.get(Calendar.MONTH);
		initDate.mDay = calendar.get(Calendar.DAY_OF_MONTH);
		initDate.mHour = calendar.get(Calendar.HOUR_OF_DAY);
		initDate.mMinute = calendar.get(Calendar.MINUTE);
		
		// 日期
		datePicker.init(initDate.mYear, initDate.mMonth, initDate.mDay, new OnDateChangedListener() {
			public void onDateChanged(DatePicker arg0, int arg1, int arg2, int arg3) {}
		});
		
		// 时间
		timePicker.setIs24HourView(true);
		timePicker.setCurrentHour(initDate.mHour);
		timePicker.setCurrentMinute(initDate.mMinute);

		Button ok = (Button) datewindow.findViewById(R.id.btn_date_time_ok);
		ok.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				dateDialog.cancel();
				dateClass.mYear = datePicker.getYear();
				dateClass.mDay = datePicker.getDayOfMonth();
				dateClass.mMonth = datePicker.getMonth() + 1;
				dateClass.mHour = timePicker.getCurrentHour();
				dateClass.mMinute = timePicker.getCurrentMinute();
				DecimalFormat df = new DecimalFormat("00");
				dateView.setText(dateClass.mYear + "-"
						+ df.format(dateClass.mMonth) + "-"
						+ df.format(dateClass.mDay) + "  "
						+ df.format(dateClass.mHour) + ":"
						+ df.format(dateClass.mMinute));
			}
		});
		Button cancel = (Button) datewindow.findViewById(R.id.btn_date_time_cancel);
		cancel.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				dateDialog.cancel();
			}
		});
	}
	
	/**
	 * 生成JSON的登录字符串
	 */
	private String generateTerminalVideoFileList() {
		JSONObject jsonObj = new JSONObject();
		DecimalFormat df = new DecimalFormat("00");
		startDateJson = ""+startDate.mYear + df.format(startDate.mMonth)
				+ df.format(startDate.mDay) + df.format(startDate.mHour) + df.format(startDate.mMinute) + "00";
		endDateJson = ""+endDate.mYear + df.format(endDate.mMonth)
				+ df.format(endDate.mDay) + df.format(endDate.mHour) + df.format(endDate.mMinute) + "00";
		try {
			jsonObj.put("type", "tunnel");
			jsonObj.put("command", "query_record");
				JSONObject subObj = new JSONObject();
				subObj.put("startTime", startDateJson);
				subObj.put("endTime", endDateJson);
				subObj.put("offset", 0);
				subObj.put("toQuery", 10);
			jsonObj.put("condition", subObj);
			return jsonObj.toString();
		} catch (JSONException e) {
			System.out.println("MyDebug: generateTerminalVideoFileList()异常！");
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 请求终端录像文件列表
	 */
	private void requestTerminalVideoFile() {
		if (checkDateTimeVilidity()) {
			Handler sendHandler = ZmqThread.zmqThreadHandler;
			String data = generateTerminalVideoFileList();
			sendHandlerMsg(IS_REQUSTING);
			sendHandlerMsg(REQUST_TIMEOUT, Value.requestTimeout);
			HashMap<String, String> map = new HashMap<String, String>();
			map.put("peerId", dealerName);
			map.put("peerData", data);
			sendHandlerMsg(sendHandler, R.id.send_to_peer_id, map);
		}
	}
	
	/**
	 * 发送Handler消息
	 */
	private void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	private void sendHandlerMsg(int what, int timeout) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessageDelayed(msg, timeout);
	}
	private void sendHandlerMsg(Handler handler, int what, HashMap<String, String> obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	private Handler handler = new Handler() {

		@SuppressWarnings("unchecked")
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_REQUSTING:
					mDialog = Utils.createLoadingDialog(mContext, "正在请求终端录像列表...");
					mDialog.show();
					break;
				case REQUST_TIMEOUT:
					if (mDialog != null)
						mDialog.dismiss();
					if (handler.hasMessages(REQUST_TIMEOUT)) {
						handler.removeMessages(REQUST_TIMEOUT);
					}
					Toast.makeText(mContext, "请求终端录像列表超时，请重试！ ", Toast.LENGTH_SHORT).show();
					break;
				case R.id.request_terminal_video_list_id:
					if (mDialog != null)
						mDialog.dismiss();
					if (handler.hasMessages(R.id.request_terminal_video_list_id)) {
						handler.removeMessages(R.id.request_terminal_video_list_id);
					}
					if (handler.hasMessages(REQUST_TIMEOUT)) {
						handler.removeMessages(REQUST_TIMEOUT);
					}
					int totalNum = msg.arg1;
					if (totalNum == 0) {
						Toast.makeText(mContext, "您请求的这个时间段没有录像文件！ ", Toast.LENGTH_LONG).show();
					} else {
						ArrayList<HashMap<String, String>> fileList = (ArrayList<HashMap<String, String>>) msg.obj;
						if (fileList != null) {
							Intent intent = new Intent(mContext,TerminalVideoListActivity.class);
							intent.putExtra("fileList", fileList);
							intent.putExtra("dealerName", dealerName);
							intent.putExtra("startDateJson", startDateJson);
							intent.putExtra("endDateJson", endDateJson);
							intent.putExtra("totalNum", totalNum);
							startActivity(intent);
							overridePendingTransition(R.anim.right_in, R.anim.fragment_nochange);
						} else {
							Toast.makeText(mContext, "请求终端录像列表错误，请重试！ ", Toast.LENGTH_LONG).show();
						}
					}
					break;
			}
		}
	};

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.ib_set_date_back:
				finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
				break;
			case R.id.btn_set_start_date:
				startDate = new MyDateClass();
				showDateTimeDialog("设置开始时间", start_date, startDate);
				break;
			case R.id.btn_set_end_date:
				endDate = new MyDateClass();
				showDateTimeDialog("设置结束时间", end_date, endDate);
				break;
			case R.id.btn_set_date_submit:
				requestTerminalVideoFile();
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
	
	/**
	 * 检查时间日期的合法性
	 */
	private boolean checkDateTimeVilidity() {
		
		int startDateLen = start_date.getText().toString().trim().length();
		int endDateLen = end_date.getText().toString().trim().length();
		
		if ((startDate == null) || (endDate == null) || (startDateLen < 15) || (endDateLen < 15)) {
			Toast.makeText(mContext, "请点击图标设置开始和结束时间", Toast.LENGTH_LONG).show();
			return false;
		} else {
			if (startDate.mYear > endDate.mYear) {
				Toast.makeText(mContext, "开始时间年份不能大于结束时间年份", Toast.LENGTH_LONG).show();
				return false;
			} else {
				if (startDate.mYear == endDate.mYear) {
					if (startDate.mMonth > endDate.mMonth) {
						Toast.makeText(mContext, "开始时间月份不能大于结束时间月份", Toast.LENGTH_LONG).show();
						return false;
					} else {
						if (startDate.mMonth == endDate.mMonth) {
							if (startDate.mDay > endDate.mDay) {
								Toast.makeText(mContext, "开始时间日份不能大于结束时间日份", Toast.LENGTH_LONG).show();
								return false;
							} else {
								if (startDate.mDay == endDate.mDay) {
									if (startDate.mHour > endDate.mHour) {
										Toast.makeText(mContext, "开始时间小时不能大于结束时间小时", Toast.LENGTH_LONG).show();
										return false;
									} else {
										if (startDate.mHour == endDate.mHour) {
											if (startDate.mMinute > endDate.mMinute) {
												Toast.makeText(mContext, "开始时间分钟不能大于结束时间分钟", Toast.LENGTH_LONG).show();
												return false;
											}
											else if (startDate.mMinute == endDate.mMinute) {
												Toast.makeText(mContext, "开始时间分钟不能等于结束时间分钟", Toast.LENGTH_LONG).show();
												return false;
											} else {
												return true;
											}
										} else {
											return true;
										}
									}
								} else {
									return true;
								}
							}
						} else {
							return true;
						}
					}
				} else {
					return true;
				}
			}
		}
	}
}
