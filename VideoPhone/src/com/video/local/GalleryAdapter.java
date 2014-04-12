package com.video.local;

import java.util.ArrayList;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.BaseAdapter;
import android.widget.Gallery;

public class GalleryAdapter extends BaseAdapter {

	private Context context;
	private ArrayList<String> imagesPath;

	public GalleryAdapter(Context context, ArrayList<String> list) {
		this.context = context;
		this.imagesPath = list;
	}

	@Override
	public int getCount() {
		return imagesPath.size();
	}

	@Override
	public Object getItem(int position) {
		return imagesPath.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		Bitmap bm = BitmapFactory.decodeFile(imagesPath.get(position), null);
		DefineImageView view = new DefineImageView(context, bm.getWidth(), bm.getHeight());
		view.setLayoutParams(new Gallery.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
		view.setImageBitmap(bm);
		return view;
	}
}
