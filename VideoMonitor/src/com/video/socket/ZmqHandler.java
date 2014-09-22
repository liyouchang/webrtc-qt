package com.video.socket;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.content.Intent;
import android.os.Build;
import android.os.Handler;
import android.os.Message;

import com.video.R;
import com.video.data.DeviceValue;
import com.video.data.Value;
import com.video.data.XmlMessage;
import com.video.play.PlayerActivity;
import com.video.service.BackstageService;
import com.video.service.MainApplication;
import com.video.utils.Utils;

public class ZmqHandler extends Handler {
	
	public static Handler mHandler = null;
	
	/**
	 * 转换获得的状态，将int类型转为String类型1: true、其它为false
	 */
	private String getState(int state) {
	    if (state == 1) {
	    	return "true";
	    }
	    return "false";
	}
	
	/**
	 * 解析获得的请求终端列表JSONArray数据
	 */
	private ArrayList<HashMap<String, String>> getReqTermList(JSONArray jsonArray) {
		ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();
		HashMap<String, String> item = null;
		int len = jsonArray.length();
		if (len <= 0) {
			return null;
		}
		  
	    try {
	    	for (int i=0; i<len; i++) { 
		    	JSONObject obj = (JSONObject) jsonArray.get(i); 
		    	item = new HashMap<String, String>();
			    item.put("deviceName", obj.getString("TermName")); 
			    item.put("deviceID", obj.getString("MAC"));
			    item.put("isOnline", getState(obj.getInt("Active")));
				item.put("dealerName", obj.getString("DealerName"));
				if (obj.isNull("PictureURL")) {
					item.put("deviceBg", "null");
				} else {
					item.put("deviceBg", obj.getString("PictureURL"));
				}
				item.put("LinkState", "notlink"); // 联机的4种状态：linked:已联机 notlink:无法联机 linking:正在联机... timeout:联机超时
				item.put(DeviceValue.HASH_PLAYER_CLARITY, String.valueOf(DeviceValue.NORMAL_CLARITY));
				list.add(item);
	    	}
	    	return list;
		} catch (JSONException e) {
			e.printStackTrace();
			Utils.log("getReqTermList()异常！");
			if ((list != null) && (list.size() > 0)) {
				return list;
			}
		}
		return null;
	}
	
	/**
	 * 解析获得的请求报警数据JSONArray数据
	 */
	private ArrayList<HashMap<String, String>> getReqAlarmList(JSONArray jsonArray) {
		ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();
		HashMap<String, String> item = null;
		int len = jsonArray.length();
		if (len <= 0) {
			return null;
		}
		int deviceCount = 0;
		if (MainApplication.getInstance().deviceList != null) {
			deviceCount = MainApplication.getInstance().deviceList.size();
		} else {
			deviceCount = 0;
		}
		  
	    try {
	    	for (int i=0; i<len; i++) { 
		    	JSONObject obj = (JSONObject) jsonArray.get(i); 
		    	item = new HashMap<String, String>();
		    	String alarmDevice = obj.getString("MAC");
		    	
		    	for (int j=0; j<deviceCount; j++) {
		    		if (MainApplication.getInstance().deviceList.get(j).get("deviceID").equals(alarmDevice)) {
		    			alarmDevice = MainApplication.getInstance().deviceList.get(j).get("deviceName");
		    			break;
		    		}
		    	}
			    item.put("msgMAC", MainApplication.getInstance().getResources().getString(R.string.from)+":"+alarmDevice);//来自
			    // alarmType 0:开关量报警 3:移动侦测 4:遮挡报警 6:敲门事件
			    int alarmType = obj.getInt("AlarmType");
			    String alarmInfo = "";
			    switch (alarmType) {
				    case 0: 
				    	alarmInfo = MainApplication.getInstance().getResources().getString(R.string.switch_alarm);//"开关量报警"
				    	break;
				    case 3: 
				    	alarmInfo = MainApplication.getInstance().getResources().getString(R.string.mobile_detection_alarm);//"移动侦测报警";
				    	break;
				    case 4: 
				    	alarmInfo = MainApplication.getInstance().getResources().getString(R.string.block_alarm);//"遮挡报警";
				    	break;
				    case 6: 
				    	alarmInfo = MainApplication.getInstance().getResources().getString(R.string.knock_door_event);//"敲门事件";
				    	break;
				    default :
				    	alarmInfo = MainApplication.getInstance().getResources().getString(R.string.unknown_alarm);//"未知报警";
				    	break;
			    }
			    item.put("msgEvent", MainApplication.getInstance().getResources().getString(R.string.event)+":"+alarmInfo);//事件
				item.put("msgTime", obj.getString("DateTime"));
				
				String pictureURL = "null";
				if (obj.isNull("PictureURL")) {
					pictureURL = "null";
				} else {
					pictureURL = obj.getString("PictureURL");
				}
				item.put("imageURL", pictureURL);
				item.put("msgID", obj.getString("ID"));
				item.put("isReaded", getState(obj.getInt("IsUsed")));
				list.add(item);
	    	}
	    	return list;
		} catch (JSONException e) {
			e.printStackTrace();
			Utils.log("getReqAlarmList()异常！");
			if ((list != null) && (list.size() > 0)) {
				return list;
			}
		}
		return null;
	}
	
	/**
	 * 请求指定终端分享用户列表
	 */
	private ArrayList<String> getReqMACShareUserList(JSONArray jsonArray) {
		ArrayList<String> list = new ArrayList<String>();
		int len = jsonArray.length();
		  
	    try {
	    	for (int i=0; i<len; i++) { 
		    	JSONObject obj = (JSONObject) jsonArray.get(i); 
				list.add(obj.getString("UserName"));
	    	}
	    	return list;
		} catch (JSONException e) {
			e.printStackTrace();
			Utils.log("getReqMACShareUserList()异常！");
		}
		return null;
	}
	
	/**
	 * 请求终端周围WiFi列表
	 */
	private ArrayList<HashMap<String, Object>> getTermWiFiList(JSONArray jsonArray) {
		ArrayList<HashMap<String, Object>> list = new ArrayList<HashMap<String, Object>>();
		HashMap<String, Object> map = null;
		int len = jsonArray.length();
	    try {
	    	for (int i=0; i<len; i++) { 
		    	JSONObject obj = (JSONObject) jsonArray.get(i); 
		    	map = new HashMap<String, Object>();
		    	map.put("WiFiSSID", obj.getString("ssid"));
		    	map.put("WiFiLevel", Utils.getWiFiIconResIdByLevel(obj.getInt("signalStrength")));
		    	map.put("WiFiEncryptMode", obj.getInt("encryptMode"));
		    	map.put("WiFiEncryptFormat", obj.getInt("encryptFormat"));
		    	map.put("WiFiEnable", obj.getInt("enable"));
				list.add(map);
	    	}
	    	return list;
		} catch (JSONException e) {
			e.printStackTrace();
			Utils.log("getTermWiFiList()异常！");
		}
		return null;
	}
	
	/**
	 * 发送Handler消息
	 */
	private void sendHandlerMsg(Handler handler, int what, String obj) {
		Message msg = new Message();
		msg.what = what;
		msg.obj = obj;
		handler.sendMessage(msg);
	}
	
	/**
	 * 请求终端录像文件列表
	 */
	private ArrayList<HashMap<String, Object>> getTerminalFileList(JSONArray jsonArray) {
		ArrayList<HashMap<String, Object>> list = new ArrayList<HashMap<String, Object>>();
		HashMap<String, Object> map = null;
		int len = jsonArray.length();
		  
	    try {
	    	for (int i=0; i<len; i++) { 
		    	JSONObject obj = (JSONObject) jsonArray.get(i); 
		    	map = new HashMap<String, Object>();
		    	map.put("fileName", obj.getString("fileName"));
		    	map.put("fileDate", obj.getString("fileEndTime"));

		    	DecimalFormat df = new DecimalFormat("0.00");
		    	double fileSize = obj.getInt("fileSize");
		    	double fileSizeDouble = fileSize/1024;
		    	map.put("fileSize", df.format(fileSizeDouble)+"MB");
		    	map.put("fileSizeInt", ""+obj.getInt("fileSize"));
		    	
				list.add(map);
	    	}
	    	return list;
		} catch (JSONException e) {
			e.printStackTrace();
			Utils.log("getTerminalFileList()异常！");
		}
		return null;
	}
	
	@Override
	public void handleMessage(Message msg) {
		String recvData = (String)msg.obj;
		JSONObject obj = null;
		try {
			obj = new JSONObject(recvData);
			String type = obj.getString("type");
			// 报警消息推送
			if (type.equals("Backstage_message")) {
				Value.isNeedReqAlarmListFlag = true;
				Value.newAlarmMessageCount++;
				int unreadAlarmCount = 0;
				if (MainApplication.getInstance().preferData.isExist("AlarmCount")) {
					unreadAlarmCount = MainApplication.getInstance().preferData.readInt("AlarmCount");
				}
				unreadAlarmCount++;
				MainApplication.getInstance().preferData.writeData("AlarmCount", unreadAlarmCount);
				
				String alarmMac = "";
				if (!obj.isNull("MAC")) {
					alarmMac = obj.getString("MAC");
				}
				// alarmType 0:开关量报警 3:移动侦测 4:遮挡报警 6:敲门事件
				int alarmType = 0;
				if (!obj.isNull("AlarmType")) {
					alarmType = obj.getInt("AlarmType");
				}
				Intent intent = new Intent();
				intent.putExtra("alarmCount", unreadAlarmCount);
				intent.putExtra("alarmMac", alarmMac);
				if (alarmType == 6) {
					intent.putExtra("alarmType", BackstageService.ALARM_TYPE_DOOR);
				} else {
					intent.putExtra("alarmType", BackstageService.ALARM_TYPE_PUSH);
				}
				intent.setAction(BackstageService.BACKSTAGE_MESSAGE_ACTION);
				MainApplication.getInstance().sendBroadcast(intent);
			}
			// 终端上下线消息推送
			else if (type.equals("Backstage_TermActive")) {
				Intent actionIntent = new Intent();
				actionIntent.putExtra("deviceID", obj.getString("MAC"));
				actionIntent.putExtra("dealerName", obj.getString("DealerName"));
				actionIntent.putExtra("isOnline", getState(obj.getInt("Active")));
				actionIntent.setAction(BackstageService.TERM_ONLINE_STATE_ACTION);
				MainApplication.getInstance().sendBroadcast(actionIntent);
			}
			// 客户端和服务器的心跳
			else if (type.equals("Client_BeatHeart")) {
				int resultCode = obj.getInt("Result");
				if (resultCode != 0) {
					Value.isLoginSuccess = false;
					Value.beatHeartFailFlag = true;
					String data = MainApplication.getInstance().generateLoginJson(MainApplication.getInstance().userName, MainApplication.getInstance().userPwd);
					sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
					Utils.log("【正在重新登录...】");
				}
			}
			// 重新登录
			else if ((Value.beatHeartFailFlag) && (type.equals("Client_Login"))) {
				Value.reloginTimes ++;
				if (Value.reloginTimes <= 3) {
					int resultCode = obj.getInt("Result");
					if (resultCode == 0) {
						Value.reloginTimes = 0;
						Value.isLoginSuccess = true;
						Value.beatHeartFailFlag = false;
						Utils.log("【重新登录成功】");
					} else {
						Utils.log("【重新登录失败】");
						String data = MainApplication.getInstance().generateLoginJson(MainApplication.getInstance().userName, MainApplication.getInstance().userPwd);
						sendHandlerMsg(ZmqThread.zmqThreadHandler, R.id.zmq_send_data_id, data);
					}
				}
			}
			// 请求报警数据
			else if ((Value.ownFragmentRequestAlarmFlag) && (type.equals("Client_ReqAlarm"))) {
				int resultCode = obj.getInt("Result");
				if (resultCode == 0) {
					Utils.log("【请求报警数据成功】");
					Value.isNeedReqAlarmListFlag = false;
					
					if (!obj.isNull("AlarmData")) {
						ArrayList<HashMap<String, String>> msgList = new ArrayList<HashMap<String, String>>();
						XmlMessage xmlData = new XmlMessage(MainApplication.getInstance());
						JSONArray jsonArray = obj.getJSONArray("AlarmData");
						msgList = getReqAlarmList(jsonArray);
						if (msgList != null) {
							xmlData.updateList(msgList);
						} else {
							xmlData.deleteAllItem();
						}
					}
					
					MainApplication.getInstance().preferData.writeData("msgRefreshTime", "上次更新于: "+Utils.getNowTime("yyyy-MM-dd HH:mm:ss"));
					MainApplication.getInstance().preferData.writeData("msgRefreshTerminal", "终端: "+Build.MODEL);
				} else {
					Utils.log("【请求报警数据失败】");
				}
				//请求未读报警消息数
				MainApplication.getInstance().requestUnreadAlarmCountEvent();
			}
			// 未读报警消息数
			else if (type.equals("Client_NotReadAlarm")) {
				if (obj.isNull("NotReadCount")) {
					BackstageService.unreadAlarmCount = 0;
				} else {
					BackstageService.unreadAlarmCount = obj.getInt("NotReadCount");
				}
				Utils.log("未读报警消息数: "+BackstageService.unreadAlarmCount);
				Intent intent = new Intent();
				intent.putExtra("alarmCount", BackstageService.unreadAlarmCount);
				intent.putExtra("alarmType", BackstageService.ALARM_TYPE_UNREAD);
				intent.setAction(BackstageService.BACKSTAGE_MESSAGE_ACTION);
				MainApplication.getInstance().sendBroadcast(intent);
			} else {
				// 各个界面下的handler操作
				if (mHandler != null) {
					//realm
					if (type.equals("Client_Getrealm")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							if (!obj.isNull("realm")) {
								Value.realm = obj.getString("realm");
							}
						}
						mHandler.obtainMessage(R.id.get_realm_id, resultCode, 0).sendToTarget();
					}
					//注册
					else if (type.equals("Client_Registration")) {
						int resultCode = obj.getInt("Result");
						mHandler.obtainMessage(R.id.register_id, resultCode, 0).sendToTarget();
					}
					//登录
					else if (type.equals("Client_Login")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							Value.beatHeartFailFlag = false;
						}
						mHandler.obtainMessage(R.id.login_id, resultCode, 0).sendToTarget();
					}
					//重置密码
					else if (type.equals("Client_ResetPwd")) {
						int resultCode = obj.getInt("Result");
						mHandler.obtainMessage(R.id.find_pwd_id, resultCode, 0).sendToTarget();
					}
					//修改密码
					else if (type.equals("Client_ChangePwd")) {
						int resultCode = obj.getInt("Result");
						mHandler.obtainMessage(R.id.modify_pwd_id, resultCode, 0).sendToTarget();
					}
					//添加终端
					else if (type.equals("Client_AddTerm")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							String dealerName = obj.getString("DealerName");
							mHandler.obtainMessage(R.id.add_device_id, resultCode, 0, dealerName).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.add_device_id, resultCode, 0).sendToTarget();
						}
					}
					//请求终端列表
					else if (type.equals("Client_ReqTermList")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							JSONArray jsonArray = obj.getJSONArray("Terminal");
							mHandler.obtainMessage(R.id.request_terminal_list_id, resultCode, 0, getReqTermList(jsonArray)).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.request_terminal_list_id, resultCode, 0,MainApplication.getInstance().getResources().getString(R.string.request_terminal_list_failed)).sendToTarget();
						}
					}
					//修改终端名称
					else if (type.equals("Client_ModifyTerm")) {
						int resultCode = obj.getInt("Result");
						mHandler.obtainMessage(R.id.modify_device_name_id, resultCode, 0).sendToTarget();
					}
					//删除终端绑定
					else if (type.equals("Client_DelTerm")) {
						int resultCode = obj.getInt("Result");
						mHandler.obtainMessage(R.id.delete_device_item_id, resultCode, 0).sendToTarget();
					}
					//请求报警数据
					else if (type.equals("Client_ReqAlarm")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							JSONArray jsonArray = obj.getJSONArray("AlarmData");
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, getReqAlarmList(jsonArray)).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, MainApplication.getInstance().getResources().getString(R.string.request_alarm_data_failed)).sendToTarget();
						}
					}
					//删除该条报警
					else if (type.equals("Client_DelAlarm")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, MainApplication.getInstance().getResources().getString(R.string.failed_to_delete_the_clause_alarm)).sendToTarget();
						}
					}
					//删除当前全部报警
					else if (type.equals("Client_DelSelectAlarm")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, MainApplication.getInstance().getResources().getString(R.string.all_alarms_failed_to_delete_the_current)).sendToTarget();
						}
					}
					//标记该条报警
					else if (type.equals("Client_MarkAlarm")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, MainApplication.getInstance().getResources().getString(R.string.mark_which_alarm_failure)).sendToTarget();
						}
					}
					//标记当前全部报警
					else if (type.equals("Client_MarkSelectAlarm")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.request_alarm_id, resultCode, 0, MainApplication.getInstance().getResources().getString(R.string.mark_all_the_current_alarm_failure)).sendToTarget();
						}
					}
					//上传背景图片
					else if (type.equals("Client_PushBackPic")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							mHandler.obtainMessage(R.id.upload_back_image_id, resultCode, 0, obj.getString("PictureURL")).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.upload_back_image_id, resultCode, 0).sendToTarget();
						}
					}
					//删除背景图片
					else if (type.equals("Client_DelBackPic")) {
						int resultCode = obj.getInt("Result");
						mHandler.obtainMessage(R.id.delete_back_image_id, resultCode, 0).sendToTarget();
					}
					//添加终端分享
					else if (type.equals("Client_AddShareTerm")) {
						int resultCode = obj.getInt("Result");
						mHandler.obtainMessage(R.id.add_device_share_id, resultCode, 0).sendToTarget();
					}
					//请求分享终端列表
					else if (type.equals("Client_ReqShareList")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							JSONArray jsonArray = obj.getJSONArray("Terminal");
							mHandler.obtainMessage(R.id.request_device_share_id, resultCode, 0, getReqTermList(jsonArray)).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.request_device_share_id, resultCode, 0, MainApplication.getInstance().getResources().getString(R.string.share_the_list_of_failed_request)).sendToTarget();
						}
					}
					//删除终端分享
					else if (type.equals("Client_DelShareTerm")) {
						int resultCode = obj.getInt("Result");
						mHandler.obtainMessage(R.id.delete_device_share_id, resultCode, 0).sendToTarget();
					}
					//请求指定终端分享用户列表
					else if (type.equals("Client_ReqMAClist")) {
						int resultCode = obj.getInt("Result");
						if (resultCode == 0) {
							JSONArray jsonArray = obj.getJSONArray("Terminal");
							mHandler.obtainMessage(R.id.requst_device_share_user_id, resultCode, 0, getReqMACShareUserList(jsonArray)).sendToTarget();
						} else {
							mHandler.obtainMessage(R.id.requst_device_share_user_id, resultCode, 0).sendToTarget();
						}
					}
					//接收到终端发回的数据
					else if (type.equals("tunnel")) {
						String resultCode = obj.getString("command");
						if (resultCode.equals("wifi_info")) {
							if (!obj.isNull("wifis")) {
								JSONArray jsonArray = obj.getJSONArray("wifis");
								mHandler.obtainMessage(R.id.requst_wifi_list_id, 0, 0, getTermWiFiList(jsonArray)).sendToTarget();
							} else {
								mHandler.obtainMessage(R.id.requst_wifi_list_id, -1, 0, "null").sendToTarget();
							}
						}
						else if (resultCode.equals("set_wifi")) {
							boolean result = obj.getBoolean("result");//0:失败  1:成功
							if (result) {
								mHandler.obtainMessage(R.id.set_term_wifi_id, 1, 0).sendToTarget();
							} else {
								mHandler.obtainMessage(R.id.set_term_wifi_id, 0, 0).sendToTarget();
							}
						}
						else if (resultCode.equals("query_record")) {
							int totalNum = obj.getInt("totalNum");//录像文件总数
							if (totalNum == 0) {
								mHandler.obtainMessage(R.id.request_terminal_video_list_id, totalNum, 0).sendToTarget();
							} else {
								JSONArray jsonArray = obj.getJSONArray("recordList");
								mHandler.obtainMessage(R.id.request_terminal_video_list_id, totalNum, 0, getTerminalFileList(jsonArray)).sendToTarget();
							}
						}
						else if (resultCode.equals("alarm_status")) {
							Intent intent = new Intent();
							if (!obj.isNull("value")) {
								intent.putExtra("alarmDefence", obj.getInt("value"));
							}
							intent.setAction(PlayerActivity.ALARM_DEFENCE_ACTION);
							MainApplication.getInstance().sendBroadcast(intent);
						}
					}
				}
			}
		} catch (JSONException e) {
			e.printStackTrace();
		}
	}
}
