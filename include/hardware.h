#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Project version and build info
#include "../src/version.h"

// Hardware-specific definitions
#define SERIAL_BAUD 115200

// Battery monitoring
#define BATTERY_PIN A0         // Analog pin for battery voltage monitoring
#define BATTERY_DIVIDER 2      // Voltage divider ratio
#define BATTERY_MIN_VOLTAGE 3.3  // Minimum safe battery voltage
#define BATTERY_MAX_VOLTAGE 4.2  // Maximum battery voltage when fully charged

// Power management settings
#define POWER_SAVING_MODE 1     // Enable power saving features (0=disabled, 1=enabled)
#define SLEEP_AFTER_MINS 30     // Minutes of inactivity before entering sleep mode

// Power management functions
class PowerManager {
private:
  unsigned long lastActiveTime;
  bool lowBatteryMode;
  float batteryVoltage;
  uint8_t originalBrightness;
  
public:
  PowerManager() : lastActiveTime(0), lowBatteryMode(false), batteryVoltage(0.0), originalBrightness(255) {}
  
  void begin() {
    // Initialize power management
    lastActiveTime = millis();
    batteryVoltage = readBatteryVoltage();
    lowBatteryMode = (batteryVoltage < BATTERY_MIN_VOLTAGE);
    
    // Set up pins for power monitoring
    pinMode(BATTERY_PIN, INPUT);
  }
  
  float readBatteryVoltage() {
    // Read battery voltage through voltage divider
    int rawValue = analogRead(BATTERY_PIN);
    float voltage = (rawValue / 1023.0) * 3.3 * BATTERY_DIVIDER;
    return voltage;
  }
  
  void update() {
    // Check battery voltage periodically
    EVERY_N_SECONDS(30) {
      batteryVoltage = readBatteryVoltage();
      
      // Check for low battery condition
      if (batteryVoltage < BATTERY_MIN_VOLTAGE && !lowBatteryMode) {
        lowBatteryMode = true;
        originalBrightness = FastLED.getBrightness();
        // Reduce brightness to conserve power
        FastLED.setBrightness(originalBrightness / 2);
        Serial.println("Low battery mode activated");
      } else if (batteryVoltage > (BATTERY_MIN_VOLTAGE + 0.2) && lowBatteryMode) {
        // Restore normal operation when voltage is back up
        lowBatteryMode = false;
        FastLED.setBrightness(originalBrightness);
        Serial.println("Normal power mode restored");
      }
    }
    
    // Check for inactivity timeout
    if (POWER_SAVING_MODE && (millis() - lastActiveTime > SLEEP_AFTER_MINS * 60000)) {
      // Enter sleep mode to save power
      Serial.println("Entering sleep mode");
      // Save any unsaved settings here
      
      // Fade LEDs to black
      uint8_t originalBrightness = FastLED.getBrightness();
      for (int i = originalBrightness; i >= 0; i--) {
        FastLED.setBrightness(i);
        FastLED.show();
        delay(20);
      }
      
      // Put ESP into deep sleep
      ESP.deepSleep(0);
    }
  }
  
  void resetActivityTimer() {
    lastActiveTime = millis();
  }
  
  float getBatteryVoltage() {
    return batteryVoltage;
  }
  
  float getBatteryPercentage() {
    // Calculate battery percentage based on voltage
    float percentage = (batteryVoltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100.0;
    return constrain(percentage, 0.0, 100.0);
  }
  
  bool isLowBattery() {
    return lowBatteryMode;
  }
};

#endif // HARDWARE_H