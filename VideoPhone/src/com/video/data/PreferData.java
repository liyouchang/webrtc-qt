package com.video.data;

import android.content.Context;
import android.content.SharedPreferences;

public class PreferData {
	
	private static final String FILENAME =  "information";
	private SharedPreferences saveprefer;
	private SharedPreferences.Editor editor;
	
	public PreferData(Context context) {
		saveprefer = context.getSharedPreferences(FILENAME, Context.MODE_PRIVATE);
		editor = saveprefer.edit();
	}
	
	public boolean writeData(String key, String value) {
		boolean result = false;
		editor.putString(key, value);
		result = editor.commit();
		return result;
	}
	
	public boolean writeData(String key, int value) {
		boolean result = false;
		editor.putInt(key, value);
		result = editor.commit();
		return result;
	}
	
	public boolean writeData(String key, boolean value) {
		boolean result = false;
		editor.putBoolean(key, value);
		result = editor.commit();
		return result;
	}
	
	public String readString(String data) {
		String result;
		result = saveprefer.getString(data, null);
		return result;
	}
	
	public int readInt(String data) {
		int result;
		result = saveprefer.getInt(data, 0xffff);
		return result;
	}
	
	public boolean readBoolean(String data) {
		boolean result;
		result = saveprefer.getBoolean(data, false);
		return result;
	}
	
	public boolean deleteItem(String item) {
		boolean result = false;
		editor.remove(item);
		result = editor.commit();
		return result;
	}
	
	public boolean isExist(String item) {
		boolean result = false;
		result = saveprefer.contains(item);
		return result;
	}
}
