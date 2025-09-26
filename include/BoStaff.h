#ifndef BOSTAFF_H
#define BOSTAFF_H

#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "effects.h"
#include "EffectsConfig.h" // Include the new config file

// Pin definitions - UPDATED FOR SINGLE STRIP
#define LED_PIN D7  // GPIO13 - Single LED strip
#define BTN_PIN D6  // GPIO12 - Button pin

// MPU6050 accelerometer pins (I2C) - Using default pins
#define SDA_PIN D2  // GPIO4 - Default I2C data pin
#define SCL_PIN D1  // GPIO5 - Default I2C clock pin

// I2C timing settings for reliable operation
#define I2C_CLOCK_SPEED 100000  // 100 kHz standard I2C speed
#define I2C_TIMEOUT 50          // 50ms timeout for I2C operations
#define I2C_RETRY_COUNT 3       // Number of times to retry I2C operations

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

// Impact type enumeration for classification (future use)
enum ImpactType {
  IMPACT_NONE,      // No impact detected
  IMPACT_STAB,      // Stabbing motion (low gyro, high accel)
  IMPACT_ROTATION   // Rotational motion (high gyro)
};

// Global configuration structure
struct Config {
  uint8_t currentMode = EFFECT_FIRE;   // Default mode - will use EFFECT_FIRE which is now index 7
  uint8_t brightness = DEFAULT_BRIGHTNESS;            // Default from config
  uint8_t impactBrightness = IMPACT_BRIGHTNESS;     // From config
  uint8_t numModes = NUM_EFFECTS;      // Number of available modes
  
  // OLD SINGLE-SENSOR THRESHOLD - REPLACED WITH DUAL-SENSOR SYSTEM
  // uint16_t impactThreshold = IMPACT_THRESHOLD;     // Default from config
  
  // Impact classification tracking (for future use)
  ImpactType lastImpactType = IMPACT_NONE;    // Type of last detected impact
  uint32_t totalStabImpacts = 0;              // Count of stab impacts
  uint32_t totalRotationImpacts = 0;          // Count of rotation impacts
  
  uint16_t impactFlashDuration = IMPACT_FLASH_DURATION;  // Duration of impact flash from config
  bool impactFadeOut = IMPACT_FADE_OUT;  // Whether to fade out after impact
  uint8_t impactFadeRate = IMPACT_FADE_RATE;  // How quickly impact fades
};

// Global BrightnessMode enum that can be used by multiple classes
enum BrightnessMode {
  BRIGHTNESS_NORMAL,      // Regular operating brightness
  BRIGHTNESS_IMPACT,      // Brightness during impact effect
  BRIGHTNESS_LOW_BATTERY, // Reduced brightness for low battery
  BRIGHTNESS_SLEEP        // Very dim brightness before sleep
};

// LED Controller class - UPDATED FOR SINGLE STRIP
class LEDController {
public:
  LEDController() : currentMode(0), lastUpdate(0), effectStep(0), effectSpeed(30), 
                  impactEffectStart(0), impactEffectActive(false), 
                  normalBrightness(DEFAULT_BRIGHTNESS),
                  currentBrightnessMode(BRIGHTNESS_NORMAL),
                  savedBrightness(DEFAULT_BRIGHTNESS) {}
  
  // Safe strip refresh method (public so it can be accessed from main)
  void safeStripRefresh();
  
  void begin(Config* cfg);
  void update();
  void setMode(uint8_t mode);
  void triggerImpactEffect();
  
  // Enhanced brightness management
  void setBrightness(uint8_t brightness); // Set specific brightness value
  void setBrightnessMode(BrightnessMode mode); // Set brightness by mode
  void restorePreviousBrightness(); // Restore previous brightness
  uint8_t getCurrentBrightness(); // Get current brightness
  
  void forceRefresh();
  
  // Getter for LED array
  CRGB* getLeds() { return leds; }
  
  // Functions to access LED segments with proper folding logic
  CRGB& getSegment1LED(int pos); // 0-99 (counts up)
  CRGB& getSegment2LED(int pos); // 0-99 (counts down from end)
  CRGB& getSegment3LED(int pos); // 0-99 (counts up)
  CRGB& getSegment4LED(int pos); // 0-99 (counts down from end)

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
  
  BrightnessMode currentBrightnessMode;
  uint8_t savedBrightness; // For restoring previous brightness
  
  // Effect functions
  void updateFireEffect();
  void updatePulseEffect();
  void updateRainbowEffect();
  void updateStrobeEffect();
  void updateSlowRainbowEffect();  // Renamed from updateSolidEffect
  void updateSolidBlueEffect();    // New solid color effects
  void updateSolidGreenEffect();
  void updateSolidRedEffect();
  void updateSolidPurpleEffect();
  void updateSolidYellowEffect();
  void updateSolidWhiteEffect();
  
  // Helper function to map a virtual position (0-99) to the actual folded LED position
  // This handles the four segment arrangement
  int mapToFoldedIndex(int virtualPos, int segment);
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

// Accelerometer handler class - Enhanced for dual-sensor impact detection
class AccelerometerHandler {
private:
  Adafruit_MPU6050 mpu;
  Config* config;
  bool mpuInitialized;
  bool impactDetectedFlag;
  unsigned long lastImpactTime;
  unsigned long impactCooldown;
  
  // Gyro delta detection variables
  uint16_t previousGyroRaw;      // Store previous gyro magnitude for delta calculation
  bool firstReading;             // Flag to skip delta calculation on first reading
  
  // I2C management variables
  byte consecutiveErrors;
  unsigned long lastRecoveryAttempt;
  
  // Helper methods
  void configureI2C();      // New centralized I2C configuration method
  void waitForButtonPress();
  bool setupMPU();          // Separate MPU setup method for better error handling
  bool readMPUData(sensors_event_t* a, sensors_event_t* g, sensors_event_t* temp); // Enhanced I2C read method
  bool recoverI2C();        // I2C recovery method in case of errors
  
public:
  AccelerometerHandler() : mpuInitialized(false), impactDetectedFlag(false), 
                         lastImpactTime(0), impactCooldown(IMPACT_COOLDOWN),
                         previousGyroRaw(0), firstReading(true),
                         consecutiveErrors(0), lastRecoveryAttempt(0) {}
  
  bool begin(Config* cfg);
  void update();
  bool impactDetected();
  
  // OLD CALIBRATION METHOD - REMOVED IN DUAL-SENSOR SYSTEM
  // void calibrate();
  
  // Status methods
  bool isInitialized() const { return mpuInitialized; }
  byte getErrorCount() const { return consecutiveErrors; }
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