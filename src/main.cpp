#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "Config.h"
#include "NetworkManager.h"
#include "SensorManager.h"

Adafruit_SSD1306 display(kOledWidth, kOledHeight, &Wire, -1);
NetworkManager network;
SensorManager sensors;
bool displayReady = false;

void showStatus(const char* message) {
  if (!displayReady) {
    return;
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(message);
  display.display();
}

void renderReadings(const SensorReadings& readings) {
  if (!displayReady) {
    return;
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.printf("%4uppm", readings.co2);

  display.setCursor(0, 16);
  display.printf("%4.1fC", readings.temperatureC);
  display.setCursor(68, 16);
  display.printf("%4.1f%%", readings.humidityPercent);
  display.display();
}

void publishDiscoveryEntity(const char* objectId,
                           const char* name,
                           const char* valueTemplate,
                           const char* unit,
                           const char* deviceClass) {
  String topic = String(kHomeAssistantPrefix) + "/sensor/mobile_co2/" + objectId + "/config";

  String payload = "{";
  payload += "\"name\":\"" + String(name) + "\",";
  payload += "\"unique_id\":\"mobile_co2_" + String(objectId) + "\",";
  payload += "\"state_topic\":\"" + String(kMqttStateTopic) + "\",";
  payload += "\"availability_topic\":\"" + String(kMqttAvailabilityTopic) + "\",";
  payload += "\"value_template\":\"" + String(valueTemplate) + "\",";
  payload += "\"unit_of_measurement\":\"" + String(unit) + "\",";
  payload += "\"device_class\":\"" + String(deviceClass) + "\",";
  payload += "\"state_class\":\"measurement\",";
  payload += "\"device\":{";
  payload += "\"identifiers\":[\"" + String(kDeviceId) + "\"],";
  payload += "\"name\":\"" + String(kDeviceName) + "\",";
  payload += "\"model\":\"" + String(kDeviceModel) + "\",";
  payload += "\"manufacturer\":\"" + String(kDeviceManufacturer) + "\"";
  payload += "}";
  payload += "}";

  network.publish(topic, payload, true);
}

void publishHomeAssistantDiscovery() {
  publishDiscoveryEntity("co2",
                         "Mobile CO2",
                         "{{ value_json.co2 }}",
                         "ppm",
                         "carbon_dioxide");
  publishDiscoveryEntity("temperature",
                         "Mobile CO2 Temperature",
                         "{{ value_json.temperature }}",
                         "C",
                         "temperature");
  publishDiscoveryEntity("humidity",
                         "Mobile CO2 Humidity",
                         "{{ value_json.humidity }}",
                         "%",
                         "humidity");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  (void)topic;
  (void)payload;
  (void)length;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(kOledSda, kOledScl);

  if (display.begin(SSD1306_SWITCHCAPVCC, kOledAddress)) {
    displayReady = true;
    display.setRotation(kOledRotation);
    showStatus("Booting...");
  }

  sensors.begin();
  showStatus("Connecting WiFi...");
  network.begin(mqttCallback);
}

void loop() {
  network.update();

  if (network.consumeMqttConnectedEvent()) {
    publishHomeAssistantDiscovery();
    showStatus("HA linked");
  }

  if (sensors.update()) {
    const SensorReadings& readings = sensors.getReadings();
    renderReadings(readings);

    if (network.isMqttConnected()) {
      String payload = sensors.getJson();
      network.publish(kMqttStateTopic, payload, true);
      Serial.println("Published: " + payload);
    }
  }

  delay(100);
}
