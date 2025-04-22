#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "EffectsConfig.h"  // Include new config file

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

// Brightness mode enum - must match LEDController's BrightnessMode
enum BrightnessMode {
  BRIGHTNESS_NORMAL,      // Regular operating brightness
  BRIGHTNESS_IMPACT,      // Brightness during impact effect
  BRIGHTNESS_LOW_BATTERY, // Reduced brightness for low battery
  BRIGHTNESS_SLEEP        // Very dim brightness before sleep
};

// Power management functions
class PowerManager {
private:
  unsigned long lastActiveTime;
  bool lowBatteryMode;
  float batteryVoltage;
  uint8_t originalBrightness;
  unsigned long lastBatteryCheck;
  
  // Brightness change flags and modes
  bool brightnessChangeRequested;
  BrightnessMode requestedBrightnessMode;
  
  // Sleep preparation flags
  bool preparingForSleep;
  unsigned long sleepPrepStartTime;
  
public:
  PowerManager() : lastActiveTime(0), lowBatteryMode(false), batteryVoltage(0.0), 
                   originalBrightness(DEFAULT_BRIGHTNESS), lastBatteryCheck(0), 
                   brightnessChangeRequested(false), requestedBrightnessMode(BRIGHTNESS_NORMAL),
                   preparingForSleep(false), sleepPrepStartTime(0) {}
  
  // Function declarations - implementations in PowerManager.cpp
  void begin();
  float readBatteryVoltage();
  void update();
  void resetActivityTimer();
  float getBatteryVoltage();
  float getBatteryPercentage();
  bool isLowBattery();
  
  // Updated brightness management methods
  bool needsBrightnessChange();
  BrightnessMode getRequestedBrightnessMode(); // Changed to return a mode instead of a value
  void clearBrightnessRequest();
};

#endif // HARDWARE_H