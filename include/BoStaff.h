#ifndef BOSTAFF_H
#define BOSTAFF_H

#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "effects.h"

// Pin definitions - UPDATED FOR SINGLE STRIP
#define LED_PIN D7  // GPIO13 - Single LED strip
#define BTN_PIN D6  // GPIO12 - Button pin

// MPU6050 accelerometer pins (I2C) - Using default pins
#define SDA_PIN D2  // GPIO4 - Default I2C data pin
#define SCL_PIN D1  // GPIO5 - Default I2C clock pin

// LED strip configuration - UPDATED FOR SINGLE STRIP
#define NUM_LEDS_TOTAL 400
#define NUM_LEDS_SEGMENT 100  // Each of the 4 segments has 100 LEDs

// Segment indices for easier reference
// First half (original strip 1)
#define SEGMENT_1_START 0     // First segment - counts up
#define SEGMENT_1_END 99
#define SEGMENT_2_START 199   // Second segment - counts down
#define SEGMENT_2_END 100
// Second half (original strip 2)
#define SEGMENT_3_START 200   // Third segment - counts up
#define SEGMENT_3_END 299
#define SEGMENT_4_START 399   // Fourth segment - counts down
#define SEGMENT_4_END 300

// Global configuration structure
struct Config {
  uint8_t currentMode = EFFECT_FIRE;   // Default mode
  uint8_t brightness = 25;            // 10% of 255
  uint8_t impactBrightness = 25;     // 10% of 255
  uint8_t numModes = NUM_EFFECTS;      // Number of available modes
  uint16_t impactThreshold = 1600;     // ~1.6G acceleration
  uint16_t impactFlashDuration = 100;  // Duration of impact flash in ms
};

// LED Controller class - UPDATED FOR SINGLE STRIP
class LEDController {
private:
  CRGB leds[NUM_LEDS_TOTAL];  // Single array for all LEDs
  Config* config;
  uint8_t currentMode;
  unsigned long lastUpdate;
  uint16_t effectStep;
  uint8_t effectSpeed;
  unsigned long impactEffectStart;
  bool impactEffectActive;
  uint8_t normalBrightness; // Store normal brightness to restore after impact
  
  // Effect functions
  void updateFireEffect();
  void updatePulseEffect();
  void updateRainbowEffect();
  void updateStrobeEffect();
  void updateSolidEffect();
  
  // Helper function to map a virtual position (0-99) to the actual folded LED position
  // This handles the four segment arrangement
  int mapToFoldedIndex(int virtualPos, int segment);
  
public:
  LEDController() : currentMode(0), lastUpdate(0), effectStep(0), effectSpeed(30), 
                    impactEffectStart(0), impactEffectActive(false), normalBrightness(25) {}
  
  void begin(Config* cfg);
  void update();
  void setMode(uint8_t mode);
  void triggerImpactEffect();
  void setBrightness(uint8_t brightness);
  void forceRefresh();
  
  // Getter for LED array
  CRGB* getLeds() { return leds; }
  
  // Functions to access LED segments with proper folding logic
  CRGB& getSegment1LED(int pos); // 0-99 (counts up)
  CRGB& getSegment2LED(int pos); // 0-99 (counts down from end)
  CRGB& getSegment3LED(int pos); // 0-99 (counts up)
  CRGB& getSegment4LED(int pos); // 0-99 (counts down from end)
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
  
  // Helper method for calibration
  void waitForButtonPress();
  
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