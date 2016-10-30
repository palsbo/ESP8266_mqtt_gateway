#define ESP01  false
#define ONEWIRE false
#define SWITCH false
#define RF433 true
#define _I2C false  //  if true - owerwrites ONEWIRE and SWITCH to false

#include "wifi.h"
#include "utils.h"
#if _I2C
#if ESP01
#define ONEWIRE false
#define SWITCH false
#endif
#include "i2c.h"
#endif
#if RF433
#include "RF.h"
#endif
#if SWITCH
#include "pinout.h"
#endif
#if ONEWIRE
#include "ow.h"
#endif
#include "web.h"
#include <PubSubClient.h>

#define LOOP_TIME (millis()-loopTimer)
#define LOOP_RESET loopTimer = millis()
#define CONN_TIME (millis()-connecTimer)
#define CONN_RESET connecTimer = millis()

WiFiClient espClient;
PubSubClient client(espClient);

#if ONEWIRE
OW ow;
#endif

#if SWITCH
PINOUT led;
#endif

#if RF433
RF rf;
#endif

#if _I2C
I2C i2c;
#endif

long loopTimer;
long connecTimer;
String strPayload;
#define LedPin 7
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(compile_date);
  if (!SPIFFS.begin()) Serial.println("Failed to mount file system");
  readFile("mqttserver", mqtt_server);
  mqtt_port = readFile("mqttport", 1883);
  onReconnect(MQTTonReconnect);
  onResubscribe(MQTTonResubscribe);
  onWiFiOk(WIFIonOk);
  onWiFiFailed(WIFIonFailed);
  onWiFiAPmode(WIFIonAPmode);
  Serial.println("Connecting to WiFi: ");
#if RF433
  rf.begin("RF433", readFile("RF433", "ha/rf433"), 2, 0);
  rf.onReceive([](String value) {
    if (value != "") rf.set(value);
  });
  rf.onChange([](String val) {
    String topic = "st/rf";
    client.publish(&topic[0], &val[0], true);
    Serial.println(val);
  });
#endif
#if SWITCH
  led.begin("Switch", readFile("Switch", "ha/switch1"), LedPin);
  led.onReceive([](String value) {
    if (value != "") led.set(value);
  });
  led.onChange([]() {
    Serial.print(led.topic);
    Serial.print(" changed to ");
    Serial.println(led.value);
  });
#endif
  onConfigList([]() {
    //  return all config rows
    String list;
#if SWITCH
    list += led.html();
#endif
#if RF433
    list += rf.html();
#endif
#if ONEWIRE
    list += ow.html();
#endif
#if _I2C
    list += i2c.html();
#endif
    return list;
  });
  initWiFi(false);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);
  initWeb();
#if ONEWIRE
  ow.onFound(owOnFound);
  ow.onChange(owOnChange);
  ow.discover();
#endif
#if _I2C
  i2c.begin();
  i2c.onFound(i2cOnFound);
  i2c.onChange(i2cOnChange);
  i2c.discover();
#endif
  onConfig([](String fn, String val, String dev) {
#if SWITCH
    if (dev == "pinout") {
      led.topic = val;
    }
#endif
#if RF433
    if (dev == "rf433") {
      rf.topic = val;
    }
#endif
#if _I2C
    if (dev == "i2c") {
      String ad = fn.substring(0, 6);
      String fld = fn.substring(7);
      int i = i2c.getIndex(ad);
      if (fld == "type") i2c.unit[i].type = val;
      if (fld == "") i2c.unit[i].topic = val;
    }
#endif
#if OW
    if (dev == "ow") {
      String ad = fn.substring(0, 6);
      int i = i2c.getIndex(ad);
      i2c.unit[i].topic = val;
    }
#endif
  });
}

void loop() {
#if ONEWIRE
  ow.handle();
#endif
#if RF433
  rf.handle();
#endif
  checkWeb();
#if SWITCH
  led.handle();
#endif
#if _I2C
  i2c.handle();
#endif
  if (!client.connected()) {
    if (CONN_TIME > 5000) mqtt_reconnect();
  } else {
    client.loop();
    if (LOOP_TIME > 10000) {
      LOOP_RESET;
#if SWITCH
      client.publish(&led.topic[0], &led.value[0], true);
#endif
#if ONWIRE
      for (int i = 0; i < ow.devCount; i++)  owOnChange(&ow.unit[i]);;
#endif
#if _I2C
      for (int i = 0; i < i2c.devCount; i++)  i2cOnChange(&i2c.unit[i]);
#endif
    }
  }
}
