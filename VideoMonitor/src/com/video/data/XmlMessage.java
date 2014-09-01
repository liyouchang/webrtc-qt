package com.video.data;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.HashMap;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;
import org.xmlpull.v1.XmlSerializer;

import android.content.Context;
import android.util.Xml;

import com.video.utils.Utils;

public class XmlMessage {

	private static String filePath = "";
	public Context context;
	
	public XmlMessage (Context context) {
		this.context = context;
		init();
	}
	
	/**
	 * 初始化xml文件
	 */
	public void init() {
		if (Utils.checkSDCard()) {
			filePath = context.getFilesDir().getPath() + File.separator + "MessageList.xml";
			File file = new File(filePath);
			if (!file.exists()) {
				try {
					file.createNewFile();
					initXmlFile(file);
				} catch (IOException e) {
					e.printStackTrace();
					System.out.println("MyDebug: init()异常！");
				}
			}
		}
	}

	/**
	 * 初始化创建后的xml文件
	 */
	public void initXmlFile(File file) {
		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			serializer.startTag("", "MessageList");
			serializer.endTag("", "MessageList");
			serializer.endDocument();
			OutputStream os = new FileOutputStream(file);
			OutputStreamWriter ow = new OutputStreamWriter(os);
			ow.write(writer.toString());
			ow.close();
			os.close();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: initXmlFile()异常！");
		}
	}

	/**
	 * 通过传入的文件路径，返回一个document对象
	 */
	public Document loadInit(String path) {
		Document document = null;
		try {
			DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
			DocumentBuilder builder = factory.newDocumentBuilder();
			document = builder.parse(new File(path));
			document.normalize();
			return document;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: loadInit()异常！");
		}
		return null;
	}
	
	/**
	 * 增加一个List列表
	 * @return true: 增加成功  false: 增加失败
	 */
	public boolean addList(ArrayList<HashMap<String, String>> list) {
		
		int len = list.size();
		try {
			for (int i=0; i<len; i++) {
				addItem(list.get(i));
			}
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: addList()异常！�");
		}
		return false;
	}

	/**
	 * 增加一个Item节点
	 * @return true: 增加成功  false: 增加失败
	 */
	public boolean addItem(HashMap<String, String> map) {

		try {
			Document document = loadInit(filePath);
			Element itemElement = (Element) document.createElement("item");
			Element timeElement = (Element) document.createElement("time");
			Element macElement = (Element) document.createElement("mac");
			Element stateElement = (Element) document.createElement("state");
			Element eventElement = (Element) document.createElement("event");
			Element urlElement = (Element) document.createElement("url");
			Element idElement = (Element) document.createElement("id");
			
			Text timeText = document.createTextNode(map.get("msgTime"));
			timeElement.appendChild(timeText);
			Text macText = document.createTextNode(map.get("msgMAC"));
			macElement.appendChild(macText);
			Text stateText = document.createTextNode(map.get("isReaded"));
			stateElement.appendChild(stateText);
			Text eventText = document.createTextNode(map.get("msgEvent"));
			eventElement.appendChild(eventText);
			Text urlText = document.createTextNode(map.get("imageURL"));
			urlElement.appendChild(urlText);
			Text idText = document.createTextNode(map.get("msgID"));
			idElement.appendChild(idText);
			
			itemElement.appendChild(timeElement);
			itemElement.appendChild(macElement);
			itemElement.appendChild(stateElement);
			itemElement.appendChild(eventElement);
			itemElement.appendChild(urlElement);
			itemElement.appendChild(idElement);
			Element rootElement = (Element) document.getDocumentElement();
			rootElement.appendChild(itemElement);
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: addItem()异常！");
		}
		return false;
	}
	
	/**
	 * 是否存在一个Item节点
	 * @return true: 存在  false: 不存在
	 */
	public boolean isItemExit(String deviceID) {

		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName("item");
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(deviceID)) {
					return true;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: isItemExit()异常！");
		}
		return false;
	}

	/**
	 * 删除一个Item节点
	 * @return true: 删除成功  false: 删除失败
	 */
	public boolean deleteItem(String deviceID) {

		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName("item");
			int len = nodeList.getLength();
			for (int i = 0; i < len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(deviceID)) {
					Node node = nodeList.item(i);
					node.getParentNode().removeChild(node);
					writeXML(document, filePath);
					break;
				}
			}
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: deleteItem()异常！");
		}
		return false;
	}

	/**
	 * 删除所有节点 *
	 * @return true: 删除成功  false: 删除失败
	 */
	public boolean deleteAllItem() {
		try {
			File file = new File(filePath);
			initXmlFile(file);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: deleteAllItem()异常！");
		}
		return false;
	}
	
	/**
	 * 更新一个List列表
	 * @return true: 更新成功  false: 更新失败
	 */
	public boolean updateList(ArrayList<HashMap<String, String>> list) {
		
		try {
			deleteAllItem();
			addList(list);
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: updateItem()异常！");
		}
		return false;
	}
	
	/**
	 * 更新一个Item节点的状态
	 * @return true: 更新成功  false: 更新失败
	 */
	public boolean updateItemState(String id, String state) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName("item");
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String xmlID = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (xmlID.equals(id)) {
					document.getElementsByTagName("state").item(i).getFirstChild().setNodeValue(state);
					break;
				}
			}
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: updateItem()异常！");
		}
		return false;
	}

	/**
	 * 更新一个Item节点
	 * @return true: 更新成功  false: 更新失败
	 */
	public boolean updateItem(HashMap<String, String> map) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName("item");
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(map.get("msgID"))) {
					document.getElementsByTagName("mac").item(i).getFirstChild().setNodeValue(map.get("msgMAC"));
					document.getElementsByTagName("time").item(i).getFirstChild().setNodeValue(map.get("msgTime"));
					document.getElementsByTagName("state").item(i).getFirstChild().setNodeValue(map.get("isReaded"));
					document.getElementsByTagName("event").item(i).getFirstChild().setNodeValue(map.get("msgEvent"));
					document.getElementsByTagName("url").item(i).getFirstChild().setNodeValue(map.get("imageURL"));
					break;
				}
			}
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: updateItem()异常！");
		}
		return false;
	}
	
	/**
	 * 获得节点是否已标记的状态
	 */
	public boolean getItemState(int id) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName("item");
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String xmlID = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (Integer.parseInt(xmlID.trim()) == id) {
					String isReaded = document.getElementsByTagName("state").item(i).getFirstChild().getNodeValue();
					if ("true".equals(isReaded.trim())) {
						return true;
					}
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: getItemState()异常！");
		}
		return false;
	}
	
	/**
	 * 获得列表的大小
	 */
	public int getListSize() {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName("item");
			return nodeList.getLength();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: getListSize()异常！");
		}
		return 0;
	}
	
	/**
	 * @return 返回获取报警数据ID的最大值
	 */
	public int getMaxUpdateID() {
		int result = -1;
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName("item");
			int len = nodeList.getLength();
			int[] idArray = new int[len];
			
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				idArray[i] = Integer.parseInt(id);
			}
			idArray = Utils.bubbleSortIntArray(idArray);
			return idArray[0];
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: getDownUpdateID()异常！");
		}
		return result;
	}
	
	/**
	 * @return 返回获取报警数据ID的最小值
	 */
	public int getMinUpdateID() {
		int result = -1;
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName("item");
			int len = nodeList.getLength();
			int[] idArray = new int[len];
			
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				idArray[i] = Integer.parseInt(id);
			}
			idArray = Utils.bubbleSortIntArray(idArray);
			return idArray[len-1];
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: getDownUpdateID()异常！");
		}
		return result;
	}

	/**
	 * 保存document到XML文件
	 * @return true: 保存成功  false: 保存失败
	 */
	public boolean writeXML(Document document, String filePath) {
		try {
			TransformerFactory tFactory = TransformerFactory.newInstance();
			Transformer transformer = tFactory.newTransformer();
			DOMSource source = new DOMSource(document);
			StreamResult result = new StreamResult(new File(filePath));
			transformer.transform(source, result);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: writeXML()异常！");
		}
		return false;
	}

	/**
	 * 获取XML文件的所有节点
	 * @return 成功返回一个ArrayList列表  失败返回null
	 */
	public ArrayList<HashMap<String, String>> readXml() {
		ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();
		try {
			Document document = loadInit(filePath);
			NodeList nodeList = document.getElementsByTagName("item");
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				HashMap<String, String> item = new HashMap<String, String>();
				String time = document.getElementsByTagName("time").item(i).getFirstChild().getNodeValue();
				String mac = document.getElementsByTagName("mac").item(i).getFirstChild().getNodeValue();
				String state = document.getElementsByTagName("state").item(i).getFirstChild().getNodeValue();
				String event = document.getElementsByTagName("event").item(i).getFirstChild().getNodeValue();
				String url = document.getElementsByTagName("url").item(i).getFirstChild().getNodeValue();
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				item.put("msgTime", time);
				item.put("msgMAC", mac);
				item.put("isReaded", state);
				item.put("msgEvent", event);
				item.put("imageURL", url);
				item.put("msgID", id);
				list.add(item);
			}
			return list;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: readXml()异常！");
		}
		return null;
	}
}
