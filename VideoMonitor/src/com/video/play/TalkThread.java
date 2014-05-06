package com.video.play;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

public class TalkThread extends Thread {

	private AudioRecord audioRecord = null;
	private boolean runFlag = false;
	private int audioRecordBufferSize = 0; 

	/**
	 * 初始化
	 */
	private void initTalkThread() {
		audioRecordBufferSize = AudioRecord.getMinBufferSize(8000,
				AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT);
		
		audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 8000,
				AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT, audioRecordBufferSize);
	}

	/**
	 * 反初始化
	 */
	private void uninitTalkThread() {
		if (audioRecord != null) {
			try {
				audioRecord.stop();
		        audioRecord.release();
		        audioRecord = null;
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * 停止对讲
	 */
	public void stopTalkThread() {
		runFlag = false;
		uninitTalkThread();
	}
	
	public void run() {
		initTalkThread();
		runFlag = true;
		byte[] buffer = new byte[audioRecordBufferSize];
		audioRecord.startRecording();
		byte[] ulawData = new byte[1024*2];
		int ulawDataLen = 0;
		
		while (runFlag) {
			try {
				int bufferReadResult = audioRecord.read(buffer, 0, audioRecordBufferSize);
				ulawDataLen = G711EnCoder(ulawData, buffer, bufferReadResult);
				if (ulawDataLen > 0) {
					TunnelCommunication.getInstance().sendTalkData(ulawData);
					sleep(10);
				} else {
					sleep(20);
				}
			} catch (Exception e) {
				e.printStackTrace();
				uninitTalkThread();
				initTalkThread();
			}
		}
		stopTalkThread();
	}
	
	private int search(int val, short[] segTags, int size) {
		for (int i = 0; i < size; i++) {
			if (val <= segTags[i])
				return (i);
		}
		return (size);
	}
	
	/**
	 * G711编码linear
	 */
	private byte linearToUlaw(short linearData) {
		int mask;
		int seg;
		byte ulawData;
		short[] segTags = {0xFF, 0x1FF, 0x3FF, 0x7FF,0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};
		
		if (linearData < 0) {
			linearData = (short) (0x84 - linearData);
			mask = 0x7F;
		} else {
			linearData += 0x84;
			mask = 0xFF;
		}
		seg = search(linearData, segTags, 8);
		if (seg >= 8)
			return (byte) (0x7F ^ mask);
		else {
			ulawData = (byte) ((seg << 4) | ((linearData >> (seg + 3)) & 0xF));
			return (byte) (ulawData ^ mask);
		}
	}

	/**
	 * G711音频编码函数，采用linear编码
	 */
	private int G711EnCoder(byte[] dst, byte[] src, int len) {
		short[] buffer = new short[1024*2];
		for (int i = 0; i < len / 2; i++) {
			buffer[i] = (short) ((src[i * 2] & 0xff) | (src[i * 2 + 1] & 0xff) << 8);
		}
		for (int i = 0; i < len / 2; i++) {
			dst[i] = linearToUlaw(buffer[i]);
		}
		return len / 2;
	}
}
