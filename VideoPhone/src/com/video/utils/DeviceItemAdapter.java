package com.video.utils;

import java.util.ArrayList;
import java.util.HashMap;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.video.R;

public class DeviceItemAdapter extends BaseAdapter {

	private Context context;
	private ArrayList<HashMap<String, String>> list;

	public DeviceItemAdapter(Context context, ArrayList<HashMap<String, String>> list) {
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
	
	private int getOnlineState(String state) {
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
			convertView = LayoutInflater.from(context).inflate(R.layout.device_item, null);
			holder = new ViewHolder();
			convertView.setTag(holder);
			holder.device_icon = (ImageView) convertView.findViewById(R.id.iv_device_icon);
			holder.device_net_state = (ImageView) convertView.findViewById(R.id.iv_device_net_state);
			holder.device_name = (TextView) convertView.findViewById(R.id.tv_device_name);
			holder.device_id = (TextView) convertView.findViewById(R.id.tv_device_mac);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		holder.device_net_state.setVisibility(getOnlineState(list.get(position).get("isOnline")));
		holder.device_name.setText(list.get(position).get("deviceName"));
		holder.device_id.setText(list.get(position).get("deviceID"));
		holder.device_icon.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Toast.makeText(context, "AdapterÖÐµã»÷£º"+position, Toast.LENGTH_SHORT).show();
			}
		});
		return convertView;
	}

	static class ViewHolder {
		ImageView device_net_state;
		ImageView device_icon;
		TextView device_name;
		TextView device_id;
	}
}
