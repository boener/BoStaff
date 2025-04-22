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
PulseEffect* pulseEffect = nullptr;
RainbowEffect* rainbowEffect = nullptr;
StrobeEffect* strobeEffect = nullptr;

// Effect parameters
EffectParams effectParams[NUM_EFFECTS];

// Variables for calibration trigger
const unsigned long CALIBRATION_LONG_PRESS = 5000; // 5 seconds for calibration trigger
bool calibrationMode = false;
unsigned long buttonPressStart = 0;
bool buttonWasPressed = false;

// I2C and LED timing control variables
unsigned long lastAccelUpdate = 0;
const unsigned long ACCEL_UPDATE_INTERVAL = 25; // Only read accelerometer every 25ms to reduce I2C traffic

// Function to initialize all effect objects - UPDATED FOR SINGLE STRIP
void initializeAllEffects() {
  // Clear any existing effects first
  if (fireEffect) delete fireEffect;
  if (pulseEffect) delete pulseEffect;
  if (rainbowEffect) delete rainbowEffect;
  if (strobeEffect) delete strobeEffect;
  
  // Set all pointers to null (important to prevent dangling pointers)
  fireEffect = nullptr;
  pulseEffect = nullptr;
  rainbowEffect = nullptr;
  strobeEffect = nullptr;
  
  // Now create all effects fresh
  bool allEffectsInitialized = true;
  
  // Try to create FireEffect instance
  fireEffect = new FireEffect(&ledController);
  
  if (!fireEffect || !fireEffect->isInitialized()) {
    Serial.println(F("Error initializing FireEffect!"));
    allEffectsInitialized = false;
  }
  
  // Try to create PulseEffect instance
  pulseEffect = new PulseEffect(&ledController);
  
  if (!pulseEffect || !pulseEffect->isInitialized()) {
    Serial.println(F("Error initializing PulseEffect!"));
    allEffectsInitialized = false;
  }
  
  // Try to create RainbowEffect instance
  rainbowEffect = new RainbowEffect(&ledController);
  
  if (!rainbowEffect || !rainbowEffect->isInitialized()) {
    Serial.println(F("Error initializing RainbowEffect!"));
    allEffectsInitialized = false;
  }
  
  // Try to create StrobeEffect instance
  strobeEffect = new StrobeEffect(&ledController);
  
  if (!strobeEffect || !strobeEffect->isInitialized()) {
    Serial.println(F("Error initializing StrobeEffect!"));
    allEffectsInitialized = false;
  }
  
  // Clear the LED array to ensure clean start
  fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Black);
  
  // Use centralized safe strip refresh
  ledController.safeStripRefresh();
  
  if (allEffectsInitialized) {
    Serial.println(F("All LED effects initialized successfully"));
  } else {
    Serial.println(F("WARNING: Some LED effects failed to initialize properly"));
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("\nBoStaff Controller Starting"));
  Serial.print(F("Version: ")); Serial.println(VERSION);
  Serial.print(F("Build: ")); Serial.print(BUILD_DATE); Serial.print(" "); Serial.println(BUILD_TIME);
  
  // Display pin configuration
  Serial.println(F("\nPin Configuration:"));
  Serial.print(F("LED Strip: ")); Serial.print(F("D7 (GPIO13)")); Serial.println(F(" - Single strip with 400 LEDs"));
  Serial.print(F("MPU-6050 SCL: ")); Serial.println(F("D1 (GPIO5)"));
  Serial.print(F("MPU-6050 SDA: ")); Serial.println(F("D2 (GPIO4)"));
  Serial.print(F("Button: ")); Serial.println(F("D6 (GPIO12)"));
  
  // Initialize I2C for MPU-6050 (uses default pins D1/D2)
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println(F("I2C initialized"));
  
  // Load settings from flash
  settingsManager.begin();
  settingsManager.loadSettings(&config);
  
  // Initialize button - must be before LED controller to ensure proper boot state
  buttonHandler.begin(&config);
  
  // Initialize LED controller
  ledController.begin(&config);
  
  // Initialize accelerometer
  accelHandler.begin(&config);
  
  // Initialize power management
  powerManager.begin();
  
  Serial.println(F("Initializing LED effects for single-strip with four segments:"));
  Serial.println(F("Segment 1: LEDs 0-99 (counts up)"));
  Serial.println(F("Segment 2: LEDs 100-199 (counts down from 199)"));
  Serial.println(F("Segment 3: LEDs 200-299 (counts up)"));
  Serial.println(F("Segment 4: LEDs 300-399 (counts down from 399)"));
  
  // Initialize all effects
  initializeAllEffects();
  
  // Set the initial mode
  ledController.setMode(config.currentMode);
  
  Serial.println(F("Setup complete!"));
  
  // Show battery status
  float batteryVoltage = powerManager.getBatteryVoltage();
  float batteryPercentage = powerManager.getBatteryPercentage();
  Serial.print(F("Battery: ")); Serial.print(batteryVoltage); Serial.print(F("V, ")); 
  Serial.print(batteryPercentage); Serial.println(F("%"));
  
  // Print calibration instructions
  Serial.println(F("\nTo enter accelerometer calibration mode,"));
  Serial.println(F("hold the button for 5 seconds until all LEDs flash blue."));
  
  // Initialize loop timing measurement
  lastAccelUpdate = millis();
}

void loop() {
  // Check for calibration mode trigger (long button press)
  if (digitalRead(BTN_PIN) == LOW) {  // Button pressed (active LOW)
    if (!buttonWasPressed) {
      buttonWasPressed = true;
      buttonPressStart = millis();
    } else if (!calibrationMode && (millis() - buttonPressStart) > CALIBRATION_LONG_PRESS) {
      // Long press detected, enter calibration mode
      calibrationMode = true;
      
      // Clear all LEDs completely before visual feedback
      fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Black);
      ledController.safeStripRefresh();
      
      delay(100);
      
      // Visual feedback - flash LEDs blue to indicate calibration mode
      fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Blue);
      ledController.safeStripRefresh();
      
      delay(500);
      
      // Clear all LEDs completely 
      fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Black);
      ledController.safeStripRefresh();
      
      delay(500);
      
      Serial.println(F("\n*** ENTERING CALIBRATION MODE ***"));
      
      // Start the calibration process
      accelHandler.calibrate();
      
      // Save the new threshold value
      settingsManager.saveSettings(&config);
      
      Serial.print(F("New impact threshold saved: "));
      Serial.println(config.impactThreshold);
      
      // Visual feedback - flash LEDs green to indicate calibration complete
      fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Green);
      ledController.safeStripRefresh();
      
      delay(1000);
      
      // Clear all LEDs completely before restoring normal operation
      fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Black);
      ledController.safeStripRefresh();
      
      // Completely reinitialize all effect objects to ensure clean state
      initializeAllEffects();
      
      // Reset calibration mode
      calibrationMode = false;
      
      // Make sure brightness is restored to normal
      ledController.setBrightnessMode(LEDController::BRIGHTNESS_NORMAL);
      
      // Restore current LED effect
      ledController.setMode(config.currentMode);
      
      // Force a clean update of the strips
      ledController.forceRefresh();
    }
  } else {
    buttonWasPressed = false;  // Button released
  }
  
  // Skip normal operation while in calibration mode
  if (calibrationMode) {
    return;
  }
  
  // Update button state
  buttonHandler.handle();
  
  // Check for mode change request from button
  if (buttonHandler.modeChangeRequested()) {
    config.currentMode = (config.currentMode + 1) % config.numModes;
    Serial.print(F("Mode changed to: ")); Serial.print(config.currentMode); 
    Serial.print(F(" (")); Serial.print(EFFECT_NAMES[config.currentMode]); Serial.println(F(")"));
    
    ledController.setMode(config.currentMode);
    settingsManager.saveSettings(&config);
    powerManager.resetActivityTimer();
  }
  
  // Only read accelerometer at a controlled rate to avoid I2C timing conflicts
  if (millis() - lastAccelUpdate >= ACCEL_UPDATE_INTERVAL) {
    // Read accelerometer and detect impacts
    accelHandler.update();
    lastAccelUpdate = millis();
    
    // If impact detected, trigger flash effect
    if (accelHandler.impactDetected()) {
      ledController.triggerImpactEffect();
      powerManager.resetActivityTimer();
    }
  }
  
  // Check if PowerManager has requested a brightness change
  if (powerManager.needsBrightnessChange()) {
    // Use the new brightness mode system
    BrightnessMode newMode = powerManager.getRequestedBrightnessMode();
    
    // Map the BrightnessMode to LEDController::BrightnessMode
    LEDController::BrightnessMode ledMode;
    switch(newMode) {
      case BRIGHTNESS_NORMAL:
        ledMode = LEDController::BRIGHTNESS_NORMAL;
        break;
      case BRIGHTNESS_IMPACT:
        ledMode = LEDController::BRIGHTNESS_IMPACT;
        break;
      case BRIGHTNESS_LOW_BATTERY:
        ledMode = LEDController::BRIGHTNESS_LOW_BATTERY;
        break;
      case BRIGHTNESS_SLEEP:
        ledMode = LEDController::BRIGHTNESS_SLEEP;
        break;
      default:
        ledMode = LEDController::BRIGHTNESS_NORMAL;
        break;
    }
    
    // Apply the brightness mode
    ledController.setBrightnessMode(ledMode);
    
    Serial.print("Brightness mode changed via PowerManager to: ");
    Serial.println(static_cast<int>(newMode)); // Cast to int for readable output
    
    // Clear the request flag
    powerManager.clearBrightnessRequest();
  }
  
  // Update LED effects based on current mode
  switch (config.currentMode) {
    case EFFECT_FIRE:
      if (fireEffect && fireEffect->isInitialized()) {
        fireEffect->update();
      } else {
        // Fallback to a simple effect if fire effect is not available
        fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Red);
      }
      break;
      
    case EFFECT_PULSE:
      if (pulseEffect && pulseEffect->isInitialized()) {
        pulseEffect->update();
      } else {
        // Fallback effect
        fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Blue);
      }
      break;
      
    case EFFECT_RAINBOW:
      if (rainbowEffect && rainbowEffect->isInitialized()) {
        rainbowEffect->update();
      } else {
        // Fallback effect
        fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::Green);
      }
      break;
      
    case EFFECT_STROBE:
      if (strobeEffect && strobeEffect->isInitialized()) {
        strobeEffect->update();
      } else {
        // Fallback effect
        fill_solid(ledController.getLeds(), NUM_LEDS_TOTAL, CRGB::White);
      }
      break;
      
    case EFFECT_SOLID:
    default:
      // Solid color effect is handled directly by LED controller
      break;
  }
  
  // Update LED strips
  ledController.update();
  
  // Update power management
  powerManager.update();
  
  // Small delay to prevent watchdog issues
  yield();
}