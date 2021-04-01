#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
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
    .slider1 {width: 100%;}
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
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;  
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
    
    var text = document.getElementById('txt');
    if(event.data=="false"){
      
      document.getElementById("startButton").checked=false;
      document.getElementById("myRange").disabled=false;
      }
      else if (event.data=="true"){
        document.getElementById("myRange").disabled=true;
        document.getElementById("startButton").checked=true;
        
        }
        else{
   text.innerHTML =event.data + "m";
    setProgress(Number(event.data));
if (document.getElementById("startButton").checked!=true){
      document.getElementById("myRange").value=event.data;
      }
     }
  }
 
  function onLoad(event) {
    initWebSocket();
   
  }

function start(element){
 var state;
     if(element){
     state = element.checked; 
     websocket.send('' +state);   
     }
} 

function myFunction(){
var text = document.getElementById('txt');
    text.innerHTML=%TIMVAL% + "m";
setProgress(Number(%TIMVAL%));
}

function updateSliderTimer(element) {
 var valueSlider = document.getElementById("myRange").value;
 websocket.send('' +valueSlider);
  setProgress(Number(valueSlider));
 var text = document.getElementById('txt');
    text.innerHTML=valueSlider + "m ";
}
</script>      
</body>
</html>
)rawliteral";

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
      ws.textAll(buttonValue);
      }
    else if (*data >= (uint8_t)('0' + 1) && *data <= (uint8_t)('0' + 20)) {
       timerSliderValue=String((char*)(data));
      secsCount=0;
      minsCount=timerSliderValue.toInt();
      count=minsCount;
      ws.textAll(timerSliderValue);
      }
      else{}
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
     // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //Serial.printf("WebSocket client #%u disconnected\n", client->id());
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
  //Serial.begin(115200);

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);

  // Connect to Wi-Fi
  //Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid);

 // IPAddress IP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(IP);
  // Print ESP Local IP Address
 // Serial.println(WiFi.localIP());
initWebSocket();
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
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
      
      if (secsCount==sec){
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
  }
 else if (buttonValue=="false"){
  } 
 }
