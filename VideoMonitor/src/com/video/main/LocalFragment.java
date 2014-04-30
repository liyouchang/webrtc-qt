package com.video.main;

import java.io.File;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.video.R;
import com.video.local.ImageViewFileItem;
import com.video.local.ListViewAdapter;
import com.video.utils.Utils;
import com.video.utils.ViewPagerAdapter;

public class LocalFragment extends Fragment implements OnClickListener, OnPageChangeListener {

	private FragmentActivity mActivity;
	private View mView;
	
	private TextView viewpage_video;
	private TextView viewpage_image;
	
	private ViewPager mViewPager;
	private List<View> pageList;
	private View video_page;
	private View image_page;
	
	String SD_path = "";
	File currentFile;
	File[] currentFiles;
	
	//本地录像初始化
	
	//抓拍图片初始化
	private final int INIT_LOCAL_VIDEO_FINISH = 1;
	private final int INIT_LOCAL_IMAGE_FINISH = 2;
	private final int NO_LOCAL_IMAGE_FILE = 3;
	private ListView mListView;
	private ListViewAdapter mListViewAdapter;
	private List<ImageViewFileItem> mFileAll;
	private RelativeLayout noImageLayout = null;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		return inflater.inflate(R.layout.local, container, false);
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		
		mActivity = getActivity();
		mView = getView();
		
		initViewPageView();
		initView();
	}
	
	@Override
	public void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		initData();
	}
	
	private void initView() {
		viewpage_video = (TextView)mView.findViewById(R.id.tv_vp_video);
		viewpage_image = (TextView)mView.findViewById(R.id.tv_vp_image);
		//本地录像初始化
		
		//抓拍图片初始化
		mListView = (ListView)mView.findViewById(R.id.local_image_listView);
		noImageLayout = (RelativeLayout) mView.findViewById(R.id.rl_no_local_file_image);
		
		viewpage_video.setOnClickListener(this);
		viewpage_image.setOnClickListener(this);
	}
	
	private void initData() {
		//本地录像初始化
		
		//抓拍图片初始化
		new LocalImageThread().start();
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
			case R.id.tv_vp_image:
				viewpage_image.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_video.setBackgroundResource(R.drawable.viewpage_unselected);
				mViewPager.setCurrentItem(0);
				break;
			case R.id.tv_vp_video:
				viewpage_video.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_image.setBackgroundResource(R.drawable.viewpage_unselected);
				mViewPager.setCurrentItem(1);
				break;
		}
	}
	
	/**
	 * 初始化该界面下要滑动的页面
	 */
	private void initViewPageView() {
		LayoutInflater inflater = LayoutInflater.from(mActivity);
		image_page = inflater.inflate(R.layout.local_image, null);
		video_page = inflater.inflate(R.layout.local_video, null);
		pageList = new ArrayList<View>();
		pageList.add(image_page);
		pageList.add(video_page);
		mViewPager = (ViewPager)mView.findViewById(R.id.local_viewpager);
		mViewPager.setOnPageChangeListener(this);
		mViewPager.setAdapter(new ViewPagerAdapter(pageList));
		mViewPager.setCurrentItem(0);
	}

	@Override
	public void onPageScrollStateChanged(int arg0) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void onPageScrolled(int arg0, float arg1, int arg2) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void onPageSelected(int arg0) {
		// TODO Auto-generated method stub
		switch (arg0) {
			case 0:
				viewpage_image.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_video.setBackgroundResource(R.drawable.viewpage_unselected);
				mViewPager.setCurrentItem(0);
				break;
			case 1:
				viewpage_video.setBackgroundResource(R.drawable.viewpage_selected);
				viewpage_image.setBackgroundResource(R.drawable.viewpage_unselected);
				mViewPager.setCurrentItem(1);
				break;
		}
	}
	
	/**
	 * 发送Handler消息
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
	
	public Handler handler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
				case INIT_LOCAL_VIDEO_FINISH:
					
					break;
				case INIT_LOCAL_IMAGE_FINISH:
					mListViewAdapter = new ListViewAdapter(mActivity, mFileAll);
					if (mFileAll == null || mFileAll.size() == 0) {
						noImageLayout.setVisibility(View.VISIBLE);
					} else {
						noImageLayout.setVisibility(View.INVISIBLE);
					}
					if ((mListViewAdapter != null) && (mListView != null)) {
						mListView.setAdapter(mListViewAdapter);
						
					}
					break;
				case NO_LOCAL_IMAGE_FILE:
					noImageLayout.setVisibility(View.VISIBLE);
					break;
				default: break;
			}
		}
	};
	
	//-----------------------------------------------------------------------------------
	//本地录像处理
	//-----------------------------------------------------------------------------------
	
	
	//-----------------------------------------------------------------------------------
	//抓拍图片处理
	//-----------------------------------------------------------------------------------
	
	private class LocalImageThread extends Thread {

		@Override
		public void run() {
			// TODO Auto-generated method stub
			super.run();
			initLocalImageData();
		}
	}
	
	private void initLocalImageData() {
		
		mFileAll = new ArrayList<ImageViewFileItem>();
		ImageViewFileItem fileItem = null;
		
		//抓拍图片初始化
		if (Utils.checkSDCard()) {
			SD_path = Environment.getExternalStorageDirectory().getAbsolutePath();
			File rootPath = new File(SD_path);
			currentFile = rootPath;
			currentFiles = rootPath.listFiles();
			
			if (isDirectoryExist(currentFiles, "KaerVideo")) {
				if (isDirectoryExist(currentFiles, "image")) {
					
					ArrayList<String> imageFileStringArray = handleImageFileName(currentFiles);
					
					if (imageFileStringArray == null) {
						sendHandlerMsg(NO_LOCAL_IMAGE_FILE);
					} else {
						int imageFileCount = imageFileStringArray.size();
						for (int i=0; i<imageFileCount; i++) {
							fileItem = new ImageViewFileItem();
							//文件夹下的图片
							String imageFilePath = currentFile.getPath()+File.separator+imageFileStringArray.get(i);
							File file = new File(imageFilePath);
							File[] files = file.listFiles();
							ArrayList<HashMap<String, Object>> fileImages = listAllImageViews(files);
							if (fileImages == null) {
								if (file.exists())
									file.delete();
								continue;
							}
							fileItem.imageViews = fileImages;
							//文件夹名
							fileItem.fileName = imageFileStringArray.get(i);
							mFileAll.add(fileItem);
						}
						sendHandlerMsg(INIT_LOCAL_IMAGE_FINISH);
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
	private ArrayList<HashMap<String, Object>> listAllImageViews(File[] files) {
		int count = files.length;
		if (count == 0)
			return null;
		ArrayList<HashMap<String, Object>> fileImages = new ArrayList<HashMap<String, Object>>();;
		HashMap<String, Object> imageItem = null;
		
		for (int i=0; i<count; i++) {
			if (!Utils.isImageFile(files[i].getName()))
				continue;
			imageItem = new HashMap<String, Object>();
			imageItem.put("imageInfo", files[i].getPath());
			fileImages.add(imageItem);
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
			if (currentFiles[i].getName().indexOf("-") <= 0)
				continue;
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
		fileCount = arrayInteger.size();
		
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
}
