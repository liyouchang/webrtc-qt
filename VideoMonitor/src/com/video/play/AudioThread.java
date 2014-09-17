package com.video.play;

import com.video.utils.Utils;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class AudioThread extends Thread {

	private AudioTrack audioTrack = null;
	public boolean runFlag = false;
	private boolean isPlayAudio = false;
	
	private byte[] readBuf = null;
	private byte[] writeBuf = null;
	private int audioTrackBufferSize = 0; 
	private int audioTrackPlaySize = 0;

	/**
	 * 初始化
	 */
	private void initAudioThread() {
		try {
			audioTrackBufferSize = AudioTrack.getMinBufferSize(8000,
					AudioFormat.CHANNEL_CONFIGURATION_MONO,
					AudioFormat.ENCODING_PCM_16BIT);
			
			audioTrackPlaySize = audioTrackBufferSize * 2;
			audioTrackBufferSize = audioTrackBufferSize * 100;
			
			audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, 8000,
					AudioFormat.CHANNEL_CONFIGURATION_MONO,
					AudioFormat.ENCODING_PCM_16BIT, audioTrackPlaySize,
					AudioTrack.MODE_STREAM);

			readBuf = new byte[audioTrackBufferSize];
			writeBuf = new byte[audioTrackBufferSize * 2];
			audioTrack.play();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * 反初始化
	 */
	private void uninitAudioThread() {
		TunnelCommunication.audioDataCache.clearBuffer();
		if (audioTrack != null) {
			try {
				if (audioTrack.getPlayState() != AudioTrack.PLAYSTATE_STOPPED) {
					audioTrack.stop();
				}
				audioTrack.release();
				audioTrack = null;
				readBuf = null;
				writeBuf = null;
			} catch (Exception e) {
				audioTrack = null;
				Utils.log("uninitAudioThread()异常！");
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * 播放声音
	 */
	public void startAudioThread() {
		runFlag = true;
		isPlayAudio = true;
		initAudioThread();
	}
	
	/**
	 * 停止声音
	 */
	public void stopAudioThread() {
		isPlayAudio = false;
		runFlag = false;
		uninitAudioThread();
	}
	
	public void run() {
		while (runFlag) {
			try {
				// 播放、暂停
				if (!isPlayAudio) {
					sleep(100);
					continue;
				}
				int readBufLen = TunnelCommunication.audioDataCache.pop(readBuf, 0);
				if (readBufLen > 0) {
					readBufLen = G711Decoder(writeBuf, readBuf, readBufLen);
					int playPosition = 0;
					while ( (audioTrack != null) && (readBufLen > playPosition) ) {
						if (readBufLen - playPosition > audioTrackPlaySize) {
							audioTrack.write(writeBuf, playPosition, audioTrackPlaySize);
							playPosition = playPosition + audioTrackPlaySize;
						} else {
							audioTrack.write(writeBuf, playPosition, readBufLen - playPosition);
							playPosition = readBufLen;
						}
					}
					sleep(5);
				} else {
					sleep(10);
				}
			} catch (Exception e) {
				e.printStackTrace();
				uninitAudioThread();
				initAudioThread();
			}
		}
		stopAudioThread();
	}
	
	/**
	 * G711解码 ulaw
	 */
	private short ulawToLinear(byte ulawData) {
		int temp;
		ulawData = (byte) ~ulawData;
		temp = ((ulawData & 0xf) << 3) + 0x84;
		temp <<= ((byte) ulawData & 0x70) >> 4;
		return (short) ((ulawData & 0x80) != 0 ? (0x84 - temp) : (temp - 0x84));
	}

	/**
	 * G711音频解码函数，采用ulaw解码
	 */
	private int G711Decoder(byte[] dst, byte[] src, int len) {
		for (int i = 0; i < len; i++) {
			short shValues = ulawToLinear(src[i]);
			dst[2 * i] = (byte) (shValues & 0xFF);
			dst[2 * i + 1] = (byte) ((shValues >> 8) & 0xFF);
		}
		return len * 2;
	}
	
	/**
	 * 打开音频声音
	 */
	public void openAudioTrackVolume() {
		if (audioTrack != null) {
			isPlayAudio = true;
			audioTrack.setStereoVolume(1.0f, 1.0f);
		}
	}
	
	/**
	 * 关闭音频声音
	 */
	public void closeAudioTrackVolume() {
		if (audioTrack != null) {
			isPlayAudio = false;
			audioTrack.setStereoVolume(0.0f, 0.0f);
		}
	}
}
