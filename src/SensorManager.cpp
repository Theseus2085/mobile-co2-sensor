#include "SensorManager.h"

#include <Wire.h>
#include <cmath>

#include "Config.h"

SensorManager::SensorManager()
    : readings_{0, NAN, NAN, false}, warmupStartMs_(0), warmupComplete_(false) {}

void SensorManager::begin() {
  Wire1.begin(kScd41Sda, kScd41Scl);
  scd4x_.begin(Wire1, SCD41_I2C_ADDR_62);

  uint16_t error = scd4x_.startPeriodicMeasurement();
  if (error != 0) {
    Serial.printf("SCD41 start error: %u\n", error);
  } else {
    Serial.println("SCD41 periodic measurement started");
  }

  warmupStartMs_ = millis();
}

bool SensorManager::update() {
  if (!warmupComplete_) {
    if (millis() - warmupStartMs_ < kSensorWarmupMs) {
      return false;
    }
    warmupComplete_ = true;
  }

  bool dataReady = false;
  uint16_t error = scd4x_.getDataReadyStatus(dataReady);
  if (error != 0 || !dataReady) {
    return false;
  }

  uint16_t co2 = 0;
  float temperatureC = 0.0f;
  float humidityPercent = 0.0f;
  error = scd4x_.readMeasurement(co2, temperatureC, humidityPercent);
  if (error != 0 || std::isnan(temperatureC) || std::isnan(humidityPercent)) {
    readings_.valid = false;
    return false;
  }

  readings_.co2 = co2;
  readings_.temperatureC = temperatureC;
  readings_.humidityPercent = humidityPercent;
  readings_.valid = true;
  return true;
}

const SensorReadings& SensorManager::getReadings() const { return readings_; }

String SensorManager::getJson() const {
  if (!readings_.valid) {
    return "{}";
  }

  char payload[96];
  snprintf(payload,
           sizeof(payload),
           "{\"co2\":%u,\"temperature\":%.1f,\"humidity\":%.1f}",
           readings_.co2,
           readings_.temperatureC,
           readings_.humidityPercent);
  return String(payload);
}
