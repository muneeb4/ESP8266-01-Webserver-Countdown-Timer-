#include <ArduinoJson.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <AsyncPrinter.h>
#include <async_config.h>
#include <DebugPrintMacros.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncTCPbuffer.h>
#include <SyncClient.h>
#include <tcp_axtls.h>
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>
//#include <WiFi.h>
//#include <AsyncTCP.h>
//#include <ESP8266WiFi.h>
//Replace with your network credentials
const char* ssid = "ESPWiFi";
const char* password = "123456";
const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "value";
String buttonValue= "false";
String timerSliderValue = "15";
const long interval=1000UL;
const long sec=60UL;
const int output = 2;
unsigned long secsCount=0; 
unsigned  long count=timerSliderValue.toInt();
unsigned long minsCount=timerSliderValue.toInt();
unsigned long previousMillis=0;
//unsigned long durMS ;
//unsigned long durSS;
//unsigned long durHH;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
    <html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
<title>Countdown Timer</title>
<style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.4rem;}
    p {font-size: 2.2rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .slidecontainer {width: 100%;}
    .slider1 { -webkit-appearance: none; margin: 14px; width: 400px; height: 20px; border-radius: 10px; background: #c1c1c1; outline: none; opacity:0.7; -webkit-transition: .2s; transition: opacity 2s;}
    .slider1:hover { opacity:1; }
    .slider1::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 25px; height: 45px; border-radius: 20px; background:#2c3e50; cursor: pointer;}
    .slider1::-moz-range-thumb {width:30px; height: 30px; background: #2c3e50; cursor: pointer;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px;}
    .switch input {display: none}
    .slider {position: absolute;cursor: pointer;top:0; left:0; right:0; bottom:0; background-color: #ca2222; -webkit-transition: .4s; transition: .4s; border-radius: 34px;}
    .slider:before { position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: white; -webkit-transition: .4s; transition: .4s; border-radius: 68px;}
     input:checked + .slider {background-color: #2ab934;}
     input:focus + .slider {box-shadow: 0 0 1px #2196F3;}
    input:checked + .slider:before { -webkit-transform: translateX(52px);-ms-transform: translateX(52px) ;transform: translateX(52px);}
    .slider:after{ content: 'OFF'; color: white; display: block; position: absolute; transform: translate(-10px,-10px);top: 35px;left: 85px; font-size: 15px; font-family: Verdana, sans-serif;}
    input:checked + .slider:after{ content: 'ON';left: 35px;}
    .slider.round{border-radius: 68px;}
    .slider.round:before {border-radius: 68px;}
    .progress-ring {}
    .progress-ring__circle {transition: 0.35s stroke-dashoffset;transform: rotate(-90deg);transform-origin: 60px 60px;}

    </style>
</head>
<body onload ="myFunction()" >
        <h2>Countdown Timer</h2>
        <p><svg class="progress-ring" width="120" height="120" id='t'><g><circle stroke="lightgrey" stroke-width="15" fill="none" r="52" cx="60" cy="60" ></circle><circle class="progress-ring__circle" stroke=#00B5E2 stroke-width="15" fill="transparent" r="52" cx="60" cy="60" id='c'></circle><text x="60" y="60" text-anchor="middle" stroke="red" fill="red" stroke-width="1" dy=".3em" id='txt'></text></g></svg></p>
        <p><input type="range" onchange="updateSliderTimer(this)" min="1" max="20" value=%TIMERVALUE% class="slider1" id="myRange" step="1"></p>
        %BUTTONPLACEHOLDER%
<script  >
    var timer;
    var c=0;
    var gateway = `ws://${window.location.hostname}/ws`;
   var websocket;
var startTime;
var state;  
 var elapsedTime =parseInt(sessionStorage.getItem('sliderValue'))*60*1000;
window.addEventListener('load', onLoad);
var circle = document.getElementById('c');
var radius = circle.r.baseVal.value;
var circumference = radius * 2 * Math.PI;

circle.style.strokeDasharray = `${circumference} ${circumference}`;
circle.style.strokeDashoffset = `${circumference}`;

function setProgress(percent) {
  const offset = circumference - percent / Number(document.getElementById("myRange").max)* circumference;
  circle.style.strokeDashoffset = offset;
}
function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) { 
    //document.getElementById('time').innerHTML = event.data;
    var text = document.getElementById('txt');
    if(event.data=="false"){
      sessionStorage.setItem('buttonState',false);
      document.getElementById("startButton").checked=false;
      start();
      }
      else if (event.data=="true"){
        state=true;
        document.getElementById("startButton").checked=true;
        
        }
        else{
   text.innerHTML =event.data + "m";
    setProgress(Number(event.data));
   if (c){ 
   document.getElementById("myRange").value=event.data;
    c=0;
        }
     }
  }
   function onLoad(event) {
    initWebSocket();
   
  }
function start(element){
  
   
   
//  var xhr = new XMLHttpRequest();
  if(element){
    state = element.checked;
//     xhr.open("GET", "/button?value="+state, true);
//     xhr.send(); 
      websocket.send('' +state);
//     
}else{
 state=stringToBoolean(sessionStorage.getItem('buttonState'));
}
if(state==true) {
     
  document.getElementById("myRange").disabled=true;
  onMessage();
//var xhttp = new XMLHttpRequest();
//xhttp.onreadystatechange = function() {
//    if (this.readyState == 4 && this.status == 200) {
//       // Typical action to be performed when the document is ready:
//       document.getElementById("time").innerHTML = xhttp.responseText;
//    }
//};
//xhttp.open("GET", "/timer", true);
//xhttp.send();
//  if(elapsedTime!=parseInt(sessionStorage.getItem('prevTime'))){
//    elapsedTime=parseInt(sessionStorage.getItem('prevTime'));
//    startTime =parseInt(sessionStorage.getItem('startTime')||(Date.now() + elapsedTime ))  ;
//  sessionStorage.setItem('startTime', startTime);
//  } else{
//    startTime =parseInt(sessionStorage.getItem('startTime')||(Date.now() + elapsedTime ))  ;
//  sessionStorage.setItem('startTime', startTime);
//  }
//  timer = setInterval(myCounter,100);
 }
else if (state==false) {
//    clearInterval(timer);
//    timer=null;
//    sessionStorage.removeItem('startTime');
document.getElementById("myRange").disabled=false;
 }
}
//  function myCounter() { 
//var currentTime,timeElapsed;
//currentTime = Date.now();
//elapsedTime =startTime-currentTime;
//  timeElapsed = new Date(elapsedTime);
//    mins = timeElapsed.getMinutes();
//    secs = timeElapsed.getSeconds();
//    ms = timeElapsed.getMilliseconds();
//  document.getElementById("time").innerHTML =mins + "m " + secs + "." + ms + "s";
// if(mins==0 && secs==0 && ms<100){
//    reset();
//}
//
//  var timeObj = {min:mins, sec:secs, mls:ms};
//   sessionStorage.setItem("timeObj", JSON.stringify(timeObj));
//  var xhr = new XMLHttpRequest();
//   xhr.open("GET", "/timer?value="+elapsedTime, true);
// xhr.send();
//}
//function stop() {
//  clearInterval(timer);
//  timer=null;
//}
function myFunction(){
//  xhr.onreadystatechange = function() {
//    if (this.readyState == 4 && this.status == 200) {
//      sessionStorage.setItem('prevTime',this.responseText);
//     console.log(this.responseText);
//    }
//  }
//  if (%VAL%){
//  xhr.open("GET", "/timer?value="+(sessionStorage.getItem('prevTime')), true);
//  xhr.send();
//  }else{
//   xhr.open("GET", "/timer?value=" + "" %TIMEVALUE% , true);
//  xhr.send();
//  }
//var xhr = new XMLHttpRequest();
var sliderValue =document.getElementById("myRange").value;
var text = document.getElementById('txt');
    text.innerHTML=%TIMVAL% + "m";
setProgress(Number(%TIMVAL%));

sessionStorage.setItem('buttonState',%BUTTONVALUE%);
 sessionStorage.setItem('prevTime',%TIMVAL%);
// elapsedTime=parseInt(sessionStorage.getItem('prevTime'));
 
  var stat= (%BUTTONVALUE%).toString();
 if(stat=="true"){
   start();
   document.getElementById("startButton").checked=true;
     }
}
function stringToBoolean(string){
    switch(string.toLowerCase()){
        case "false": case "no": case "0": case "": return false;
        default:  return true;
    }
}
//function reset() {
//    stop();
//    sessionStorage.removeItem('startTime');
//    elapsedTime = parseInt(sessionStorage.getItem('sliderValue'))*60*1000; 
//    sessionStorage.setItem('prevTime',elapsedTime);
//    document.getElementById("myRange").disabled=false; 
//     document.getElementById('time').innerHTML = "0m 0.0s";
//     var stat = document.getElementById("startButton").checked=false;
//     sessionStorage.setItem('buttonState',stat); 
//}
function updateSliderTimer(element) {
 var valueSlider = document.getElementById("myRange").value;
 websocket.send('' +valueSlider);
  setProgress(Number(valueSlider));
 var text = document.getElementById('txt');
    text.innerHTML=valueSlider + "m ";
    c=1;
//document.getElementById("time").innerHTML = valueSlider + "m " + 0 + "." + 0 + "s";
//elapsedTime=parseInt(sessionStorage.getItem('sliderValue'))*60*1000;
//sessionStorage.setItem('prevTime',elapsedTime);

}

</script>      
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String outputState(){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}


String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    String outputStateValue = outputState();
    buttons+= "<p><label class=\"switch\"><input type=\"checkbox\" id=\"startButton\" onchange=\"start(this)\" " + outputStateValue + "/><span class=\"slider round\"></span></label></p>";
    return buttons;
  }
  else if(var == "TIMERVALUE"){
    return timerSliderValue;
  }
   else if(var == "TIMVAL"){
    
    return String(minsCount);
   }
   else if(var == "BUTTONVALUE"){
    return buttonValue;
    }
  return String();
}

//void notifyClients() {
//  ws.textAll(minsCount));
//}
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;//strcmp((char*)data, "toggle") == 0
    if (String((char*)data)=="true") {
      digitalWrite(output, HIGH);
      buttonValue=String((char*)data);
      ws.textAll(buttonValue);
      
    }
    else if (String((char*)data)=="false")
    {
      digitalWrite(output, LOW);
      buttonValue=String((char*)data);
      
        //notifyClients();
        
      }
    else if (*data >= (uint8_t)('0' + 1) && *data <= (uint8_t)('0' + 20)) {
       timerSliderValue=String((char*)(data));
        //count= timerSliderValue.toInt();
     // Serial.println(timerSliderValue);
      secsCount=0;
      minsCount=timerSliderValue.toInt();
      count=minsCount;
      ws.textAll(timerSliderValue);
     Serial.println(timerSliderValue);
      }
      else{}
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);

  // Connect to Wi-Fi
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
initWebSocket();
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  
//  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
//  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
//    String inputMessage;
//    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
//    if (request->hasParam(PARAM_INPUT_1)) {
//      inputMessage = request->getParam(PARAM_INPUT_1)->value();
//      digitalWrite(output, inputMessage.toInt());
//    }
//    else {
//      inputMessage = "No message sent";
//    }
//    Serial.println(inputMessage);
//    request->send(200, "text/plain", "OK");
//  });
//  
//  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
//  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
//    String inputMessage;
//    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
//    if (request->hasParam(PARAM_INPUT_2)) {
//      inputMessage = request->getParam(PARAM_INPUT_2)->value();
//      timerSliderValue = inputMessage;
//      elapsedMillis=(timerSliderValue.toInt())*60*1000;
//    }
//    else {
//      inputMessage = "No message sent";
//    }
//    Serial.println(inputMessage);
//    request->send(200, "text/plain", "OK");
//  });
//  server.on("/timer", HTTP_GET, [] (AsyncWebServerRequest *request) {
//    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
//    request->send_P(200, "text/plain",timObj.c_str());
//  });
//
//    server.on("/button", HTTP_GET, [] (AsyncWebServerRequest *request) {
//    String inputMessage;
//    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
//    if (request->hasParam(PARAM_INPUT_2)) {
//      inputMessage = request->getParam(PARAM_INPUT_2)->value();
//      buttonValue= inputMessage;
//    }    else {
//      inputMessage = "No message sent";
//    }
//    request->send_P(200, "text/plain","OK");
//   Serial.println(inputMessage);
//  });
  // Start server
  server.begin();
}
void loop() {
 
      
      unsigned long currentMillis=millis();
        if (buttonValue!="true"){
  previousMillis=currentMillis;
  }
  if (buttonValue=="true" && (unsigned long)(currentMillis - previousMillis) >= interval) {
     previousMillis=currentMillis;
     secsCount=secsCount+1;
      if (secsCount==60){
  
  minsCount=minsCount-1;
  secsCount=0;
 
  }
  
   if(minsCount==0){
    digitalWrite(output, LOW);
    ws.textAll(String(minsCount));
  buttonValue="false";
ws.textAll(String(buttonValue));
minsCount=timerSliderValue.toInt();
count=minsCount;
    }
    
    if(minsCount==count){
 ws.textAll(String(minsCount));
 count=count-1;

    }
  //  unsigned long durMS = (elapsedMillis%1000);       //Milliseconds
  // unsigned long durSS = (elapsedMillis/1000)%60;    //Seconds
  //  unsigned long durMM = (elapsedMillis/(60000))%60; //Minutes
  //  unsigned long durHH = (elapsedMillis/(3600000));  //Hours
  //  durHH = durHH % 24;

}
//   Serial.print("Time: ");
//   if (durMM < 10) Serial.print("0");
 
//    Serial.print(" : ");
//   if (durSS < 10) Serial.print("0");
   
//    Serial.print(" : ");
//    Serial.println("");

 else if (buttonValue=="false"){
  } 
 }
