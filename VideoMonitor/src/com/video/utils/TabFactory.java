package com.video.utils;

import android.content.Context;
import android.view.View;
import android.widget.TabHost.TabContentFactory;

public class TabFactory implements TabContentFactory {
	private Context mContext;

	public TabFactory(Context context) {
		mContext = context;
	}

	@Override
	public View createTabContent(String tag) {
		View view = new View(mContext);
		return view;
	}

}
