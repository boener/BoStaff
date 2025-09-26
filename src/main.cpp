#include <Arduino.h>
#include "BoStaff.h"
#include "hardware.h"
#include "effects.h"
#include "EffectsConfig.h"

// Global configuration
Config config;

// LED strip controller
LEDController ledController;

// Button handler
ButtonHandler buttonHandler;

// Accelerometer for impact detection
AccelerometerHandler accelHandler;

// Settings manager for flash storage
SettingsManager settingsManager;

// Power management
PowerManager powerManager;

// Effect instances - UPDATED FOR SINGLE STRIP
FireEffect* fireEffect = nullptr;
FireEffect1* fireEffect1 = nullptr;
FireEffect2* fireEffect2 = nullptr;
PulseEffect* pulseEffect = nullptr;
PulseEffect1* pulseEffect1 = nullptr;  // Added for new pulse effect variant
PulseEffect2* pulseEffect2 = nullptr;  // Added for new pulse effect variant
RainbowEffect* rainbowEffect = nullptr;
StrobeEffect* strobeEffect = nullptr;

// Effect parameters
EffectParams effectParams[NUM_EFFECTS];

// OLD CALIBRATION SYSTEM VARIABLES - REMOVED FOR DUAL-SENSOR SYSTEM
/*
const unsigned long CALIBRATION_LONG_PRESS = 5000; // 5 seconds for calibration trigger
bool calibrationMode = false;
unsigned long buttonPressStart = 0;
bool buttonWasPressed = false;
*/

// Timing management for task scheduling
unsigned long lastAccelUpdate = 0;
// Increased interval to reduce I2C conflicts (increased from 25ms to 40ms)
const unsigned long ACCEL_UPDATE_INTERVAL = 5;

// Timing management for better task distribution
unsigned long lastLEDUpdate = 0;
const unsigned long LED_UPDATE_INTERVAL = 15; // LED effects update every 15ms
unsigned long lastPowerUpdate = 0;
const unsigned long POWER_UPDATE_INTERVAL = 500; // Power check every 500ms

// Function to initialize all effect objects - UPDATED FOR SINGLE STRIP
void initializeAllEffects() {
  // Clear any existing effects first
  if (fireEffect) delete fireEffect;
  if (fireEffect1) delete fireEffect1;
  if (fireEffect2) delete fireEffect2;
  if (pulseEffect) delete pulseEffect;
  if (pulseEffect1) delete pulseEffect1;  // Delete new pulse effect if exists
  if (pulseEffect2) delete pulseEffect2;  // Delete new pulse effect if exists
  if (rainbowEffect) delete rainbowEffect;
  if (strobeEffect) delete strobeEffect;
  
  // Set all pointers to null (important to prevent dangling pointers)
  fireEffect = nullptr;
  fireEffect1 = nullptr;
  fireEffect2 = nullptr;
  pulseEffect = nullptr;
  pulseEffect1 = nullptr;  // Reset pointer to null
  pulseEffect2 = nullptr;  // Reset pointer to null
  rainbowEffect = nullptr;
  strobeEffect = nullptr;
  
  // Now create all effects fresh
  bool allEffectsInitialized = true;
  
  // Try to create FireEffect instance
  fireEffect = new FireEffect(&ledController);
  
  if (!fireEffect || !fireEffect->isInitialized()) {
    DEBUG_PRINTLN_F("Error initializing FireEffect!");
    allEffectsInitialized = false;
  }
  
  // Try to create FireEffect1 instance
  fireEffect1 = new FireEffect1(&ledController);
  
  if (!fireEffect1 || !fireEffect1->isInitialized()) {
    DEBUG_PRINTLN_F("Error initializing FireEffect1!");
    allEffectsInitialized = false;
  }
  
  // Try to create FireEffect2 instance
  fireEffect2 = new FireEffect2(&ledController);
  
  if (!fireEffect2 || !fireEffect2->isInitialized()) {
    DEBUG_PRINTLN_F("Error initializing FireEffect2!");
    allEffectsInitialized = false;
  }
  
  // Try to create PulseEffect instance
  pulseEffect = new PulseEffect(&ledController);
  
  if (!pulseEffect || !pulseEffect->isInitialized()) {
    DEBUG_PRINTLN_F("Error initializing PulseEffect!");
    allEffectsInitialized = false;
  }
  
  // Try to create PulseEffect1 instance
  pulseEffect1 = new PulseEffect1(&ledController);
  
  if (!pulseEffect1 || !pulseEffect1->isInitialized()) {
    DEBUG_PRINTLN_F("Error initializing PulseEffect1!");
    allEffectsInitialized = false;
  }
  
  // Try to create PulseEffect2 instance
  pulseEffect2 = new PulseEffect2(&ledController);
  
  if (!pulseEffect2 || !pulseEffect2->isInitialized()) {
    DEBUG_PRINTLN_F("Error initializing PulseEffect2!");
    allEffectsInitialized = false;
  }
  
  // Try to create RainbowEffect instance
  rainbowEffect = new RainbowEffect(&ledController);
  
  if (!rainbowEffect || !rainbowEffect->isInitialized()) {
    DEBUG_PRINTLN_F("Error initializing RainbowEffect!");
    allEffectsInitialized = false;
  }
  
  // Try to create StrobeEffect instance
  strobeEffect = new StrobeEffect(&ledController);
  
  if (!strobeEffect || !strobeEffect->isInitialized()) {
    DEBUG_PRINTLN_F("Error initializing StrobeEffect!");
    allEffectsInitialized = false;
  }
  
  // Clear the LED array to ensure clean start
  fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Black);
  
  // Use centralized safe strip refresh
  ledController.safeStripRefresh();
  
  if (allEffectsInitialized) {
    DEBUG_PRINTLN_F("All LED effects initialized successfully");
  } else {
    DEBUG_PRINTLN_F("WARNING: Some LED effects failed to initialize properly");
  }
}

void setup() {
  // Initialize serial communication
  DEBUG_BEGIN(SERIAL_BAUD);
  DEBUG_PRINTLN_F("\nBoStaff Controller Starting");
  DEBUG_PRINT_F("Version: "); DEBUG_PRINTLN(VERSION);
  DEBUG_PRINT_F("Build: "); DEBUG_PRINT(BUILD_DATE); DEBUG_PRINT(" "); DEBUG_PRINTLN(BUILD_TIME);
  
  // Display pin configuration
  DEBUG_PRINTLN_F("\nPin Configuration:");
  DEBUG_PRINT_F("LED Strip: "); DEBUG_PRINT_F("D7 (GPIO13)"); DEBUG_PRINTLN_F(" - Single strip with 400 LEDs");
  DEBUG_PRINT_F("MPU-6050 SCL: "); DEBUG_PRINTLN_F("D1 (GPIO5)");
  DEBUG_PRINT_F("MPU-6050 SDA: "); DEBUG_PRINTLN_F("D2 (GPIO4)");
  DEBUG_PRINT_F("Button: "); DEBUG_PRINTLN_F("D6 (GPIO12)");
  
  // Initialize I2C for MPU-6050 (uses default pins D1/D2)
  // Note: Wire.begin is now handled in AccelerometerHandler for better control
  DEBUG_PRINTLN_F("I2C will be initialized by AccelerometerHandler");
  
  // Load settings from flash
  settingsManager.begin();
  settingsManager.loadSettings(&config);
  
  // Initialize button - must be before LED controller to ensure proper boot state
  buttonHandler.begin(&config);
  
  // Initialize LED controller
  ledController.begin(&config);
  
  // Initialize power management
  powerManager.begin();
  
  // Initialize accelerometer (after other components to avoid I2C conflicts)
  if (accelHandler.begin(&config)) {
    DEBUG_PRINTLN_F("Dual-sensor accelerometer system initialized successfully");
  } else {
    DEBUG_PRINTLN_F("WARNING: Accelerometer initialization had issues, will retry in main loop");
  }
  
  DEBUG_PRINTLN_F("Initializing LED effects for single-strip with four segments:");
  DEBUG_PRINTLN_F("Segment 1: LEDs 0-99 (counts up)");
  DEBUG_PRINTLN_F("Segment 2: LEDs 100-199 (counts down from 199)");
  DEBUG_PRINTLN_F("Segment 3: LEDs 200-299 (counts up)");
  DEBUG_PRINTLN_F("Segment 4: LEDs 300-399 (counts down from 399)");
  
  // Initialize all effects
  initializeAllEffects();
  
  // Set the initial mode
  ledController.setMode(config.currentMode);
  
  DEBUG_PRINTLN_F("Setup complete!");
  
  // Show battery status
  float batteryVoltage = powerManager.getBatteryVoltage();
  float batteryPercentage = powerManager.getBatteryPercentage();
  DEBUG_PRINT_F("Battery: "); DEBUG_PRINT(batteryVoltage); DEBUG_PRINT_F("V, "); 
  DEBUG_PRINT(batteryPercentage); DEBUG_PRINTLN_F("%");
  
  // DUAL-SENSOR SYSTEM INFO - CALIBRATION NO LONGER NEEDED
  DEBUG_PRINTLN_F("\nDual-Sensor Impact Detection System Active");
  DEBUG_PRINTLN_F("Calibration not required - using optimized fixed thresholds");
  DEBUG_PRINT_F("Accelerometer Threshold: "); DEBUG_PRINTLN(IMPACT_ACCEL_THRESHOLD);
  DEBUG_PRINT_F("Rotation Classification: "); DEBUG_PRINTLN(ROTATION_CLASSIFICATION_THRESHOLD);
  
  // Initialize loop timing variables
  lastAccelUpdate = millis();
  lastLEDUpdate = millis();
  lastPowerUpdate = millis();
}

void loop() {
  // OLD CALIBRATION TRIGGER CODE - REMOVED FOR DUAL-SENSOR SYSTEM
  /*
  // Check for calibration mode trigger (long button press)
  if (digitalRead(BTN_PIN) == LOW) {  // Button pressed (active LOW)
    if (!buttonWasPressed) {
      buttonWasPressed = true;
      buttonPressStart = millis();
    } else if (!calibrationMode && (millis() - buttonPressStart) > CALIBRATION_LONG_PRESS) {
      // Long press detected, enter calibration mode
      calibrationMode = true;
      
      // [... calibration logic removed ...]
      
      // Start the calibration process
      accelHandler.calibrate();
      
      // [... rest of calibration code removed ...]
    }
  } else {
    buttonWasPressed = false;  // Button released
  }
  
  // Skip normal operation while in calibration mode
  if (calibrationMode) {
    yield(); // Allow watchdog to be fed
    return;
  }
  */
  
  // Update button state - highest priority task
  buttonHandler.handle();
  
  // Check for mode change request from button
  if (buttonHandler.modeChangeRequested()) {
    config.currentMode = (config.currentMode + 1) % config.numModes;
    DEBUG_PRINT_F("Mode changed to: "); DEBUG_PRINT(config.currentMode); 
    DEBUG_PRINT_F(" ("); DEBUG_PRINT(EFFECT_NAMES[config.currentMode]); DEBUG_PRINTLN_F(")");
    
    ledController.setMode(config.currentMode);
    settingsManager.saveSettings(&config);
    powerManager.resetActivityTimer();
  }
  
  // Only read accelerometer at a controlled rate to avoid I2C timing conflicts
  if (millis() - lastAccelUpdate >= ACCEL_UPDATE_INTERVAL) {
    // Read accelerometer and detect impacts using dual-sensor system
    accelHandler.update();
    lastAccelUpdate = millis();
    
    // If impact detected, trigger flash effect
    if (accelHandler.impactDetected()) {
      ledController.triggerImpactEffect();
      powerManager.resetActivityTimer();
    }
    
    // Allow time between I2C and LED updates to prevent conflicts
    yield();
  }
  
  // Update LED effects on their own schedule
  if (millis() - lastLEDUpdate >= LED_UPDATE_INTERVAL) {
    // Update LED effects based on current mode - UPDATED FOR NEW EFFECT INDICES
    switch (config.currentMode) {
      case EFFECT_FIRE:  // Now index 7
        if (fireEffect && fireEffect->isInitialized()) {
          fireEffect->update();
        } else {
          // Fallback to a simple effect if fire effect is not available
          fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Red);
        }
        break;
        
      case EFFECT_FIRE1:  // Now index 8
        if (fireEffect1 && fireEffect1->isInitialized()) {
          fireEffect1->update();
        } else {
          // Fallback to a simple effect
          fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Blue);
        }
        break;
        
      case EFFECT_FIRE2:  // Now index 9
        if (fireEffect2 && fireEffect2->isInitialized()) {
          fireEffect2->update();
        } else {
          // Fallback to a simple effect
          fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Green);
        }
        break;
        
      case EFFECT_PULSE:  // Now index 10
        if (pulseEffect && pulseEffect->isInitialized()) {
          pulseEffect->update();
        } else {
          // Fallback effect
          fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Blue);
        }
        break;
        
      case EFFECT_PULSE1:  // Now index 11
        if (pulseEffect1 && pulseEffect1->isInitialized()) {
          pulseEffect1->update();
        } else {
          // Fallback effect
          fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Green);
        }
        break;
        
      case EFFECT_PULSE2:  // Now index 12
        if (pulseEffect2 && pulseEffect2->isInitialized()) {
          pulseEffect2->update();
        } else {
          // Fallback effect
          fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Red);
        }
        break;
        
      case EFFECT_RAINBOW:  // Now index 13
        if (rainbowEffect && rainbowEffect->isInitialized()) {
          rainbowEffect->update();
        } else {
          // Fallback effect
          fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Green);
        }
        break;
        
      case EFFECT_STROBE:  // Now index 14
        if (strobeEffect && strobeEffect->isInitialized()) {
          strobeEffect->update();
        } else {
          // Fallback effect
          fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::White);
        }
        break;
        
      // All other effects (including new solid colors and slow rainbow) are handled by LEDController
      default:
        // No action needed - LEDController.update() will handle these
        break;
    }
    
    // Update LED strips - this will handle solid colors and slow rainbow
    ledController.update();
    
    lastLEDUpdate = millis();
  }
  
  // Check power management on a less frequent schedule
  if (millis() - lastPowerUpdate >= POWER_UPDATE_INTERVAL) {
    // Update power management
    powerManager.update();
    
    // Check if PowerManager has requested a brightness change
    if (powerManager.needsBrightnessChange()) {
      // Get the brightness mode directly from PowerManager
      BrightnessMode newMode = powerManager.getRequestedBrightnessMode();
      
      // Apply the brightness mode directly (now using the same enum)
      ledController.setBrightnessMode(newMode);
      
      DEBUG_PRINT("Brightness mode changed via PowerManager to: ");
      DEBUG_PRINTLN(static_cast<int>(newMode)); // Cast to int for readable output
      
      // Clear the request flag
      powerManager.clearBrightnessRequest();
    }
    
    lastPowerUpdate = millis();
  }
  
  // Small delay to prevent watchdog issues
  yield();
}
