#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

String content;
char topicLED[20] = "";
//String topicLED
String topicSubscribe = "";
char mqtt_server[30] = "palsbo.net";
int mqtt_port = 1883;
const char compile_date[] = __DATE__ " " __TIME__;

static const char index_html[] PROGMEM = R"=====(
<html>
<head>
  <title>'Home Assistant switch'</title>
  <style>
    .left { text-align: left; }
    .right { text-align: right; }
    .hdr { height:40px; font-size:18px; text-align:left; vertical-align:bottom }
  </style>
</head>
<body>
  <h2>Configuration:</h2>
  <p>Compile date: %%CD%%</p>
  <table>
  <tr><td colspan='3'></td><th class)'right'>Refresh </th><td><button onclick="document.location='/'">Update</button></td></tr>
  <tr><th class="left">MQTT</th><th colspan=2 class="right">Broker name:</th><td><input id="mqttserver" value="%%MQS%%"/></td><td><button onclick="document.location='/?mqttserver=' + getElementById('mqttserver').value">Update</button></td></tr>
  <tr><th class="left">MQTT</th><th colspan=2 class="right">Port::</th><td><input id="mqttport" value="%%MQP%%"/></td><td><button onclick="document.location='/?mqttport=' + getElementById('mqttport').value">Update</button></td></tr>
  <tr><th class="left">Subscribe:</th><th colspan=2 class="right">Topic:<th><input id="subscribe" value="%%SU%%"/></td><td><button onclick="document.location='/?subscribe=' + getElementById('subscribe').value">Update</button></td></tr>
  %%LIST%%
  </table>
</body>
</html>
)=====";

typedef void (*rcon)();
typedef void (*rsub)(String val);
typedef void (*rtop)(String fn, String val, String dev);

rcon xx;
rsub uu;
rtop tt;

void onReconnect(rcon x_cb) {
  xx = x_cb;
}

void onResubscribe(rsub x_cb) {
  uu = x_cb;
}

void onConfig(rtop x_cb) {
  tt = x_cb;
}

void config(String fn, String val, String dev) {
  writeFile(fn, val);
  Serial.println(fn + " " + val);
  if (fn == "mqttserver") {
    Serial.println("Setting Server to " + val);
    if (xx) xx();
  }
  else if (fn=="mqttport") {
    mqtt_port = readFile("mqttport", 1883);
    Serial.println("Setting port to " + val);
    if (xx) xx();
  }
  else if (fn == "subscribe") {
    if (uu) uu(val);
  } else {
    if (tt) tt(fn, val, dev);
  }
}

typedef String (*callback_s0)();

callback_s0 ccb0;

String onConfigList( callback_s0 _ccb0) {
  ccb0 = _ccb0;
}

void handleRoot() {
  if (server.args()>0) config(server.argName(0), server.arg(0), server.arg(1));
  content = index_html;
  String list;
  if (ccb0) list = ccb0();
  content.replace("%%LIST%%", list);
  content.replace("%%CD%%",(char *)compile_date);
  //content.replace("%%SW%%",topicLED);
  content.replace("%%SU%%",topicSubscribe);
  content.replace("%%MQS%%",(char *)mqtt_server);
  content.replace("%%MQP%%",String(mqtt_port));
  server.send(200, "text/html", content);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void initWeb() {
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void checkWeb() {
  server.handleClient();
}


