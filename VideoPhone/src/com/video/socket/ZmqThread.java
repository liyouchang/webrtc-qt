package com.video.socket;

import java.util.HashMap;

import org.zeromq.ZMQ;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.video.R;
import com.video.data.Value;
import com.video.play.TunnelCommunication;

public class ZmqThread extends Thread {
	
	private ZMQ.Socket zmq_socket = null;
	public static Handler zmqThreadHandler;
	
	public ZmqThread(ZMQ.Socket socket) {
		this.zmq_socket = socket;
	}
	
	/**
	 * ZMQ�������
	 * @param data Ҫ���͵����
	 * @return true:���ͳɹ�  false:����ʧ��
	 */
	private boolean sendZmqData(String data) {
		boolean result = false;
		String requestContentString = data+" ";  
        byte[] requestContentByteArray = requestContentString.getBytes();  
        requestContentByteArray[requestContentByteArray.length-1] = 0;  
        
        result = zmq_socket.send(Value.BackstageName, ZMQ.SNDMORE); 
        result = zmq_socket.send(requestContentByteArray, 0);
        
        return result;
	}
	
	private boolean sendZmqData(String peerId, String data) {
		boolean result = false;
		String requestContentString = data+" ";  
        byte[] requestContentByteArray = requestContentString.getBytes();  
        requestContentByteArray[requestContentByteArray.length-1] = 0;  
        
        result = zmq_socket.send(peerId, ZMQ.SNDMORE); 
        result = zmq_socket.send(requestContentByteArray, 0);
        
        return result;
	}
	
	/**
	 * ZMQ��Backstage�������
	 * @return null:���ղ��ɹ������򷵻ؽ��յ������
	 */
	private String recvZmqData() {
		String result = null;
		ZMQ.Poller items = new ZMQ.Poller(1);
        items.register(zmq_socket, ZMQ.Poller.POLLIN);
        byte[] message_byte1;
        byte[] message_byte2;
        items.poll(100);
        if (items.pollin(0)) {
            message_byte1 = zmq_socket.recv(0);
            message_byte2 = zmq_socket.recv(0);
			String message_str1 = new String(message_byte1);
			String message_str2 = new String(message_byte2);
			if (message_str1.equals(Value.BackstageName)) {
				result = message_str2;
			} 
			else if (message_str1.equals("123456")) {
				System.out.println("MyDebug: �յ���Peer���1: " + message_str1);
				TunnelCommunication.Instance().messageFromPeer("123456", message_str2);
			}
        }
		return result;
	}
	
	/**
	 * ����Handler��Ϣ
	 */
	public void sendHandlerMsg(int what) {
		Message msg = new Message();
		msg.obj = what;
		zmqThreadHandler.sendMessageAtTime(msg, 100);
	}
	public void sendHandlerMsg(String obj) {
		Message msg = new Message();
		msg.obj = obj;
		ZmqHandler handler= new ZmqHandler();
		handler.sendMessage(msg);
	}

	@Override
	public void run() {
		zmq_socket.connect(Value.BackstageIPPort);
		
		Looper.prepare();
		zmqThreadHandler = new Handler() {
			@SuppressWarnings("unchecked")
			@Override
			public void handleMessage(Message msg) {
				switch (msg.what) {
					//�������
					case R.id.zmq_recv_data_id:
						String result = recvZmqData();
						if (result != null) {
							sendHandlerMsg(result);
							System.out.println("MyDebug: �յ������: "+result);
						}
						if (zmqThreadHandler.hasMessages(R.id.zmq_recv_data_id)) {
							zmqThreadHandler.removeMessages(R.id.zmq_recv_data_id);
						}
						break;
					//�������
					case R.id.zmq_send_data_id:
						sendZmqData((String)msg.obj);
						System.out.println("MyDebug: \n");
						System.out.println("MyDebug: ���͵����: "+(String)msg.obj);
						if (zmqThreadHandler.hasMessages(R.id.zmq_send_data_id)) {
							zmqThreadHandler.removeMessages(R.id.zmq_send_data_id);
						}
						break;
					//���ͶԵȶ����
					case R.id.send_to_peer_id:
						HashMap<String, String> mapData = (HashMap<String, String>)msg.obj;
						String peerId = mapData.get("peerId");
						String peerData = mapData.get("peerData");
						sendZmqData(peerId, peerData);
						System.out.println("MyDebug: \n");
						System.out.println("MyDebug: ���͵�Peer���: " + peerData);
						if (zmqThreadHandler.hasMessages(R.id.send_to_peer_id)) {
							zmqThreadHandler.removeMessages(R.id.send_to_peer_id);
						}
						break;
					//�ر�ZMQ Socket
					case R.id.close_zmq_socket_id:
						zmq_socket.close();
						ZmqCtrl.getInstance().exit();
						if (zmqThreadHandler.hasMessages(R.id.close_zmq_socket_id)) {
							zmqThreadHandler.removeMessages(R.id.close_zmq_socket_id);
						}
						break;
				}
			}
		};
		Looper.loop();
	}
}
