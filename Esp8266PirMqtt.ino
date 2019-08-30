#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include "defines.h"

void mqttDataCb(char* topic, byte* payload, unsigned int length);
void mqttDisconnectedCb();
void processNet();

boolean pendingDisconnect = true;
int lastMillis;
bool activeState = LOW;

WiFiClient wclient;
PubSubClient client(MQTTip, MQTTport, mqttDataCb, wclient);

// #################### mqtt ####################

void mqttDataCb(char* topic, byte* payload, unsigned int length) {
}

void mqttConnectedCb() {
  client.publish("testtopic/connected", "connected");
}

void mqttDisconnectedCb() {  
}

// #################### setup e loop ####################

void setup() {
  pinMode(PirSensorPin, INPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);

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
      if (client.connect(MQTTid, MQTTuser, MQTTpsw, "testtopic/out", 1, 0, "Out")) {
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

  long state = digitalRead(PirSensorPin);
  Serial.print(millis());
  Serial.print(" ");
  int nextSend = lastMillis + 3 * 1000;
  Serial.print(nextSend);
  Serial.print(" ");
  if (state == HIGH) {
    Serial.print("HIGH");
    if(nextSend < millis()) {
      Serial.print(" SEND");
      client.publish("testtopic/motion", "motion");
      activeState = HIGH;
    }
    lastMillis = millis();
  } else {
    Serial.print("LOW");
    if(nextSend > millis() && activeState == HIGH) {
      activeState = LOW;
      client.publish("testtopic/nomotion", "nomotion");
    }
  }
    Serial.println("");
}
