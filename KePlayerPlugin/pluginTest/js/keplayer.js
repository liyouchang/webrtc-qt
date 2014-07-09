var kePlayerObj ;
var g_peer_id;
var g_infoDiv;

function load(){
    var infoDiv = document.getElementById("textDiv");
    g_infoDiv = document.getElementById("textDiv");
    kePlayerObj = document.getElementById("KePlayerPlugin");
    kePlayerObj.TunnelOpened = function (peer){
        infoDiv.textContent = "TunnelOpened "+ peer;
    }
    kePlayerObj.TunnelClosed = function (peer){
        infoDiv.textContent = "TunnelClosed "+ peer;
    }
    kePlayerObj.RecvPeerMsg = function (peer,msg){
        infoDiv.textContent = "RecvPeerMsg "+ peer + " msg "+ msg;
    }
    kePlayerObj.RecordStatus = function(peer,status){
        infoDiv.textContent = "RecordStatus "+ peer + " status-"+ status;
    }
    var iceServers = [{"uri":"stun:192.168.40.192:5389"},{"uri":"turn:192.168.40.192:5766"}];
    var strIceServers = JSON.stringify(iceServers);
    var strRouterUrl = "tcp://192.168.40.192:5555"
    kePlayerObj.Initialize(strRouterUrl,strIceServers);
    eventFunction();


}

window.onbeforeunload = function (e){
    console.log("unload");
    kePlayerObj.DestroyAll();
    //return false;
}

function CheckOpen(peerId){
    var opend = kePlayerObj.IsTunnelOpened(peerId);
    g_infoDiv.textContent = opend;

}

function SetDivision(num){
    KePlayerPlugin.SetDivision(num);
}

var eventFunction = function(){
//IE should use these functions
//    function kePlayerObj::TunnelOpened(peer) {
//        kePlayerObj.TunnelOpened(peer);
//    }
//    function kePlayerObj::TunnelClosed(peer) {
//        kePlayerObj.TunnelClosed(peer);
//    }
//    function kePlayerObj::RecvPeerMsg(peer,msg) {
//        kePlayerObj.RecvPeerMsg(peer,msg);
//    }
//    function kePlayerObj::RecordStatus(peer,status){
//        kePlayerObj.RecordStatus(peer,status);
//    }
};

function QueryCameraRemoteFile(){
    g_peer_id =  PeerID.value;
    var query_command = {type:"tunnel",command:"query_record",
        condition:{startTime:"20130417175100",
            endTime:"20140618175500",offset:0,toQuery:3}};
    var query_str = JSON.stringify(query_command);
    kePlayerObj.SendCommand(g_peer_id,query_str);
}

function PlayCameraRemoteFile(){
    g_peer_id =  PeerID.value;
    //"/mnt/sd/20140428/video/01/104953_1.avi",
    var record_list =
            [{ "fileDate" : "20140428104953",
                 "fileName" : "/mnt/sd/20140428/video/01/104953_1.avi",
                 "fileSize" : 12136382 },
             { "fileDate" : "20140428104750",
                 "fileName" : "/mnt/sd/20140428/video/01/104750_1.avi",
                 "fileSize" : 12323578 }];
    var record_str = JSON.stringify(record_list);
    kePlayerObj.PlayRecordFiles(g_peer_id,record_str);
}

function GetWifiList(){
    g_peer_id =  PeerID.value;
    var wifiInfo =
            {"type":"tunnel",
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


