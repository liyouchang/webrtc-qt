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

public class XmlDevice {

	public Context mContext;
	private static String filePath = "";
	private static String ItemNode = "node";
	
	public XmlDevice (Context context) {
		this.mContext = context;
		init();
	}
	
	/**
	 * 初始化xml文件
	 */
	public void init() {
		filePath = mContext.getFilesDir().getPath() + File.separator + "DeviceList.xml";
//			filePath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "KaerVideo" + File.separator + "DeviceList.xml";
		File file = new File(filePath);
		if (!file.exists()) {
			try {
				file.createNewFile();
				initXmlFile(file);
			} catch (IOException e) {
				e.printStackTrace();
				Utils.log("init() abnormal！");
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
			serializer.startTag("", "DeviceList");
			serializer.endTag("", "DeviceList");
			serializer.endDocument();
			OutputStream os = new FileOutputStream(file);
			OutputStreamWriter ow = new OutputStreamWriter(os);
			ow.write(writer.toString());
			ow.close();
			os.close();
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("initXmlFile() abnormal！");
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
			Utils.log("loadInit() abnormal！");
		}
		return null;
	}
	
	/**
	 * 增加一个List列表
	 * @return true: 增加成功  false: 增加失败
	 */
	public synchronized boolean writeList(ArrayList<HashMap<String, String>> list) {
		
		int len = list.size();
		try {
			for (int i=0; i<len; i++) {
				writeItem(list.get(i));
			}
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("addList() abnormal ！");
		}
		return false;
	}

	/**
	 * 增加一个Item节点
	 * @return true: 增加成功  false: 增加失败
	 */
	public synchronized boolean writeItem(HashMap<String, String> map) {

		try {
			Document document = loadInit(filePath);
			Element itemElement = (Element) document.createElement(ItemNode);
			
			Element nameElement = (Element) document.createElement("name");
			Element idElement = (Element) document.createElement("id");
			Element stateElement = (Element) document.createElement("state");
			Element dealerNameElement = (Element) document.createElement("dealer");
			Element bgElement = (Element) document.createElement("bg");
			Element linkElement = (Element) document.createElement("link");
			Element clarityElement = (Element) document.createElement("clarity");
			
			Text nameText = document.createTextNode(map.get("deviceName"));
			nameElement.appendChild(nameText);
			Text idText = document.createTextNode(map.get("deviceID"));
			idElement.appendChild(idText);
			Text stateText = document.createTextNode(map.get("isOnline"));
			stateElement.appendChild(stateText);
			Text dealerNameText = document.createTextNode(map.get("dealerName"));
			dealerNameElement.appendChild(dealerNameText);
			Text bgText = document.createTextNode(map.get("deviceBg"));
			bgElement.appendChild(bgText);
			Text linkText = document.createTextNode(map.get("LinkState"));
			linkElement.appendChild(linkText);
			Text clarityText = document.createTextNode(map.get("playerClarity"));
			clarityElement.appendChild(clarityText);
			
			itemElement.appendChild(nameElement);
			itemElement.appendChild(idElement);
			itemElement.appendChild(stateElement);
			itemElement.appendChild(dealerNameElement);
			itemElement.appendChild(bgElement);
			itemElement.appendChild(linkElement);
			itemElement.appendChild(clarityElement);
			Element rootElement = (Element) document.getDocumentElement();
			rootElement.appendChild(itemElement);
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("addItem() abnormal！");
		}
		return false;
	}
	
	/**
	 * 是否存在一个Item节点
	 * @return true: 存在  false: 不存在
	 */
	public synchronized boolean isItemExist(String deviceID) {

		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(deviceID)) {
					return true;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("isItemExit() abnormal！");
		}
		return false;
	}

	/**
	 * 删除一个Item节点
	 * @return true: 删除成功  false: 删除失败
	 */
	public synchronized boolean deleteItem(String deviceID) {

		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
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
			Utils.log("deleteItem() abnormal！");
		}
		return false;
	}
	
	/**
	 * 删除一个Item节点的背景
	 * @return true: 删除成功  false: 删除失败
	 */
	public synchronized boolean deleteItemBg(String mac) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(mac)) {
					document.getElementsByTagName("bg").item(i).getFirstChild().setNodeValue("null");
					break;
				}
			}
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("deleteItemBg() abnormal！");
		}
		return false;
	}

	/**
	 * 删除所有节点 *
	 * @return true: 删除成功  false: 删除失败
	 */
	public synchronized boolean deleteAllItem() {
		try {
			File file = new File(filePath);
			initXmlFile(file);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("MyDebug: deleteAllItem()abnormal！");
		}
		return false;
	}
	
	/**
	 * 获得列表的大小
	 */
	public synchronized int getListSize() {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			return nodeList.getLength();
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("getListSize() abnormal！");
		}
		return 0;
	}
	
	/**
	 * 更新一个List列表
	 * @return true: 更新成功  false: 更新失败
	 */
	public synchronized boolean updateList(ArrayList<HashMap<String, String>> list) {
		try {
			int listSize = list.size();
			if (listSize == 0) {
				return false;
			}
			ArrayList<HashMap<String, String>> xmlList = readXml();
			if (xmlList.size() == 0) {
				deleteAllItem();
				writeList(list);
				return true;
			} else {
				deleteAllItem();
			}
			ArrayList<HashMap<String, String>> newList = new ArrayList<HashMap<String, String>>();
			for (int i=0; i<listSize; i++) {
				boolean isExistItem = false;
				String id = list.get(i).get("deviceID").trim();
				for (int j=0; j<xmlList.size(); j++) {
					String xmlId = xmlList.get(j).get("deviceID").trim();
					if (xmlId.equals(id)) {
						isExistItem = true;
						HashMap<String, String> item = new HashMap<String, String>();
						item.put("deviceName", list.get(i).get("deviceName"));
						item.put("deviceID", list.get(i).get("deviceID"));
						item.put("isOnline", list.get(i).get("isOnline"));
						item.put("dealerName", list.get(i).get("dealerName"));
						item.put("deviceBg", list.get(i).get("deviceBg"));
						item.put("LinkState", list.get(i).get("LinkState"));
						item.put(DeviceValue.HASH_PLAYER_CLARITY, xmlList.get(i).get(DeviceValue.HASH_PLAYER_CLARITY));
						newList.add(item);
						break;
					}
				}
				if (!isExistItem) {
					newList.add(list.get(i));
				}
			}
			writeList(newList);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("updateItem() abnormal！");
		}
		return false;
	}
	
	/**
	 * 更新一个Item节点的状态
	 * @return true: 更新成功  false: 更新失败
	 */
	public synchronized boolean updateItemState(String mac, String state, String dealerName) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(mac)) {
					document.getElementsByTagName("state").item(i).getFirstChild().setNodeValue(state);
					document.getElementsByTagName("dealer").item(i).getFirstChild().setNodeValue(dealerName);
					break;
				}
			}
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("updateItem() abnormal！");
		}
		return false;
	}
	
	
	/**
	 * 更新一个Item节点的名称
	 * @return true: 更新成功  false: 更新失败
	 */
	public synchronized boolean updateItemName(String mac, String newName) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(mac)) {
					document.getElementsByTagName("name").item(i).getFirstChild().setNodeValue(newName);
					break;
				}
			}
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("updateItem() abnormal！");
		}
		return false;
	}
	
	/**
	 * 改变终端播放的清晰度
	 */
	public synchronized boolean changePlayerClarity(String mac, int clarity) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(mac)) {
					document.getElementsByTagName(DeviceValue.XML_PLAYER_CLARITY).item(i).getFirstChild().setNodeValue(String.valueOf(clarity));
					break;
				}
			}
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("updateItem() abnormal！");
		}
		return false;
	}
	
	/**
	 * 更新一个Item节点的背景的Url
	 * @return true: 更新成功  false: 更新失败
	 */
	public synchronized boolean updateItemBg(String mac, String bg) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(mac)) {
					document.getElementsByTagName("bg").item(i).getFirstChild().setNodeValue(bg);
					break;
				}
			}
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("updateItemBg() abnormal！");
		}
		return false;
	}

	/**
	 * 更新一个Item节点
	 * @return true: 更新成功  false: 更新失败
	 */
	public synchronized boolean updateItem(HashMap<String, String> map) {
		Document document = loadInit(filePath);
		try {
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String id = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (id.equals(map.get("deviceID"))) {
					document.getElementsByTagName("name").item(i).getFirstChild().setNodeValue(map.get("deviceName"));
					document.getElementsByTagName("state").item(i).getFirstChild().setNodeValue(map.get("isOnline"));
					document.getElementsByTagName("dealer").item(i).getFirstChild().setNodeValue(map.get("dealerName"));
					document.getElementsByTagName("bg").item(i).getFirstChild().setNodeValue(map.get("deviceBg"));
					document.getElementsByTagName("link").item(i).getFirstChild().setNodeValue(map.get("LinkState"));
					break;
				}
			}
			writeXML(document, filePath);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("updateItem() abnormal！");
		}
		return false;
	}
	
	/**
	 * 获得一个节点的信息
	 */
	private synchronized HashMap<String, String> getNodeHashMap(Document document, int index) {
		HashMap<String, String> item = new HashMap<String, String>();
		try {
			String name = document.getElementsByTagName("name").item(index).getFirstChild().getNodeValue();
			String id = document.getElementsByTagName("id").item(index).getFirstChild().getNodeValue();
			String state = document.getElementsByTagName("state").item(index).getFirstChild().getNodeValue();
			String dealer = document.getElementsByTagName("dealer").item(index).getFirstChild().getNodeValue();
			String bg = document.getElementsByTagName("bg").item(index).getFirstChild().getNodeValue();
			String link = document.getElementsByTagName("link").item(index).getFirstChild().getNodeValue();
			String clarity = document.getElementsByTagName(DeviceValue.XML_PLAYER_CLARITY).item(index).getFirstChild().getNodeValue();
			item.put("deviceName", name);
			item.put("deviceID", id);
			item.put("isOnline", state);
			item.put("dealerName", dealer);
			item.put("deviceBg", bg);
			item.put("LinkState", link);
			item.put(DeviceValue.HASH_PLAYER_CLARITY, clarity);
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("getNodeHashMap() abnormal！");
		}
		return item;
	}
	
	/**
	 * 获取XML文件的一个节点
	 * @return 成功返回一个ArrayList列表的一个节点  失败返回null
	 */
	public synchronized HashMap<String, String> readItem(String mac) {
		try {
			Document document = loadInit(filePath);
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String searchId = document.getElementsByTagName("id").item(i).getFirstChild().getNodeValue();
				if (searchId.equals(mac)) {
					HashMap<String, String> item = new HashMap<String, String>();
					item = getNodeHashMap(document, i);
					return item;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("readItem() abnormal！");
		}
		return null;
	}
	
	/**
	 * 获取XML文件在线的节点
	 * @return 成功返回一个ArrayList列表的一个节点  失败返回null
	 */
	public synchronized ArrayList<HashMap<String, String>> getOnlineList() {
		try {
			ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();
			Document document = loadInit(filePath);
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				String state = document.getElementsByTagName("state").item(i).getFirstChild().getNodeValue();
				if (state.equals("true")) {
					HashMap<String, String> item = new HashMap<String, String>();
					item = getNodeHashMap(document, i);
					list.add(item);
				}
			}
			if (list.size() != 0) {
				return list;
			} else {
				return null;
			}
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("getItem() abnormal！");
		}
		return null;
	}

	/**
	 * 保存document到XML文件
	 * @return true: 保存成功  false: 保存失败
	 */
	public synchronized boolean writeXML(Document document, String filePath) {
		try {
			TransformerFactory tFactory = TransformerFactory.newInstance();
			Transformer transformer = tFactory.newTransformer();
			DOMSource source = new DOMSource(document);
			StreamResult result = new StreamResult(new File(filePath));
			transformer.transform(source, result);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("writeXML() abnormal！");
		}
		return false;
	}

	/**
	 * 获取XML文件的所有节点
	 * @return 成功返回一个ArrayList列表  失败返回null
	 */
	public synchronized ArrayList<HashMap<String, String>> readXml() {
		ArrayList<HashMap<String, String>> list = new ArrayList<HashMap<String, String>>();
		try {
			Document document = loadInit(filePath);
			NodeList nodeList = document.getElementsByTagName(ItemNode);
			int len = nodeList.getLength();
			for (int i=0; i<len; i++) {
				HashMap<String, String> item = new HashMap<String, String>();
				item = getNodeHashMap(document, i);
				list.add(item);
			}
		} catch (Exception e) {
			e.printStackTrace();
			Utils.log("readXml() abnormal！");
			return null;
		}
		return list;
	}
}
