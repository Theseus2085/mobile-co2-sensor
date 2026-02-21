#include "NetworkManager.h"

#include <cstring>

NetworkManager::NetworkManager()
    : mqttClient_(wifiClient_),
      mqttCallback_(nullptr),
      lastWiFiReconnectAttemptMs_(0),
      lastMqttReconnectAttemptMs_(0),
      mqttConnectedEventPending_(false) {}

void NetworkManager::begin(MqttCallback mqttCallback) {
  mqttCallback_ = mqttCallback;
  setupWiFi();
  setupOTA();
  setupMQTT();
}

void NetworkManager::setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 40) {
    delay(500);
    Serial.print('.');
    retries++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi not connected yet, will retry in loop.");
  }

  lastWiFiReconnectAttemptMs_ = millis();
}

void NetworkManager::setupOTA() {
  ArduinoOTA.setHostname(kMqttClientId);

  if (std::strlen(OTA_PASSWORD) > 0) {
    ArduinoOTA.setPassword(OTA_PASSWORD);
  }

  ArduinoOTA.onStart([]() { Serial.println("OTA start"); });
  ArduinoOTA.onEnd([]() { Serial.println("OTA end"); });
  ArduinoOTA.onError([](ota_error_t error) { Serial.printf("OTA error: %u\n", error); });
  ArduinoOTA.begin();
}

void NetworkManager::setupMQTT() {
  mqttClient_.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient_.setBufferSize(1024);

  if (mqttCallback_ != nullptr) {
    mqttClient_.setCallback(mqttCallback_);
  }

  connectMQTT();
}

void NetworkManager::connectMQTT() {
  if (WiFi.status() != WL_CONNECTED || mqttClient_.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT...");
  bool connected = false;

  if (std::strlen(MQTT_USER) > 0) {
    connected = mqttClient_.connect(kMqttClientId,
                                    MQTT_USER,
                                    MQTT_PASS,
                                    kMqttAvailabilityTopic,
                                    1,
                                    true,
                                    "offline");
  } else {
    connected = mqttClient_.connect(kMqttClientId,
                                    kMqttAvailabilityTopic,
                                    1,
                                    true,
                                    "offline");
  }

  if (!connected) {
    Serial.print(" failed, rc=");
    Serial.println(mqttClient_.state());
    return;
  }

  Serial.println(" connected");
  mqttClient_.publish(kMqttAvailabilityTopic, "online", true);
  mqttConnectedEventPending_ = true;
}

void NetworkManager::update() {
  ArduinoOTA.handle();

  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWiFiReconnectAttemptMs_ >= kWiFiReconnectIntervalMs) {
      lastWiFiReconnectAttemptMs_ = now;
      Serial.println("Retrying WiFi...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
    return;
  }

  if (!mqttClient_.connected() && now - lastMqttReconnectAttemptMs_ >= kMqttReconnectIntervalMs) {
    lastMqttReconnectAttemptMs_ = now;
    connectMQTT();
  }

  if (mqttClient_.connected()) {
    mqttClient_.loop();
  }
}

bool NetworkManager::publish(const char* topic, const char* payload, bool retained) {
  if (!mqttClient_.connected()) {
    return false;
  }

  return mqttClient_.publish(topic, payload, retained);
}

bool NetworkManager::publish(const String& topic, const String& payload, bool retained) {
  return publish(topic.c_str(), payload.c_str(), retained);
}

bool NetworkManager::subscribe(const char* topic) {
  if (!mqttClient_.connected()) {
    return false;
  }

  return mqttClient_.subscribe(topic);
}

bool NetworkManager::isWiFiConnected() const { return WiFi.status() == WL_CONNECTED; }

bool NetworkManager::isMqttConnected() { return mqttClient_.connected(); }

bool NetworkManager::consumeMqttConnectedEvent() {
  if (!mqttConnectedEventPending_) {
    return false;
  }

  mqttConnectedEventPending_ = false;
  return true;
}
