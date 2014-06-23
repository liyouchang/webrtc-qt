package com.video.main;

import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.video.R;
import com.video.utils.Utils;

public class SetDeviceBgGridViewAdapter extends BaseAdapter {
	
	private Context mContext;
	private List<String> mList;
	
	private View mLastView;
    private int mLastPosition;
    private int mLastVisibility;

	public SetDeviceBgGridViewAdapter(Context context, List<String> list) {
		super();
		this.mContext = context;
		this.mList = list;
		mLastPosition = -1;
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
			convertView = LayoutInflater.from(this.mContext).inflate(R.layout.set_device_bg_gridview_item, null);
			holder.imageView = (ImageView) convertView.findViewById(R.id.gridview_item_imageview);
			holder.imageViewSelected = (ImageView) convertView.findViewById(R.id.gridview_selected_item);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		if (this.mList != null) {
			final String imagePath = this.mList.get(position);
			if (holder.imageView != null) {
				BitmapFactory.Options opts = new BitmapFactory.Options();
				opts.inJustDecodeBounds = true;
				BitmapFactory.decodeFile(imagePath, opts);
				opts.inJustDecodeBounds = false;
				opts.inSampleSize = Utils.computeSampleSize(opts, -1, 256*256);
				try {
					Bitmap bm = BitmapFactory.decodeFile(imagePath, opts);
					holder.imageView.setImageBitmap(bm);
				} catch (OutOfMemoryError e) {
					e.printStackTrace();
				}
				holder.imageView.setScaleType(ImageView.ScaleType.FIT_XY);
				holder.imageView.setLayoutParams(new RelativeLayout.LayoutParams(Utils.screenWidth/3, (Utils.screenWidth-240)/3));
			}
			if (mLastPosition == position) {
				holder.imageViewSelected.setVisibility(mLastVisibility);
			} else {
				holder.imageViewSelected.setVisibility(View.GONE);
			}
		}
		return convertView;
	}

	private class ViewHolder {
		ImageView imageView;
		ImageView imageViewSelected;
	}
	
	public void changeImageVisable(View view, int position) {
		if (mLastView != null && mLastPosition != position) {
			ViewHolder holder = (ViewHolder) mLastView.getTag();
			if (holder.imageViewSelected.getVisibility() == View.VISIBLE) {
				holder.imageViewSelected.setVisibility(View.GONE);
				mLastVisibility = View.GONE;
			}
		}
		mLastPosition = position;
		mLastView = view;
		ViewHolder holder = (ViewHolder) view.getTag();
		switch (holder.imageViewSelected.getVisibility()) {
			case View.GONE:
				holder.imageViewSelected.setVisibility(View.VISIBLE);
				mLastVisibility = View.VISIBLE;
				break;
			case View.VISIBLE:
				holder.imageViewSelected.setVisibility(View.GONE);
				mLastVisibility = View.GONE;
				break;
		}
	}

}