#ifndef BOSTAFF_H
#define BOSTAFF_H

#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "effects.h"

// Pin definitions
#define LED_PIN_1 D1  // GPIO5 - First LED strip
#define LED_PIN_2 D2  // GPIO4 - Second LED strip
#define BTN_PIN   D6  // GPIO12 - Button pin

// MPU6050 accelerometer pins (I2C)
#define SDA_PIN   D2  // GPIO4
#define SCL_PIN   D1  // GPIO5

// LED strip configuration
#define NUM_LEDS_PER_STRIP 200
#define NUM_STRIPS 2
#define TOTAL_LEDS (NUM_LEDS_PER_STRIP * NUM_STRIPS)

// Global configuration structure
struct Config {
  uint8_t currentMode = EFFECT_FIRE;   // Default mode
  uint8_t brightness = 150;             // Default brightness
  uint8_t numModes = NUM_EFFECTS;       // Number of available modes
  uint8_t impactThreshold = 8000;       // Acceleration threshold for impact detection
  uint16_t impactFlashDuration = 100;   // Duration of impact flash in ms
};

// LED Controller class
class LEDController {
private:
  CRGB leds1[NUM_LEDS_PER_STRIP];
  CRGB leds2[NUM_LEDS_PER_STRIP];
  Config* config;
  uint8_t currentMode;
  unsigned long lastUpdate;
  unsigned long impactEffectStart;
  bool impactEffectActive;
  
  // Effect variables
  uint16_t effectStep;
  uint8_t effectSpeed;
  
  // Effect functions
  void updateFireEffect();
  void updatePulseEffect();
  void updateRainbowEffect();
  void updateStrobeEffect();
  void updateSolidEffect();
  
public:
  LEDController() : currentMode(0), lastUpdate(0), effectStep(0), effectSpeed(30), 
                   impactEffectStart(0), impactEffectActive(false) {}
  
  void begin(Config* cfg);
  void update();
  void setMode(uint8_t mode);
  void triggerImpactEffect();
  void setBrightness(uint8_t brightness);
  
  // Getters for LED arrays
  CRGB* getLeds1() { return leds1; }
  CRGB* getLeds2() { return leds2; }
};

// Button handler class
class ButtonHandler {
private:
  Config* config;
  bool buttonState;
  bool lastButtonState;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;
  bool modeChange;
  
public:
  ButtonHandler() : buttonState(false), lastButtonState(false), 
                   lastDebounceTime(0), debounceDelay(50), modeChange(false) {}
  
  void begin(Config* cfg);
  void handle();
  bool modeChangeRequested();
};

// Accelerometer handler class
class AccelerometerHandler {
private:
  Adafruit_MPU6050 mpu;
  Config* config;
  bool mpuInitialized;
  bool impactDetectedFlag;
  unsigned long lastImpactTime;
  unsigned long impactCooldown;
  
public:
  AccelerometerHandler() : mpuInitialized(false), impactDetectedFlag(false), 
                           lastImpactTime(0), impactCooldown(500) {}
  
  bool begin(Config* cfg);
  void update();
  bool impactDetected();
  void calibrate();
};

// Settings manager class for storing configuration in flash
class SettingsManager {
private:
  // EEPROM address where settings are stored
  const int configAddress = 0;
  
public:
  void begin();
  bool loadSettings(Config* cfg);
  void saveSettings(Config* cfg);
};

#endif // BOSTAFF_H
