package com.video.local;

import java.io.File;
import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Gallery;
import android.widget.ImageButton;

import com.video.R;
import com.video.utils.OkCancelDialog;
import com.video.utils.Utils;

public class ViewLocalImageActivity extends Activity implements OnClickListener {

	private Context mContext;
	private Gallery mGallery;
	private ArrayList<String> imagesPath = null;
	private final int INIT_FINISH = 1;
	
	private String imagePath;
	private String imageFile;
	
	private int count = 0;
	private int index = 0;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.view_local_image);
		
		initView();
		initData();
		new InitImageDataThread().start();
	}
	
	private void initView() {
		mGallery = (Gallery) super.findViewById(R.id.mygallery);
		
		ImageButton back = (ImageButton) super.findViewById(R.id.ib_image_back);
		back.setOnClickListener(this);
		ImageButton delete = (ImageButton) super.findViewById(R.id.ib_image_delete);
		delete.setOnClickListener(this);
	}
	
	private void initData() {
		mContext = ViewLocalImageActivity.this;
		Bundle bundle = this.getIntent().getExtras();
		imagePath = bundle.getString("imageViewPath");
		imageFile = bundle.getString("imageViewFile");
	}
	
	public Handler handler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case INIT_FINISH:
					mGallery.setVerticalFadingEdgeEnabled(false);	
					mGallery.setHorizontalFadingEdgeEnabled(false);
					mGallery.setAdapter(new GalleryAdapter(mContext, imagesPath));
					mGallery.setSelection(index);
					break;
				default: break;
			}
		}
	};
	
	private void initImageViewDataSource() {
		imagesPath = new ArrayList<String>();
		count = 0;
		File file = new File(imageFile);
		File[] files = file.listFiles();
		int filesCount = files.length;
		
		for (int i=0; i<filesCount; i++) {
			if (Utils.isImageFile(files[i].getName())) {
				count++;
				imagesPath.add(files[i].getPath());
				if (imagePath.equals(files[i].getPath())) {
					index = i;
				}
			}
		}
		sendHandlerMsg(INIT_FINISH);
	}
	
	private class InitImageDataThread extends Thread {

		@Override
		public void run() {
			// TODO Auto-generated method stub
			super.run();
			initImageViewDataSource();
		}
	}
	
	/**
	 * 向主线程发送Handler消息
	 * @param what 消息类型
	 */
	public void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}
	public void sendHandlerMsg(int what, int arg1) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		handler.sendMessage(msg);
	}
	
	private void deleteImage() {
		File file = new File(imagesPath.get(index));
		file.delete();
		count--;
		if (count == 0) {
			this.finish();
		} else {
			if ((index+1) < count)
				index++;
			else
				index--;
			if (index < 0)
				index = 0;
			new InitImageDataThread().start();
		}
	} 
	
	/**
	 * 删除照片的提示
	 */
	private void showDeleteImageDialog() {
		final OkCancelDialog myDialog=new OkCancelDialog(mContext);
		myDialog.setTitle("温馨提示");
		myDialog.setMessage("确认删除该张图片？");
		myDialog.setPositiveButton("确认", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
				deleteImage();
			}
		});
		myDialog.setNegativeButton("取消", new OnClickListener() {
			@Override
			public void onClick(View v) {
				myDialog.dismiss();
			}
		});
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.ib_image_delete:
				showDeleteImageDialog();
				break;
			case R.id.ib_image_back:
				this.finish();
				break;
		}
	}
}
