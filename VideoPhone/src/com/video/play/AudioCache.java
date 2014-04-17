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

	public void clearBuffer() {
		synchronized (mLock) {
			inPtr = 0;
		}
	}

	public boolean push(byte[] buf, int offset, int len) {
		boolean result = (len + inPtr < mBufferLen);
		if (result) {
			synchronized (this) {
				Tools.CopyByteArray(mBuffer, inPtr, buf, offset, len);
				inPtr += len;
			}
		}
		return result;
	}

	public int pop(byte[] buf, int offset) {
		int result = inPtr;
		if (result > 0) {
			synchronized (this) {
				Tools.CopyByteArray(buf, offset, mBuffer, 0, inPtr);
				inPtr = 0;
			}
		}
		return result;
	}
}
