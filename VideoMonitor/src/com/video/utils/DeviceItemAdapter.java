package com.video.utils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;

import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.PreferData;
import com.video.play.PlayerActivity;

public class DeviceItemAdapter extends BaseAdapter {

	private Context context;
	private ArrayList<HashMap<String, String>> list;
	private File thumbnailsFile = null;

	public DeviceItemAdapter(Context context, File thumbnailsFile, ArrayList<HashMap<String, String>> list) {
		this.context = context;
		this.list = list;
		this.thumbnailsFile = thumbnailsFile;
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
	
	private boolean getOnlineState(String state) {
		boolean result = false;
		if (state.equals("true")) {
			result = true;
		} else {
			result = false;
		}
		return result;
	}

	@Override
	public View getView(final int position, View convertView, ViewGroup viewGroup) {
		ViewHolder holder;

		if (convertView == null) {
			convertView = LayoutInflater.from(context).inflate(R.layout.device_item, null);
			holder = new ViewHolder();
			convertView.setTag(holder);
			holder.device_bg = (RelativeLayout) convertView.findViewById(R.id.rl_device_bg);
			holder.device_icon = (ImageView) convertView.findViewById(R.id.iv_device_icon);
			holder.device_net_state = (ImageView) convertView.findViewById(R.id.iv_device_net_state);
			holder.device_name = (TextView) convertView.findViewById(R.id.tv_device_name);
			holder.device_id = (TextView) convertView.findViewById(R.id.tv_device_mac);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		String path = list.get(position).get("deviceBg");
		if (!path.equals("null")) {
			AsyncImageTask task = new AsyncImageTask(holder.device_bg, path);
			task.execute();
		} else {
			holder.device_bg.setBackgroundResource(R.drawable.device_item_view_bg);
		}
		boolean termState = getOnlineState(list.get(position).get("isOnline"));
		if (termState) {
			holder.device_net_state.setBackgroundResource(R.drawable.icon_online);
		} else {
			holder.device_net_state.setBackgroundResource(R.drawable.icon_offline);
		}
		holder.device_name.setText(list.get(position).get("deviceName"));
		holder.device_id.setText(list.get(position).get("deviceID"));
		
		holder.device_icon.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				final String name = list.get(position).get("deviceName");
				if (getOnlineState(list.get(position).get("isOnline"))) {
					//读取流量保护开关设置
					boolean isProtectTraffic = true;
					PreferData preferData = new PreferData(context);
					if (preferData.isExist("ProtectTraffic")) {
						isProtectTraffic = preferData.readBoolean("ProtectTraffic");
					}
					
					if (!isProtectTraffic) {
						//实时视频
						Intent intent = new Intent(context, PlayerActivity.class);
						intent.putExtra("deviceName", name);
						intent.putExtra("dealerName", list.get(position).get("dealerName"));
						context.startActivity(intent);
					} else {
						if (Utils.isWiFiNetwork(context)) {
							//实时视频
							Intent intent = new Intent(context, PlayerActivity.class);
							intent.putExtra("deviceName", name);
							intent.putExtra("dealerName", list.get(position).get("dealerName"));
							context.startActivity(intent);
						} else {
							final MyAlertDialog myDialog=new MyAlertDialog(context);
							myDialog.setTitle("温馨提示");
							myDialog.setMessage("当前网络不是WiFi，继续观看视频？");
							myDialog.setPositiveButton("确认", new OnClickListener() {
								@Override
								public void onClick(View v) {
									myDialog.dismiss();
									//实时视频
									Intent intent = new Intent(context, PlayerActivity.class);
									intent.putExtra("deviceName", name);
									intent.putExtra("dealerName", list.get(position).get("dealerName"));
									context.startActivity(intent);
								}
							});
							myDialog.setNegativeButton("取消", new OnClickListener() {
								@Override
								public void onClick(View v) {
									myDialog.dismiss();
								}
							});
						}
					}
				} else {
					Toast.makeText(context, "【"+name+"】终端设备不在线！", Toast.LENGTH_SHORT).show();
				}
			}
		});
		return convertView;
	}

	static class ViewHolder {
		RelativeLayout device_bg;
		ImageView device_net_state;
		ImageView device_icon;
		TextView device_name;
		TextView device_id;
	}
	
	/**
	 * 异步加载图片类
	 */
	private final class AsyncImageTask extends AsyncTask<String, Integer, String> {
		private RelativeLayout deviceBg;
		private String imagePath;
		
		public AsyncImageTask(RelativeLayout device_bg, String path) {
			this.deviceBg = device_bg;
			this.imagePath = path;
		}

		//后台运行的子线程
		@Override
		protected String doInBackground(String... params) {
			try {
				return getCacheImageUri(imagePath, thumbnailsFile);
			} catch (Exception e) {
				e.printStackTrace();
				System.out.println("MyDebug: doInBackground()异常！");
			}
			return null;
		}

		//更新界面显示
		@Override
		protected void onPostExecute(String path) {
			super.onPostExecute(path);
			if (deviceBg != null && path != null) {
				Drawable d = Drawable.createFromPath(path);
				deviceBg.setBackgroundDrawable(d);
			}
		}
	}
	
	/**
	 * 选择从本地或网上加载图片
	 */
	public String getCacheImageUri(String path, File cache) throws Exception {
		String name = path.substring(path.lastIndexOf("/")+1);
		File file = new File(cache, name);
		if (file.exists()) {
			return file.getPath();
		} else {
			URL url = new URL(path);
			HttpURLConnection conn = (HttpURLConnection) url.openConnection();
			conn.setConnectTimeout(5000);
			conn.setRequestMethod("GET");
			conn.setDoInput(true);
			if (conn.getResponseCode() == 200) {
				InputStream is = conn.getInputStream();
				FileOutputStream fos = new FileOutputStream(file);
				byte[] buffer = new byte[1024*2];
				int len = 0;
				while ((len = is.read(buffer)) != -1) {
					fos.write(buffer, 0, len);
				}
				is.close();
				fos.close();
				return file.getPath();
			}
		}
		return null;
	}
}
