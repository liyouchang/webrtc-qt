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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.video.R;
import com.video.main.AlarmImageViewActivity;
import com.video.main.MsgFragment;
import com.video.service.MainApplication;

public class MessageItemAdapter extends BaseAdapter {

	private Context context;
	private ArrayList<HashMap<String, String>> list;
	private File imageCache = null;

	public MessageItemAdapter(Context context, File imageCache, ArrayList<HashMap<String, String>> list) {
		this.context = context;
		this.imageCache = imageCache;
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
	
	private int getReadedState(String state) {
		int result = -1;
		if (state.equals("true")) {
			result = View.INVISIBLE;
		} else {
			result = View.VISIBLE;
		}
		return result;
	}

	@Override
	public View getView(final int position, View convertView, ViewGroup viewGroup) {
		ViewHolder holder;

		if (convertView == null) {
			convertView = LayoutInflater.from(context).inflate(R.layout.msg_alert_item, null);
			holder = new ViewHolder();
			convertView.setTag(holder);
			holder.iv_image = (ImageView) convertView.findViewById(R.id.iv_msg_image);
			holder.iv_state = (ImageView) convertView.findViewById(R.id.iv_msg_state);
			holder.msg_event = (TextView) convertView.findViewById(R.id.tv_msg_event);
			holder.msg_mac = (TextView) convertView.findViewById(R.id.tv_msg_mac);
			holder.msg_time = (TextView) convertView.findViewById(R.id.tv_msg_time);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		holder.iv_state.setVisibility(getReadedState(list.get(position).get("isReaded")));
		holder.msg_event.setText(list.get(position).get("msgEvent"));
		holder.msg_mac.setText(list.get(position).get("msgMAC"));
		holder.msg_time.setText(list.get(position).get("msgTime"));
		
		String msg_image_url = list.get(position).get("imageURL");
		if (!msg_image_url.equals("null")) {
			AsyncImageTask task = new AsyncImageTask(holder.iv_image, msg_image_url);
			task.execute();
		}

		holder.iv_image.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Intent intent = new Intent();
				if (list.get(position).get("isReaded").equals("false")) {
					intent.putExtra("listMsgID", list.get(position).get("msgID"));
					intent.setAction(MsgFragment.READ_MESSAGE_ACTION);
					MainApplication.getInstance().sendBroadcast(intent);
				}
				
				String UrlPath = list.get(position).get("imageURL");
				String filePath = imageCache+File.separator+UrlPath.substring(UrlPath.lastIndexOf("/")+1);
				intent = new Intent(context, AlarmImageViewActivity.class);
				intent.putExtra("imagePath", filePath);
				context.startActivity(intent);
			}
		});
		return convertView;
	}

	static class ViewHolder {
		ImageView iv_image;
		ImageView iv_state;
		TextView msg_event;
		TextView msg_mac;
		TextView msg_time;
	}
	
	/**
	 * 异步加载图片类
	 */
	private final class AsyncImageTask extends AsyncTask<String, Integer, String> {
		private ImageView imageView;
		private String imagePath;
		
		public AsyncImageTask(ImageView imageView, String path) {
			this.imageView = imageView;
			this.imagePath = path;
		}

		//后台运行的子线程
		@Override
		protected String doInBackground(String... params) {
			try {
				return getCacheImageLocalPath(imagePath, imageCache);
			} catch (Exception e) {
				e.printStackTrace();
				System.out.println("MyDebug: getCacheImageLocalPath()异常！");
			}
			return null;
		}

		//更新界面显示
		@Override
		protected void onPostExecute(String localPath) {
			super.onPostExecute(localPath);
			if (imageView != null && localPath != null) {
				BitmapFactory.Options opts = new BitmapFactory.Options();
				opts.inJustDecodeBounds = true;
				BitmapFactory.decodeFile(localPath, opts);
				opts.inJustDecodeBounds = false;
				opts.inSampleSize = Utils.computeSampleSize(opts, -1, 256*256);
				try {
					Bitmap bm = BitmapFactory.decodeFile(localPath, opts);
					Drawable drawable =new BitmapDrawable(bm);
					imageView.setBackgroundDrawable(drawable);
				} catch (OutOfMemoryError e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	/**
	 * 获得图片缓存的本地路径
	 * @param path 网络上图片的路径
	 * @param cache 本地缓存文件夹
	 * @return 返回缓存的本地路径
	 * @throws Exception 
	 */
	public String getCacheImageLocalPath(String path, File cache) throws Exception {
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
				byte[] buffer = new byte[1024];
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
