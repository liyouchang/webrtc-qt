package com.video.utils;

import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Gallery;
import android.widget.ImageView;

@SuppressWarnings("deprecation")
public class ImageItem {

	public String fileName;
	public List<String> imageViews;
	public ImageAdapter imageAdapter;
	
	public void initImageItem(Context context, List<String> list) {
		this.imageAdapter = new ImageAdapter(context);
		this.imageViews = list;
	}
	
	public class ImageAdapter extends BaseAdapter {

		private Context context;
		
		private ImageAdapter (Context context) {
			this.context = context;
		}
		@Override
		public int getCount() {
			// TODO Auto-generated method stub
			return imageViews.size();
		}

		@Override
		public Object getItem(int position) {
			// TODO Auto-generated method stub
			return position;
		}

		@Override
		public long getItemId(int position) {
			// TODO Auto-generated method stub
			return position;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			// TODO Auto-generated method stub
			ImageView imageView = new ImageView(context);
		    Bitmap bm = BitmapFactory.decodeFile(imageViews.get(position).toString());
		    imageView.setImageBitmap(bm);
		    imageView.setScaleType(ImageView.ScaleType.FIT_XY);
		    imageView.setLayoutParams(new Gallery.LayoutParams((Utils.screenWidth-20)/3, (Utils.screenWidth-20)/3));
		    return imageView;
		}
	}
}
