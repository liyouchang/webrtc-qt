package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONException;
import org.json.JSONObject;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.main.PullToRefreshView.OnFooterRefreshListener;
import com.video.main.PullToRefreshView.OnHeaderRefreshListener;
import com.video.play.RemoteFilePlayerActivity;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

@SuppressLint("HandlerLeak")
public class TerminalVideoListActivity extends Activity implements
		OnClickListener, OnHeaderRefreshListener, OnFooterRefreshListener {

	private Context mContext;
	private PreferData preferData = null;

	private String startDateJson = "";
	private String endDateJson = "";
	private String dealerName = "";
	private int indexNum = 0;
	private int totalNum = 0;

	private ArrayList<HashMap<String, String>> fileList = null;
	private FileListItemAdapter fileAdapter;
	private ListView lv_list;
	
	private PullToRefreshView mPullToRefreshView;
	private String file_refresh_time = null;
	private String file_refresh_terminal = null;
	
	private Dialog mDialog = null;
	private final int IS_REQUSTING = 1;
	private final int REQUST_TIMEOUT = 2;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.terminal_video_list);

		initView();
		initData();
	}

	private void initView() {
		lv_list = (ListView) this.findViewById(R.id.terminal_video_file_list);
		lv_list.setOnItemClickListener(new OnItemClickListenerImpl());

		ImageButton back = (ImageButton) this.findViewById(R.id.ib_terminal_video_file_back);
		back.setOnClickListener(this);

		mPullToRefreshView = (PullToRefreshView) this.findViewById(R.id.main_pull_refresh_view);
		mPullToRefreshView.setOnHeaderRefreshListener(this);
		mPullToRefreshView.setOnFooterRefreshListener(this);
	}

	@SuppressWarnings("unchecked")
	private void initData() {
		mContext = TerminalVideoListActivity.this;
		ZmqHandler.mHandler = handler;
		preferData = new PreferData(mContext);

		Intent intent = this.getIntent();
		if (intent != null) {
			fileList = (ArrayList<HashMap<String, String>>) intent.getSerializableExtra("fileList");
			dealerName = (String) intent.getCharSequenceExtra("dealerName");
			startDateJson = (String) intent.getCharSequenceExtra("startDateJson");
			endDateJson = (String) intent.getCharSequenceExtra("endDateJson");
			indexNum = fileList.size();
			totalNum = intent.getIntExtra("totalNum", 0);
		}

		if (fileList != null) {
			fileAdapter = new FileListItemAdapter(mContext, fileList);
			lv_list.setAdapter(fileAdapter);
		}
		
		// 初始化下拉刷新的显示
		if (preferData.isExist("fileRefreshTime")) {
			file_refresh_time = preferData.readString("fileRefreshTime");
		}
		if (preferData.isExist("fileRefreshTerminal")) {
			file_refresh_terminal = preferData.readString("fileRefreshTerminal");
		}
		if ((file_refresh_time != null) && (file_refresh_terminal != null)) {
			mPullToRefreshView.onHeaderRefreshComplete(file_refresh_time, file_refresh_terminal);
		}
	}
	
	/**
	 * 生成JSON的登录字符串
	 */
	private String generateTerminalVideoFileList(int offset) {
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "tunnel");
			jsonObj.put("command", "query_record");
				JSONObject subObj = new JSONObject();
				subObj.put("startTime", startDateJson);
				subObj.put("endTime", endDateJson);
				subObj.put("offset", offset);
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
	private void requestTerminalVideoFile(int offset) {
		String data = generateTerminalVideoFileList(offset);
		sendHandlerMsg(REQUST_TIMEOUT, Value.REQ_TIME_10S);
		HashMap<String, String> map = new HashMap<String, String>();
		map.put("peerId", dealerName);
		map.put("peerData", data);
		sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.send_to_peer_id, map);
	}
	
	/**
	 * 发送Handler消息
	 */
	public void sendHandlerMsg(int what) {
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
					mDialog = Utils.createLoadingDialog(mContext, getResources().getString(R.string.is_requesting_a_list_of_terminal_video));
					mDialog.show();
					break;
				case REQUST_TIMEOUT:
					if (mDialog != null) {
						if (mDialog.isShowing())
							mDialog.dismiss();
					}
					if (mPullToRefreshView.getFooterState() == PullToRefreshView.REFRESHING) {
						mPullToRefreshView.onFooterRefreshComplete();
					}
					if (handler.hasMessages(REQUST_TIMEOUT)) {
						handler.removeMessages(REQUST_TIMEOUT);
					}
					Toast.makeText(mContext, getResources().getString(R.string.request_a_list_of_terminal_video_failed), Toast.LENGTH_SHORT).show();
					break;
				case R.id.request_terminal_video_list_id:
					if (mPullToRefreshView.getFooterState() == PullToRefreshView.REFRESHING) {
						mPullToRefreshView.onFooterRefreshComplete();
					}
					if (handler.hasMessages(R.id.request_terminal_video_list_id)) {
						handler.removeMessages(R.id.request_terminal_video_list_id);
					}
					if (handler.hasMessages(REQUST_TIMEOUT)) {
						handler.removeMessages(REQUST_TIMEOUT);
					}
					totalNum = msg.arg1;
					if (totalNum == 0) {
						Toast.makeText(mContext, getResources().getString(R.string.no_video_file_during_the_period), Toast.LENGTH_LONG).show();
					} else {
						ArrayList<HashMap<String, String>> bufferList = (ArrayList<HashMap<String, String>>) msg.obj;
						if (bufferList != null) {
							fileList.addAll(indexNum, bufferList);
							fileAdapter.notifyDataSetChanged();
						} else {
							Toast.makeText(mContext,getResources().getString(R.string.requesting_terminal_video_list_error), Toast.LENGTH_LONG).show();
						}
					}
					break;
			}
		}
	};

	/**
	 * 上拖刷新
	 */
	@Override
	public void onFooterRefresh(PullToRefreshView view) {
		// TODO Auto-generated method stub
		mPullToRefreshView.postDelayed(new Runnable() {
			@Override
			public void run() {
				if (totalNum > indexNum) {
					requestTerminalVideoFile(indexNum);
				} else {
					mPullToRefreshView.onFooterRefreshComplete();
					Toast.makeText(mContext, getResources().getString(R.string.no_pictures_behind), Toast.LENGTH_LONG).show();
				}
			}
		}, 1000);
	}

	/**
	 * 下拉刷新
	 * yyyy-MM-dd hh:mm:ss 12小时制  yyyy-MM-dd HH:mm:ss 24小时制
	 */
	@Override
	public void onHeaderRefresh(PullToRefreshView view) {
		// TODO Auto-generated method stub
		mPullToRefreshView.postDelayed(new Runnable() {
			@Override
			public void run() {
				file_refresh_time = getResources().getString(R.string.last_Updated_on)+":" + Utils.getNowTime("yyyy-MM-dd HH:mm:ss");
				file_refresh_terminal = getResources().getString(R.string.terminal)+":" + Build.MODEL;
				preferData.writeData("fileRefreshTime", file_refresh_time);
				preferData.writeData("fileRefreshTerminal", file_refresh_terminal);
				mPullToRefreshView.onHeaderRefreshComplete(file_refresh_time, file_refresh_terminal);

				Toast.makeText(mContext, getResources().getString(R.string.no_pictures_front), Toast.LENGTH_LONG).show();
			}
		}, 1000);
	}
	
	private class OnItemClickListenerImpl implements OnItemClickListener {

		@Override
		public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
			Intent intent = new Intent(mContext, RemoteFilePlayerActivity.class);
			intent.putExtra("dealerName", dealerName);
			intent.putExtra("fileList", fileList);
			intent.putExtra("fileIndex", arg2);
			startActivity(intent);
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
		case R.id.ib_terminal_video_file_back:
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
			break;
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0) {
			finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.right_out);
		}
		return super.onKeyDown(keyCode, event);
	}

	private class FileListItemAdapter extends BaseAdapter {

		private Context context;
		private ArrayList<HashMap<String, String>> list;

		public FileListItemAdapter(Context context, ArrayList<HashMap<String, String>> list) {
			this.context = context;
			this.list = list;
		}

		@Override
		public int getCount() {
			return list.size();
		}

		@Override
		public Object getItem(int position) {
			return list.get(position);
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public View getView(final int position, View convertView, ViewGroup viewGroup) {
			ViewHolder holder;

			if (convertView == null) {
				convertView = LayoutInflater.from(context).inflate( R.layout.terminal_video_list_item, null);
				holder = new ViewHolder();
				convertView.setTag(holder);
				holder.file_name = (TextView) convertView .findViewById(R.id.tv_terminal_file_name);
				holder.file_size = (TextView) convertView .findViewById(R.id.tv_terminal_file_size);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			String dateString = list.get(position).get("fileDate");
			holder.file_name.setText(getResources().getString(R.string.the_end_time)+":" + dateString);
			holder.file_size.setText(getResources().getString(R.string.the_file_size)+":" + list.get(position).get("fileSize"));

			return convertView;
		}

		class ViewHolder {
			TextView file_name;
			TextView file_size;
		}
	}

}
