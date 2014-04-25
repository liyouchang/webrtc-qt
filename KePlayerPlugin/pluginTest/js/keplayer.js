var kePlayerObj ;
var g_peer_id;
function load(){
    var infoDiv = document.getElementById("textDiv");

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
    eventFunction();
}

function SetDivision(num){

    KePlayerPlugin.SetDivision(num);
}

var eventFunction = function(){
    function kePlayerObj::TunnelOpened(peer) {
        kePlayerObj.TunnelOpened(peer);
    }
    function kePlayerObj::TunnelClosed(peer) {
        kePlayerObj.TunnelClosed(peer);
    }
    function kePlayerObj::RecvPeerMsg(peer,msg) {
        kePlayerObj.RecvPeerMsg(peer,msg);
    }

};

function QueryCameraRemoteFile(){

   g_peer_id =  PeerID.value;
    var query_command = {type:"tunnel",command:"query_record",
                condition:{start_time:"20140417175100",
            end_time:"20140418175500",offset:0,to_query:30}};
    var query_str = JSON.stringify(query_command);
    kePlayerObj.SendCommand(g_peer_id,query_str);

}

function PlayCameraRemoteFile(){
    g_peer_id =  PeerID.value;
    var record_list =
            [{"file_name":"testfile1","file_date":"20140417175100","file_size":256},
            {"file_name":"testfile2","file_date":"20140417175100","file_size":256}];
    var record_str = JSON.stringify(record_list);

    kePlayerObj.PlayRecordFiles(g_peer_id,record_str);
}


