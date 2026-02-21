#pragma once

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "Config.h"

typedef void (*MqttCallback)(char* topic, byte* payload, unsigned int length);

class NetworkManager {
 public:
  NetworkManager();

  void begin(MqttCallback mqttCallback = nullptr);
  void update();

  bool publish(const char* topic, const char* payload, bool retained = false);
  bool publish(const String& topic, const String& payload, bool retained = false);
  bool subscribe(const char* topic);

  bool isWiFiConnected() const;
  bool isMqttConnected();
  bool consumeMqttConnectedEvent();

 private:
  void setupWiFi();
  void setupOTA();
  void setupMQTT();
  void connectMQTT();

  WiFiClient wifiClient_;
  PubSubClient mqttClient_;
  MqttCallback mqttCallback_;
  unsigned long lastWiFiReconnectAttemptMs_;
  unsigned long lastMqttReconnectAttemptMs_;
  bool mqttConnectedEventPending_;
};
