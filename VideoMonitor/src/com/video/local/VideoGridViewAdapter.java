package com.video.local;

import java.io.File;
import java.util.HashMap;
import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.video.R;
import com.video.local.player.LocalPlayerActivity;
import com.video.main.LocalFragment;
import com.video.utils.OkCancelDialog;
import com.video.utils.Utils;

public class VideoGridViewAdapter extends BaseAdapter {
	private Context mContext;
	private List<HashMap<String, Object>> mList;

	public VideoGridViewAdapter(Context context, List<HashMap<String, Object>> list) {
		super();
		this.mContext = context;
		this.mList = list;
	}

	@Override
	public int getCount() {
		if (mList == null) {
			return 0;
		} else {
			return this.mList.size();
		}
	}

	@Override
	public Object getItem(int position) {
		if (mList == null) {
			return null;
		} else {
			return this.mList.get(position);
		}
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
	
	@Override
	public View getView(final int position, View convertView, ViewGroup parent) {
		ViewHolder holder = null;
		
		if (convertView == null) {
			holder = new ViewHolder();
			convertView = LayoutInflater.from(this.mContext).inflate(R.layout.local_video_gridview_item, null);
			holder.video_bg = (ImageView) convertView.findViewById(R.id.iv_local_video_bg);
			holder.video_view = (ImageView) convertView.findViewById(R.id.iv_local_video_icon);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		if (this.mList != null) {
			final HashMap<String, Object> hashMap = this.mList.get(position);
			if (holder.video_bg != null) {
				BitmapFactory.Options opts = new BitmapFactory.Options();
				opts.inJustDecodeBounds = true;
				BitmapFactory.decodeFile(hashMap.get("videoBg").toString(), opts);
				opts.inJustDecodeBounds = false;
				opts.inSampleSize = Utils.computeSampleSize(opts, -1, 256*256);
				try {
					Bitmap bm = BitmapFactory.decodeFile(hashMap.get("videoBg").toString(), opts);
					holder.video_bg.setImageBitmap(bm);
				} catch (OutOfMemoryError e) {
					e.printStackTrace();
				}
				holder.video_bg.setScaleType(ImageView.ScaleType.FIT_XY);
				holder.video_bg.setLayoutParams(new RelativeLayout.LayoutParams(Utils.screenWidth/3, (Utils.screenWidth-120)/3));
				holder.video_bg.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						// TODO Auto-generated method stub
						final OkCancelDialog myDialog=new OkCancelDialog(mContext);
						myDialog.setTitle("温馨提示");
						myDialog.setMessage("确认删除当前的本地录像？");
						myDialog.setPositiveButton("确定", new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog.dismiss();
								File fileVideo = new File((String) hashMap.get("videoPath"));
								if (fileVideo.exists()) {
									fileVideo.delete();
								}
								File fileBg = new File((String) hashMap.get("videoBg"));
								if (fileBg.exists()) {
									fileBg.delete();
								}
								Intent intent = new Intent();
								intent.setAction(LocalFragment.REFRESH_VIDEO_FILE);
								mContext.sendBroadcast(intent);
							}
						});
						myDialog.setNegativeButton("取消", new OnClickListener() {
							@Override
							public void onClick(View v) {
								myDialog.dismiss();
							}
						});
					}
				});
			}
			if (holder.video_view != null) {
				holder.video_view.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						// TODO Auto-generated method stub
						//本地播放器
						Intent intent = new Intent(mContext, LocalPlayerActivity.class);
						intent.putExtra("videoFile", (String) hashMap.get("videoFile"));
						intent.putExtra("videoPath", (String) hashMap.get("videoPath"));
						mContext.startActivity(intent);
					}
				});
			}
		}
		return convertView;
	}

	private class ViewHolder {
		ImageView video_bg;
		ImageView video_view;
	}
}