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
  unsigned long lastBatteryCheck;  // For controlled battery check interval
  const unsigned long BATTERY_CHECK_INTERVAL = 300000;  // 5 minutes (300,000 ms)
  
  // Brightness change flags
  bool brightnessChangeRequested;
  uint8_t requestedBrightness;
  
  // Sleep preparation flags
  bool preparingForSleep;
  unsigned long sleepPrepStartTime;
  
public:
  PowerManager() : lastActiveTime(0), lowBatteryMode(false), batteryVoltage(0.0), 
                   originalBrightness(255), lastBatteryCheck(0), 
                   brightnessChangeRequested(false), requestedBrightness(0),
                   preparingForSleep(false), sleepPrepStartTime(0) {}
  
  // Function declarations - implementations moved to PowerManager.cpp
  void begin();
  float readBatteryVoltage();
  void update();
  void resetActivityTimer();
  float getBatteryVoltage();
  float getBatteryPercentage();
  bool isLowBattery();
  bool needsBrightnessChange();
  uint8_t getRequestedBrightness();
  void clearBrightnessRequest();
};

#endif // HARDWARE_H