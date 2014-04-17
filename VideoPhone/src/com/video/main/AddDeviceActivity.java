package com.video.main;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.view.ViewPager.LayoutParams;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.qrcode.view.CaptureActivity;
import com.video.R;
import com.video.data.PreferData;
import com.video.data.Value;
import com.video.data.XmlDevice;
import com.video.socket.ZmqHandler;
import com.video.socket.ZmqThread;
import com.video.utils.Utils;

public class AddDeviceActivity extends Activity implements OnClickListener {

	private Context mContext;
	private XmlDevice xmlData;
	private PreferData preferData = null;
	
	private ImageButton button_title_more;
	private EditText et_name;
	private EditText et_id;
	private RelativeLayout add_title;
	private ProgressDialog progressDialog;
	//终端列表项
	private String mDeviceName = null;
	private String mDeviceId = null;
	
	private String userName = "";
	private String termMac = "";
	private String termName = "";
	
	private final int IS_ADDING = 1;
	private final int ADD_TIMEOUT = 2;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.add);
		
		initView();
		initData();
	}

	private void initView() {
		add_title = (RelativeLayout) this.findViewById(R.id.add_device_title);
		button_title_more = (ImageButton) this.findViewById(R.id.btn_add_device_more);
		button_title_more.setOnClickListener(this);
		
		ImageButton button_back = (ImageButton) this.findViewById(R.id.btn_add_device_back);
		button_back.setOnClickListener(this);
		Button button_ok = (Button) this.findViewById(R.id.btn_add_device_ok);
		button_ok.setOnClickListener(this);
		
		et_id = (EditText) this.findViewById(R.id.et_add_device_id);
		et_name = (EditText) this.findViewById(R.id.et_add_device_name);
	}
	
	private void initData() {
		mContext = AddDeviceActivity.this;
		ZmqHandler.setHandler(handler);
		xmlData = new XmlDevice(mContext);
		preferData = new PreferData(mContext);
		
		if (preferData.isExist("UserName")) {
			userName = preferData.readString("UserName");
		}
	}
	
	/**
	 * 生成JSON的登录字符串
	 */
	private String generateAddDeviceJson(String username, String mac, String termname) {
		String result = "";
		JSONObject jsonObj = new JSONObject();
		try {
			jsonObj.put("type", "Client_AddTerm");
			jsonObj.put("UserName", username);
			jsonObj.put("MAC", mac);
			jsonObj.put("TermName", termname);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		result = jsonObj.toString();
		return result;
	}
	
	/**
	 * 显示操作的进度条
	 */
	private void showProgressDialog(String info) {
		progressDialog = new ProgressDialog(mContext);
        progressDialog.setMessage(info); 
        progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);  
        progressDialog.setIndeterminate(false);     
        progressDialog.setCancelable(false); 
        progressDialog.show(); 
	}
	
	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case IS_ADDING:
					showProgressDialog("正在添加... ");
					break;
				case ADD_TIMEOUT:
					if (progressDialog != null)
						progressDialog.dismiss();
					Toast.makeText(mContext, "添加终端失败，网络超时！", Toast.LENGTH_SHORT).show();
					if (handler.hasMessages(ADD_TIMEOUT)) {
						handler.removeMessages(ADD_TIMEOUT);
					}
					break;
				case R.id.add_device_id:
					if (handler.hasMessages(ADD_TIMEOUT)) {
						handler.removeMessages(ADD_TIMEOUT);
						if (progressDialog != null)
							progressDialog.dismiss();
						int resultCode = msg.arg1;
						if (resultCode == 0) {
							String dealerName = (String)msg.obj;
							xmlData.addItem(getDeviceItem(mDeviceName, mDeviceId, dealerName));
							Toast.makeText(mContext, "添加终端成功！", Toast.LENGTH_SHORT).show();
							AddDeviceActivity.this.finish();
						} else {
							Toast.makeText(mContext, "添加终端失败，"+Utils.getErrorReason(resultCode), Toast.LENGTH_SHORT).show();
						}
					} else {
						handler.removeMessages(R.id.add_device_id);
					}
					break;
			}
		}
	};
	
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
	private void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	public void clickAddDeviceEvent() {
		if (Utils.isNetworkAvailable(mContext)) {
			if (checkAddDeviceData()) {
				Handler sendHandler = ZmqThread.zmqThreadHandler;
				String data = generateAddDeviceJson(userName, termMac, termName);
				sendHandlerMsg(IS_ADDING);
				sendHandlerMsg(ADD_TIMEOUT, Value.requestTimeout);
				sendHandlerMsg(sendHandler, R.id.zmq_send_data_id, data);
			}
		} else {
			Toast.makeText(mContext, "没有可用的网络连接，请确认后重试！", Toast.LENGTH_SHORT).show();
		}
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.btn_add_device_back:
				finish();
				break;
			case R.id.btn_add_device_more:
				showPopupWindow();
				break;
			case R.id.btn_add_device_ok:
				clickAddDeviceEvent();
				break;
		}
	}
	
	/**
	 * @return true:注册信息格式正确  false:注册信息格式错误
	 */
	private boolean checkAddDeviceData() {
		boolean resultFlag = false;
		
		//获取EditText输入框的字符串
		termName = et_name.getText().toString().trim();
		termMac = et_id.getText().toString().trim();
		
		if (termName.equals("")) {
			resultFlag = false;
			et_name.setError("请输入设备名称！");
		}
		else if ((termName.length()<2) || (termName.length()>20)) {
			resultFlag = false;
			et_name.setError("设备名称长度范围2~20！");
		} else {
			resultFlag = true;
			mDeviceName = termName;
			if (termMac.equals("")) {
				resultFlag = false;
				et_id.setError("请输入设备ID，您可以通过扫描二维码或搜索设备输入设备ID！");
			}
			else if ((termMac.length()<6) || (termMac.length()>20)) {
				resultFlag = false;
				et_id.setError("设备ID长度范围6~20！");
			} else {
				resultFlag = true;
				mDeviceId = termMac;
			}
		}
		return resultFlag;
	}
	
	/**
	 * 获得一个设备项Item
	 * @param deviceName 设备名称
	 * @param deviceID 设备的MAC
	 * @return 返回一个设备项Item
	 */
	private HashMap<String, String> getDeviceItem(String deviceName, String deviceId, String dealerName) {
		HashMap<String, String> item = new HashMap<String, String>();
		item.put("isOnline", "false");
		item.put("deviceName", deviceName);
		item.put("deviceID", deviceId);
		item.put("dealerName", dealerName);
		return item;
	}
	
	/**
	 * 添加设备的弹出对话框
	 */
	private void showPopupWindow() {
		LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View pop_view = inflater.inflate(R.layout.pop_main, null);
		ListView pop_listView = (ListView)pop_view.findViewById(R.id.pop_list);
		
		List<String> item_list = new ArrayList<String>();
		item_list.add("扫描二维码");
		item_list.add("搜索设备");
		AddPopupWindowAdapter popAdapter = new AddPopupWindowAdapter(mContext, item_list);
		pop_listView.setAdapter(popAdapter);
		
		final PopupWindow mPopupWindow = new PopupWindow(pop_view, Utils.screenWidth/2, 190, true);
		mPopupWindow.setHeight(LayoutParams.WRAP_CONTENT); 
		mPopupWindow.setBackgroundDrawable(new BitmapDrawable());
		mPopupWindow.setOutsideTouchable(true);
		
		int X_position = Utils.screenWidth - mPopupWindow.getWidth() - 10;
		mPopupWindow.setAnimationStyle(R.style.PopupAnimationTop);
		mPopupWindow.showAsDropDown(add_title, X_position, 0);
		mPopupWindow.update();

		pop_listView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				switch (position) {
					case 0:
						if (mPopupWindow.isShowing()) {
							mPopupWindow.dismiss();
						}
						startActivityForResult(new Intent(AddDeviceActivity.this, CaptureActivity.class), 0);
						break;
					case 1:
						if (mPopupWindow.isShowing()) {
							mPopupWindow.dismiss();
						}
						break;
				}
			}
		});
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		if (resultCode == RESULT_OK) {
			Bundle bundle = data.getExtras();
			String qrcode_string = bundle.getString("qrcode");
			et_id.setText(qrcode_string);
		}
	}
	
	/**
	 * 添加设备的适配器
	 */
	private class AddPopupWindowAdapter extends BaseAdapter {

		private Context context;
		private List<String> list;

		public AddPopupWindowAdapter(Context context, List<String> list) {
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
		public View getView(int position, View convertView, ViewGroup viewGroup) {
			ViewHolder holder;

			if (convertView == null) {
				convertView = LayoutInflater.from(context).inflate(R.layout.pop_item, null);
				holder = new ViewHolder();
				convertView.setTag(holder);
				holder.pop_textView = (TextView) convertView.findViewById(R.id.pop_text);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			holder.pop_textView.setText(list.get(position));
			return convertView;
		}

		class ViewHolder {
			TextView pop_textView;
		}
	}
}
