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
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;
import com.video.data.Value;
import com.video.play.TunnelCommunication;
import com.video.play.VideoPlayActivity;

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
			holder.terminal_video = (Button) convertView.findViewById(R.id.btn_terminal_video);
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
		//实时视频
		holder.device_icon.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
//				Intent intent = new Intent(context, VideoPlayActivity.class);
//				intent.putExtra("dealerName", list.get(position).get("dealerName"));
//				context.startActivity(intent);
				if (position == 0) {
					TunnelCommunication.getInstance().askMediaData(Value.TerminalDealerName);
				} else {
					TunnelCommunication.getInstance().tunnelInitialize("com/video/play/TunnelCommunication");
					TunnelCommunication.getInstance().openTunnel(list.get(position).get("dealerName"));
					Value.TerminalDealerName = list.get(position).get("dealerName");
				}
			}
		});
		//终端视频
		holder.terminal_video.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Toast.makeText(context, "终端视频："+position, Toast.LENGTH_SHORT).show();
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
		Button terminal_video;
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
