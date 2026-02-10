#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2cScd4x.h>

// OLED setup
constexpr uint8_t kOledWidth = 128;
constexpr uint8_t kOledHeight = 32;
constexpr uint8_t kOledAddress = 0x3C;
Adafruit_SSD1306 display(kOledWidth, kOledHeight, &Wire, -1);

// SCD41 setup — second I2C bus (Wire1)
constexpr uint8_t kScd41Sda = 16;
constexpr uint8_t kScd41Scl = 17;
SensirionI2cScd4x scd4x;

void scanI2CBus() {
  Serial.println("I2C scan start");
  uint8_t count = 0;

  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("  found: 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      ++count;
    }
  }

  Serial.print("I2C scan done, devices: ");
  Serial.println(count);
}

void renderReadings(uint16_t co2, float temperatureC, float humidityRh) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.printf("%4uppm", co2);

  // Second row: temperature and humidity side by side
  display.setCursor(0, 16);
  display.printf("%4.1fC", temperatureC);
  display.setCursor(68, 16);
  display.printf("%4.1f%%", humidityRh);
  display.display();
}

void showStatus(const char* message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(message);
  display.display();
}

void setup() {
  Serial.begin(9600);
  delay(100);
  Wire.begin();
  Wire1.begin(kScd41Sda, kScd41Scl);

  if (!display.begin(SSD1306_SWITCHCAPVCC, kOledAddress)) {
    // Avoid reboot loops; nothing else to do if display fails
    while (true) {
      delay(1000);
    }
  }

  display.setRotation(2);

  showStatus("Init sensor...");

  scd4x.begin(Wire1, SCD41_I2C_ADDR_62);
  scd4x.startPeriodicMeasurement();

  scanI2CBus();

  // Give the sensor time to prepare the first reading (~5s typical for SCD41)
  delay(5000);
}

void loop() {
  bool dataReady = false;
  scd4x.getDataReadyStatus(dataReady);
  if (!dataReady) {
    delay(100);
    return;
  }

  uint16_t co2 = 0;
  float temperatureC = 0.0f;
  float humidityRh = 0.0f;

  uint16_t error = scd4x.readMeasurement(co2, temperatureC, humidityRh);
  if (error == 0 && !isnan(temperatureC) && !isnan(humidityRh)) {
    renderReadings(co2, temperatureC, humidityRh);
  } else {
    showStatus("Read error");
  }

  // Poll roughly every 2 seconds; the sensor itself updates ~every 5 seconds
  delay(2000);
}