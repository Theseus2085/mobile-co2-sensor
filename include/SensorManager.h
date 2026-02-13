#pragma once

#include <Arduino.h>
#include <SensirionI2cScd4x.h>

struct SensorReadings {
  uint16_t co2;
  float temperatureC;
  float humidityPercent;
  bool valid;
};

class SensorManager {
 public:
  SensorManager();
  void begin();
  bool update();

  const SensorReadings& getReadings() const;
  String getJson() const;

 private:
  SensirionI2cScd4x scd4x_;
  SensorReadings readings_;
  unsigned long warmupStartMs_;
  bool warmupComplete_;
};
