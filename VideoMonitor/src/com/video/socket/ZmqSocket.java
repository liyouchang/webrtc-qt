package com.video.socket;

import org.zeromq.ZMQ;

import android.content.Intent;
import android.os.Handler;

import com.video.data.Value;
import com.video.service.BackstageService;
import com.video.utils.Utils;

public class ZmqSocket {
	
	private ZMQ.Context context = null;
	private ZMQ.Socket zmq_socket = null;
	
	private ZmqThread  zmqThread = null;
	public static Handler zmqSocketHandler = null;
	
	/**
	 * ZmqSocket初始化
	 */
	public ZmqSocket() {
		context = ZMQ.context(1); 
		zmq_socket = context.socket(ZMQ.DEALER); 
		byte[] byteArray = Utils.getRandomUUID().getBytes();
		zmq_socket.setIdentity(byteArray);
		System.out.println("MyDebug: ZMQ identfy: "+Utils.BytesToHexString(byteArray));
		
		if (zmqThread == null) {
        	zmqThread = new ZmqThread(zmq_socket);
        	zmqThread.start();
    		Intent intent = new Intent(HandlerApplication.getInstance(), BackstageService.class);
        	HandlerApplication.getInstance().startService(intent);
        }
	}
	
	/**
	 * 关闭ZMQ句柄
	 */
	public void zmqExit() {
		if (zmqThread != null) {
			zmqThread = null;
		}
		if (zmq_socket != null) {
			new ExitSocketThread().start();
			
			new ZmqSocket();
		}
	}
	
	public class ExitSocketThread extends Thread {

		@Override
		public void run() {
			try {
				zmq_socket.disconnect(Value.BackstageIPPort);
				zmq_socket = null;
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
}
