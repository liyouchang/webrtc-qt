var kePlayerObj ;

function load(){
    var infoDiv = document.getElementById("textDiv");

    kePlayerObj = document.getElementById("KePlayerPlugin");
    kePlayerObj.TunnelOpened = function (peer){
        infoDiv.textContent = "TunnelOpened "+ peer;
    }
    kePlayerObj.TunnelClosed = function (peer){
        infoDiv.textContent = "TunnelClosed "+ peer;
    }
    eventFunction();
}

var eventFunction = function(){
    function kePlayerObj::TunnelOpened(peer) {
        kePlayerObj.TunnelOpened(peer);
    }
    function kePlayerObj::TunnelClosed(peer) {
        kePlayerObj.TunnelClosed(peer);
    }

};


