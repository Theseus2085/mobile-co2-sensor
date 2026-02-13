#pragma once

#include <Arduino.h>

#if __has_include("Secrets.h")
#include "Secrets.h"
#else
#error "Missing include/Secrets.h. Copy include/Secrets.h.example to include/Secrets.h and fill your credentials."
#endif

constexpr char kDeviceId[] = "mobile_co2_sensor";
constexpr char kDeviceName[] = "Mobile CO2 Sensor";
constexpr char kDeviceModel[] = "ESP32 + SCD41 + SSD1306";
constexpr char kDeviceManufacturer[] = "DIY";
constexpr char kMqttClientId[] = "mobile_co2_sensor";
constexpr char kMqttStateTopic[] = "mobile_co2/sensors";
constexpr char kMqttAvailabilityTopic[] = "mobile_co2/availability";
constexpr char kHomeAssistantPrefix[] = "homeassistant";

constexpr uint32_t kWiFiReconnectIntervalMs = 10000;
constexpr uint32_t kMqttReconnectIntervalMs = 5000;
constexpr uint32_t kSensorWarmupMs = 5000;

constexpr uint8_t kOledSda = 21;
constexpr uint8_t kOledScl = 22;
constexpr uint8_t kOledWidth = 128;
constexpr uint8_t kOledHeight = 32;
constexpr uint8_t kOledAddress = 0x3C;
constexpr uint8_t kOledRotation = 2;

constexpr uint8_t kScd41Sda = 16;
constexpr uint8_t kScd41Scl = 17;
