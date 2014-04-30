package com.video.play;

import com.video.utils.Tools;

public class AudioCache {

	private byte[] mLock = new byte[0];
	private byte[] mBuffer = null;
	private int mBufferLen;
	private int inPtr = 0;

	public AudioCache(int size) {
		mBufferLen = size;
		mBuffer = new byte[mBufferLen];
		clearBuffer();
	}

	/**
	 * 清空缓冲区
	 */
	public void clearBuffer() {
		synchronized (mLock) {
			inPtr = 0;
		}
	}

	/**
	 * 将数据压入缓冲区
	 * @param buf 要压入的数据
	 * @param offset 压入数据的偏移量
	 * @param len 压入数据的长度
	 * @return 0:压入成功  <0压入失败
	 */
	public boolean push(byte[] buf, int offset, int len) {
		boolean result = (len + inPtr < mBufferLen);
		if (result) {
			synchronized (mLock) {
				Tools.CopyByteArray(mBuffer, inPtr, buf, offset, len);
				inPtr += len;
			}
		} else {
			inPtr = 0;
		}
		return result;
	}

	/**
	 * 弹出缓冲区的数据
	 * @param buf 弹出的数据
	 * @param offset 弹出数据的偏移量
	 * @return 返回弹出数据的大小
	 */
	public int pop(byte[] buf, int offset) {
		int result = inPtr;
		if (result > 0) {
			synchronized (mLock) {
				Tools.CopyByteArray(buf, offset, mBuffer, 0, inPtr);
				inPtr = 0;
			}
		}
		return result;
	}
}
