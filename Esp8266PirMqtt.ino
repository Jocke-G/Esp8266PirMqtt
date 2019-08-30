#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include "defines.h"

void mqttDataCb(char* topic, byte* payload, unsigned int length);
void mqttConnectedCb();
void mqttDisconnectedCb();
void processNet();

boolean pendingDisconnect = true;

WiFiClient wclient;
PubSubClient client(MQTTip, MQTTport, mqttDataCb, wclient);

// #################### mqtt ####################

void mqttDataCb(char* topic, byte* payload, unsigned int length) {
  char* message = (char *) payload;
  message[length] = 0;

  if (strcmp(message, "on")) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (strcmp(message, "off")) {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void mqttConnectedCb() {
  client.subscribe("testtopic/sub", MQTTsubQos);
  client.publish("testtopic/connected", "connected");
}

void mqttDisconnectedCb() {  

}

// #################### setup e loop ####################

void setup() {
  ArduinoOTA.setHostname(MQTTid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(DEFAULTssid, DEFAULTpsw);

  pinMode(LED_BUILTIN, OUTPUT);
}

void processNet() {
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.begin();
    ArduinoOTA.handle();
    if (client.connected()) {
      client.loop();
    } else {
      if (client.connect(MQTTid, MQTTuser, MQTTpsw)) {
          pendingDisconnect = false;
          mqttConnectedCb();
      }
    }
  } else {
    if (client.connected())
      client.disconnect();
  }
  if (!client.connected() && !pendingDisconnect) {
    pendingDisconnect = true;
    mqttDisconnectedCb();
  }
}

void loop() {
  processNet();
}
