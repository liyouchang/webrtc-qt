package com.video.main;

import java.io.File;
import java.util.ArrayList;
import java.util.Date;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.GridView;
import android.widget.ImageButton;
import android.widget.Toast;

import com.video.R;
import com.video.utils.Utils;

public class SetDeviceBgActivity extends Activity implements OnClickListener {

	private Context mContext;
	private final int INIT_LOCAL_IMAGE_REFRESH = 1;
	private int listPosition = 0;
	
	String SD_path = "";
	File currentFile;
	File[] currentFiles;
	
	private GridView gridView = null;
	public ArrayList<String> imageViews = null;
	private SetDeviceBgGridViewAdapter gridViewAdapter = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.set_device_bg);
		
		initView();
		initData();
	}
	
	/**
	 * 发送Handler消息
	 */
	public void sendHandlerMsg(int what, int arg1) {
		Message msg = new Message();
		msg.what = what;
		msg.arg1 = arg1;
		handler.sendMessage(msg);
	}
	
	public Handler handler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case INIT_LOCAL_IMAGE_REFRESH:
					if (msg.arg1 == 0) {
						gridView.setAdapter(gridViewAdapter);
					} else {
						gridViewAdapter.notifyDataSetChanged();
					}
					break;
				default: break;
			}
		}
	};
	
	private void initView() {
		ImageButton button_back = (ImageButton) this.findViewById(R.id.ib_set_device_bg_back); 
		button_back.setOnClickListener(this);
		
		Button button_ok = (Button) this.findViewById(R.id.btn_title_ok); 
		button_ok.setOnClickListener(this);
		
		gridView = (GridView) this.findViewById(R.id.image_gridview);
		gridView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) { 
				gridViewAdapter.changeImageVisable(view, position); 
				listPosition = position;
			}
		});
	}
	
	private void initData() {
		mContext = SetDeviceBgActivity.this;
		
		if (Utils.checkSDCard()) {
			SD_path = Environment.getExternalStorageDirectory().getAbsolutePath();
			File rootPath = new File(SD_path);
			currentFile = rootPath;
			currentFiles = rootPath.listFiles();
			
			if (isDirectoryExist(currentFiles, "KaerVideo")) {
				if (isDirectoryExist(currentFiles, "image")) {
					
					int imageFileCount = currentFiles.length;
					ArrayList<String> imageFileStringArray = handleImageFileName(currentFiles);
					
					if (imageFileStringArray == null) {
						Toast.makeText(mContext, "没有本地抓拍图片", Toast.LENGTH_SHORT).show();
					} else {
						for (int i=0; i<imageFileCount; i++) {
							//文件夹下的图片
							String imageFilePath = currentFile.getPath()+File.separator+imageFileStringArray.get(i);
							File file = new File(imageFilePath);
							File[] files = file.listFiles();
							ArrayList<String> imageViewsCache = listAllImageViews(files);
							if (imageViewsCache == null) {
								if (file.exists())
									file.delete();
								continue;
							} else {
								if (i == 0) {
									imageViews = imageViewsCache;
									gridViewAdapter = new SetDeviceBgGridViewAdapter(mContext, imageViews);
									sendHandlerMsg(INIT_LOCAL_IMAGE_REFRESH, i);
								} else {
									imageViews.addAll(imageViewsCache);
									sendHandlerMsg(INIT_LOCAL_IMAGE_REFRESH, i);
								}
							}
						}
					}
				}
			}
		}
	}
	
	/**
	 * 将制定文件夹下的图片全部列出来
	 * @param files 文件结合
	 * @return 返回图片格式的文件结合
	 */
	private ArrayList<String> listAllImageViews(File[] files) {
		int count = files.length;
		if (count == 0)
			return null;
		ArrayList<String> fileImages = new ArrayList<String>();;
		
		for (int i=0; i<count; i++) {
			if (!Utils.isImageFile(files[i].getName()))
				continue;
			fileImages.add(files[i].getPath());
		}
		return fileImages;
	}
	
	/**
	 * 排序文件夹集合，日期从大到小
	 */
	private ArrayList<Integer> sortImageFileName(File[] fileArray) {
		int fileCount = fileArray.length;
		ArrayList<Integer> arrayInteger = new ArrayList<Integer>();
		
		for (int i=0; i<fileCount; i++) {
			Date date = Utils.StringToDate(currentFiles[i].getName(), "yyyy-MM-dd");
			String fileString = Utils.DateToString(date, "yyyyMMdd");
			int fileInt = Integer.parseInt(fileString);
			arrayInteger.add(fileInt);
		}
		arrayInteger = Utils.bubbleSortArrayList(arrayInteger);
		return arrayInteger; 
	}
	
	/**
	 * 处理图片文件夹名字集合，排序日期从大到小的字符串数组
	 */
	private ArrayList<String> handleImageFileName(File[] fileArray) {
		int fileCount = fileArray.length;
		if (fileCount == 0)
			return null;
		ArrayList<String> arrayString = new ArrayList<String>();
		ArrayList<Integer> arrayInteger = sortImageFileName(fileArray);
		
		for (int i=0; i<fileCount; i++) {
			String fileString = arrayInteger.get(i).toString();
			Date date = Utils.StringToDate(fileString, "yyyyMMdd");
			fileString = Utils.DateToString(date, "yyyy-MM-dd");
			arrayString.add(fileString);
		}
		return arrayString;
	}

	/**
	 * 查找文件夹集合下是否有这个文件夹
	 * @param files 文件夹集合
	 * @param fileName 要查找的文件夹
	 * @return true:有 false:没有
	 */
	private boolean isDirectoryExist(File[] files, String fileName) {
		for (int i=0; i<files.length; i++) {
			if (files[i].getName().equals(fileName)) {
				currentFile = files[i];
				currentFiles = currentFile.listFiles();
				return true;
			}
		}
		return false;
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.ib_set_device_bg_back:
				SetDeviceBgActivity.this.finish();
				overridePendingTransition(R.anim.fragment_nochange, R.anim.down_out);
				break;
			case R.id.btn_title_ok:
				if ((imageViews != null) && (imageViews.size() > 0)) {
					Bundle bundle = new Bundle();
					bundle.putString("ImageBgPath", imageViews.get(listPosition));
					Intent intent = new Intent();
					intent.putExtras(bundle);
					setResult(2, intent);
				}
				SetDeviceBgActivity.this.finish();
				overridePendingTransition(0, R.anim.down_out);
				break;
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if (keyCode == KeyEvent.KEYCODE_BACK  && event.getRepeatCount() == 0) {
			SetDeviceBgActivity.this.finish();
			overridePendingTransition(R.anim.fragment_nochange, R.anim.down_out);
		}
		return super.onKeyDown(keyCode, event);
	}
}
