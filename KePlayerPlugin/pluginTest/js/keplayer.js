var kePlayerObj ;
var g_peer_id;
var g_infoDiv;

function load(){

    var infoDiv = document.getElementById("textDiv");
    g_infoDiv = document.getElementById("textDiv");
    kePlayerObj = document.getElementById("KePlayerPlugin");

    kePlayerObj.TunnelOpened = function (peer) {
        infoDiv.textContent = "TunnelOpened "+ peer;
    }

    kePlayerObj.TunnelClosed = function (peer) {
        infoDiv.textContent = "TunnelClosed "+ peer;
    }

    kePlayerObj.RecvPeerMsg = function (peer,msg) {
        infoDiv.textContent = "RecvPeerMsg "+ peer + " msg "+ msg;
    }

    kePlayerObj.RecordStatus = function(peer,status) {
        //infoDiv.textContent = "RecordStatus "+ peer + " status-"+ status;
    }

    kePlayerObj.LocalDeviceInfo = function(devInfo) {
        infoDiv.textContent = devInfo;
    }

//    var iceServers = [{"uri":"stun:192.168.40.179:5389"},
//                      {"uri":"turn:192.168.40.179:5766","username":"lht1","password":"1234567"}];

    var iceServers = [{"uri":"stun:192.168.40.179:5389"}];
    var strIceServers = JSON.stringify(iceServers);
//    var strRouterUrl = "tcp://222.174.213.181:5555"
    var strRouterUrl = "tcp://192.168.40.179:5555"
    kePlayerObj.Initialize(strRouterUrl,strIceServers);
    eventFunction();

}

window.onbeforeunload = function (e){
    console.log("unload");
    kePlayerObj.DestroyAll();
    //return false;
}

function SetSize(){
    console.log("SetSize");
    kePlayerObj.style.height = "300px";
    kePlayerObj.style.width = "300px";
}

function CheckOpen(peerId){
    var opend = kePlayerObj.IsTunnelOpened(peerId);
    g_infoDiv.textContent = opend;

}

function SetDivision(num){
    KePlayerPlugin.SetDivision(num);
}

function SetPosition(){

    var pos = parseInt(playPosition.value);
    console.log(pos);

    KePlayerPlugin.SetPlayPosition(PeerID.value,parseInt(Position.value))
}

var eventFunction = function(){
//IE should use these functions
    function kePlayerObj::TunnelOpened(peer) {
        kePlayerObj.TunnelOpened(peer);
    }
    function kePlayerObj::TunnelClosed(peer) {
        kePlayerObj.TunnelClosed(peer);
    }
    function kePlayerObj::RecvPeerMsg(peer,msg) {
        kePlayerObj.RecvPeerMsg(peer,msg);
    }
    function kePlayerObj::RecordStatus(peer,status){
        kePlayerObj.RecordStatus(peer,status);
    }
    function kePlayerObj::LocalDeviceInfo(devInfo){
        kePlayerObj.LocalDeviceInfo(devInfo);
    }
    function kePlayerObj::ResizeToNormal(devInfo){
        SetSize();
    }

};

function QueryCameraRemoteFile(){
    g_peer_id =  PeerID.value;
    var query_command = {type:"tunnel",command:"query_record",
        condition:{startTime:"2013/04/17 17:51:00",
            endTime:"2014/09/18 17:55:00",offset:0,toQuery:3}};
    var query_str = JSON.stringify(query_command);
    kePlayerObj.SendCommand(g_peer_id,query_str);
}

function PlayCameraRemoteFile(){
    g_peer_id =  PeerID.value;
    //"/mnt/sd/20140428/video/01/104953_1.avi",
//    var record_list =
//            [{ "fileDate" : "20140428104953",
//                 "fileName" : "/mnt/sd/20140428/video/01/104953_1.avi",
//                 "fileSize" : 12136382 },
//             { "fileDate" : "20140428104750",
//                 "fileName" : "/mnt/sd/20140428/video/01/104750_1.avi",
//                 "fileSize" : 12323578 }];
//    var record_str = JSON.stringify(record_list);
    var recordFile = "/mnt/sd/20140901/video/01/103028_1.avi";
    kePlayerObj.PlayRecordFile(g_peer_id,recordFile);
}

function GetWifiList(){
    g_peer_id =  PeerID.value;
    var wifiInfo = {
        "type":"tunnel",
        "command":"wifi_info"
    };
    var wifiInfoStr = JSON.stringify(wifiInfo);
    kePlayerObj.SendCommand(g_peer_id,wifiInfoStr);
}

function CapturePictur(){
    g_peer_id =  PeerID.value;
    var retStr = kePlayerObj.Capture(g_peer_id);
    var infoDiv = document.getElementById("textDiv");
    infoDiv.textContent = "CapturePictur " + g_peer_id + " return " + retStr;
}
function GetCaptureDirList(){
    g_peer_id = PeerID.value;
    var retStr = kePlayerObj.GetSaveDirList("CaptureFiles/2014-05-20");
    g_infoDiv.textContent = retStr;
}
function GetCaptureFileList(){
    g_peer_id = PeerID.value;
    var retStr = kePlayerObj.GetSaveFileList("CaptureFiles/2014-05-20");
    g_infoDiv.textContent = retStr;
}

function GetCaptureFileData(){
    g_peer_id = PeerID.value;
    var retStr = kePlayerObj.GetSaveFileData(
                "CaptureFiles/2014-05-20/0090B01AF67F_2014-05-20_13-36-37-012.bmp",
                200,150);
    g_infoDiv.textContent = retStr;
}

function StartCut(){
    g_peer_id = PeerID.value;
    var retStr = kePlayerObj.StartCut(g_peer_id);
    g_infoDiv.textContent = retStr;
}

function GetLocalDirPath(){
    var retStr = kePlayerObj.GetLocalPath();
    g_infoDiv.textContent = retStr;
}

function Echo(){
    g_peer_id =  PeerID.value;
    var echo =
    {"type":"tunnel",
     "command":"echo"
    };
    var str = JSON.stringify(echo);
    kePlayerObj.SendCommand(g_peer_id,str);
}


function Ptz(side){
    g_peer_id =  PeerID.value;
    var ptz ={
                "type":"tunnel",
                "command":"ptz",
                "control":side,
                "param":10
            };
    var str = JSON.stringify(ptz);
    console.log(str);
    kePlayerObj.SendCommand(g_peer_id,str);
}
function ArmingStatus(){
    g_peer_id =  PeerID.value;
    var ptz ={
                "type":"tunnel",
                "command":"arming_status",
                "value":1
            };
    var str = JSON.stringify(ptz);
    console.log(str);
    kePlayerObj.SendCommand(g_peer_id,str);
}

