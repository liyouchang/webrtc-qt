package com.video.local;

import java.util.HashMap;
import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.TextView;

import com.video.R;

public class ListViewAdapter extends BaseAdapter {
	private List<ImageViewFileItem> mList;
	private Context mContext;

	public ListViewAdapter(Context context, List<ImageViewFileItem> list) {
		super();
		this.mList = list;
		this.mContext = context;
	}

	@Override
	public int getCount() {
		return mList.size();
	}

	@Override
	public Object getItem(int position) {
		return mList.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder = null;
		
		if (convertView == null) {
			holder = new ViewHolder();
			convertView = LayoutInflater.from(mContext).inflate(R.layout.local_image_listview_item, null);
			holder.textView = (TextView) convertView.findViewById(R.id.listview_item_textview);
			holder.gridView = (GridView) convertView.findViewById(R.id.listview_item_gridview);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}

		if (this.mList != null) {
			ImageViewFileItem fileItem = mList.get(position);
			List<HashMap<String, Object>> imageViewList = mList.get(position).imageViews;
			
			if (holder.textView != null) {
				holder.textView.setText(fileItem.fileName);
			}
			if (holder.gridView != null) {
				GridViewAdapter gridViewAdapter = new GridViewAdapter(mContext, imageViewList);
				holder.gridView.setAdapter(gridViewAdapter);
			}
		}
		return convertView;
	}

	private class ViewHolder {
		TextView textView;
		GridView gridView;
	}
}