
/*
   MQTT callbacks
*/
void mqtt_callback(char* _topic, byte* _payload, unsigned int length) {
  _payload[length] = '\0';
  //  Handle all received topics here
#if SWITCH
  led.check(_topic, (char*)_payload);
#endif
#if RF433
  rf.check(_topic, (char*)_payload);
#endif
}

void mqtt_reconnect() {
  String s;
  String p;
    s = readFile("mqttserver", "homeassistant");
    strcpy(&mqtt_server[0],&s[0]);
    p = readFile("mqttport", "1883");
    mqtt_port = p.toInt();
  Serial.println("Attempting MQTT connection..." + s + ":" + p);
  client.setServer(mqtt_server, mqtt_port);
  if (client.connect("arduinoClient")) {
    Serial.println("connected");
    topicSubscribe = readFile("subscribe", topicSubscribe);
    client.subscribe(&topicSubscribe[0]);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    CONN_RESET;
  }
}

void MQTTonReconnect() {
  readFile("mqttserver", mqtt_server, 30);
  client.disconnect();
  client.setServer(mqtt_server, mqtt_port);
  mqtt_reconnect();
}

void MQTTonResubscribe(String val) {
  client.unsubscribe(&topicSubscribe[0]);
  topicSubscribe = val;
  client.subscribe(&topicSubscribe[0]);
}

/*
   WiFi callbacks
*/

void WIFIonOk() {
  Serial.println("WiFi connected to:  ");
  Serial.println(WiFi.SSID());
  Serial.println("IP address: ");
  Serial.println(ip2string(WiFi.localIP()));
}

void WIFIonFailed() {
  Serial.println("Failed to connect ");
  Serial.println("and hit timeout");
}

void WIFIonAPmode(WiFiManager * myWiFiManager) {
  Serial.println("Entered config mode:");
  Serial.println("Go to WiFi setup.");
  Serial.println("Select '" + myWiFiManager->getConfigPortalSSID() + "'");
  Serial.println("Browse 192.168.4.1");
}

/*
   Onewire callback
*/

#if ONEWIRE
void owOnFound(DEV * unit) {
  Serial.print("Found: ");
  Serial.print(unit->chip + " ");
  Serial.println(unit->fn);
}

void owOnChange(DEV * unit) {
  if (!client.connected()) return;
  String val = float2string(unit->value,4,1);
  client.publish(&unit->topic[0], &val[0], true);
}
#endif

/*
   I2C callback
*/

#if _I2C
void i2cOnFound(I2C_DEV * unit) {
  Serial.print("Found: ");
  Serial.println(unit->fn);
}

void i2cOnChange(I2C_DEV * unit) {
  if (!client.connected()) return;
  String val = float2string(unit->value,4,1);
  client.publish(&unit->topic[0], &val[0], true);
  //Serial.println(unit->topic + " " + val);
}
#endif

