package com.video.local;

import java.io.File;
import java.util.HashMap;
import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.video.R;
import com.video.utils.Utils;

public class GridViewAdapter extends BaseAdapter {
	private Context mContext;
	private List<HashMap<String, Object>> mList;

	public GridViewAdapter(Context context, List<HashMap<String, Object>> list) {
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
			convertView = LayoutInflater.from(this.mContext).inflate(R.layout.local_image_gridview_item, null);
			holder.imageView = (ImageView) convertView.findViewById(R.id.gridview_item_imageview);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		if (this.mList != null) {
			final HashMap<String, Object> hashMap = this.mList.get(position);
			if (holder.imageView != null) {
				BitmapFactory.Options opts = new BitmapFactory.Options();
				opts.inJustDecodeBounds = true;
				BitmapFactory.decodeFile(hashMap.get("imageInfo").toString(), opts);
				opts.inJustDecodeBounds = false;
				opts.inSampleSize = Utils.computeSampleSize(opts, -1, 256*256);
				try {
					Bitmap bm = BitmapFactory.decodeFile(hashMap.get("imageInfo").toString(), opts);
					holder.imageView.setImageBitmap(bm);
				} catch (OutOfMemoryError e) {
					e.printStackTrace();
				}
				holder.imageView.setScaleType(ImageView.ScaleType.FIT_XY);
				holder.imageView.setLayoutParams(new LinearLayout.LayoutParams(Utils.screenWidth/3, (Utils.screenWidth-240)/3));
			    
				holder.imageView.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						String imagePath = hashMap.get("imageInfo").toString();
						Bundle bundle = new Bundle();
						bundle.putString("imageViewPath", imagePath);
						File file = new File(imagePath);
						File imageFile = file.getParentFile();
						bundle.putString("imageViewFile", imageFile.toString());
						
						Intent intent = new Intent();
						intent.putExtras(bundle);
						intent.setClass(mContext, ViewLocalImageActivity.class);
						mContext.startActivity(intent);
					}
				});
			}
		}
		return convertView;
	}

	private class ViewHolder {
		ImageView imageView;
	}
}