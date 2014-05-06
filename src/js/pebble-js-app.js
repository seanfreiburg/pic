var origin = '';
var destination = '' ;
var travmode = 'WALKING' ;
var eventhr = 0 ;
var eventmin = 0 ;
var eventampm = 'AM' ; // AM is default
var seconds = 0 ;
var rotation_angle = 0 ;

// Function to send a message to the Pebble using AppMessage API
function sendMessage(init) {
  console.log(rotation_angle);
  console.log("Seconds " +seconds);
  if (init){
    console.log("init!!!!!!!!!")
    Pebble.sendAppMessage({"angle": rotation_angle, "seconds":seconds, "init": true});
  }
  else{
   Pebble.sendAppMessage({"angle": rotation_angle, "seconds":seconds});
  }


  // PRO TIP: If you are sending more than one message, or a complex set of messages,
  // it is important that you setup an ackHandler and a nackHandler and call
  // Pebble.sendAppMessage({ /* Message here */ }, ackHandler, nackHandler), which
  // will designate the ackHandler and nackHandler that will be called upon the Pebble
  // ack-ing or nack-ing the message you just sent. The specified nackHandler will
  // also be called if your message send attempt times out.
}



// Called when incoming message from the Pebble is received
var request_info = function(init) {
                //console.log("Received Status: " + e.payload.status);
  var url = 'http://people.lis.illinois.edu/~csevans2/evenstone2/GetRoute.php?tokenid='+Pebble.getAccountToken();
  var req = new XMLHttpRequest();

  console.log(url) ;

  req.open('GET', url, true);
  req.onload = function() {
    console.log(req);
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        var result = JSON.parse(req.responseText);
        origin = result.origin;
        destination = result.destination;
        travmode = result.travmode ;
        eventhr = result.eventhr ;
        eventmin = result.eventmin ;
        eventampm = result.eventampm ;
        var starthr = eventhr ;
        if (eventhr == 12) starthr = 0 ;
        rotation_angle = starthr*30 + eventmin/2 ;
        var respcode = result.ResponseCode ;
        var respmsg = result.ResponseMsg ;
        // console.log(result) ;
        console.log("RespCode: " + respcode + "\tRespMsg:" + respmsg) ;
        console.log("Origin: " + origin + "\nDestination: " + destination + "\nTravMode: " + travmode + "\nEventTime: " + eventhr + ":" + eventmin + " " + eventampm + "\nBitmapRotation:" + rotation_angle) ;
       // Pebble.sendAppMessage({ "icon":icon, "temperature":temperature + "\u00B0C"});
        console.log("Time to " + travmode + "\n\tbetween " + origin + "\n\tand " + destination + "\n\tis " + getDurationInSecs(origin, destination,init) + "secs") ;
      } else { console.log("Error"); }
    }
  } ;
  req.send(null);
  //sendMessage(rotation_angle);
  };

Pebble.addEventListener("showConfiguration", function() {
  var url = 'http://people.lis.illinois.edu/~csevans2/evenstone2/config.html?token='+Pebble.getAccountToken();
  console.log("showing configuration at " + url);
  Pebble.openURL(url);
  console.log("blah");
  request_info(true);

});




function getDurationInSecs(start, end, init) { //,success_callback) {
  var xmlHttp = null;
  xmlHttp = new XMLHttpRequest();
  var url = "https://maps.googleapis.com/maps/api/directions/json?key=AIzaSyBvbZUMkNxFl5lvHp7U8763z8WsWtmD1Kw&origin="+start+"&destination="+end+"&mode="+travmode+"&sensor=false";
  url=encodeURI(url);
  console.log("getDurationInSecs");
  console.log(url);
  xmlHttp.open( "GET",url ,true);
  xmlHttp.send();
  xmlHttp.onreadystatechange=function(){
    console.log("readyState: "+xmlHttp.readyState);
    console.log("status: " + xmlHttp.status);
  if (xmlHttp.readyState==4 && xmlHttp.status==200)
  {
      var result = JSON.parse(xmlHttp.responseText) ;
      console.log(result) ;
      seconds = JSON.parse(xmlHttp.responseText)['routes'][0]['legs'][0]['duration']['value'] ;
      console.log(seconds) ;
      sendMessage(init);
    }
  };

} ;


Pebble.addEventListener("appmessage",
  function(e) {
    request_info(false);
  }
);

Pebble.addEventListener("ready",
  function(e) {
    console.log("JavaScript app ready and running!");
  }
);
