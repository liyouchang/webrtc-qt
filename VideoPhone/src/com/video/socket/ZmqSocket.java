package com.video.socket;

import org.zeromq.ZMQ;

import android.content.Intent;
import android.os.Handler;

import com.video.service.BackstageService;
import com.video.utils.Utils;

public class ZmqSocket {
	
	private ZMQ.Context context;
	private ZMQ.Socket zmq_socket = null;
	
	private ZmqThread  zmqThread = null;
	public static Handler zmqSocketHandler = null;
	
	/**
	 * ZmqSocket的构造函数
	 */
	public ZmqSocket() {
		context = ZMQ.context(1);  
		zmq_socket = context.socket(ZMQ.DEALER); 
		zmq_socket.setIdentity(Utils.getRandomUUID().getBytes());
		
		if (zmqThread == null) {
        	zmqThread = new ZmqThread(zmq_socket);
        	zmqThread.start();
        	Intent intent = new Intent(HandlerApplication.getInstance(), BackstageService.class);
        	HandlerApplication.getInstance().startService(intent);
        }
	}
	
	/**
	 * 关闭ZMQ连接句柄
	 */
	public void zmqExit() {
		if (zmqThread != null) {
			zmqThread = null;
		}
		if (zmq_socket != null) {
			zmq_socket = null;
		}
	}
}
