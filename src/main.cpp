#include <Arduino.h>
#include "BoStaff.h"
#include "hardware.h"
#include "effects.h"

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

// Effect instances
FireEffect* fireEffect1 = nullptr;
FireEffect* fireEffect2 = nullptr;
PulseEffect* pulseEffect1 = nullptr;
PulseEffect* pulseEffect2 = nullptr;
RainbowEffect* rainbowEffect1 = nullptr;
RainbowEffect* rainbowEffect2 = nullptr;
StrobeEffect* strobeEffect1 = nullptr;
StrobeEffect* strobeEffect2 = nullptr;

// Effect parameters
EffectParams effectParams[NUM_EFFECTS];

void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("\nBoStaff Controller Starting"));
  Serial.print(F("Version: ")); Serial.println(VERSION);
  Serial.print(F("Build: ")); Serial.print(BUILD_DATE); Serial.print(" "); Serial.println(BUILD_TIME);
  
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
  
  // Initialize effect objects
  fireEffect1 = new FireEffect(ledController.getLeds1(), NUM_LEDS_PER_STRIP);
  fireEffect2 = new FireEffect(ledController.getLeds2(), NUM_LEDS_PER_STRIP, true); // reversed
  
  pulseEffect1 = new PulseEffect(ledController.getLeds1(), NUM_LEDS_PER_STRIP);
  pulseEffect2 = new PulseEffect(ledController.getLeds2(), NUM_LEDS_PER_STRIP);
  
  rainbowEffect1 = new RainbowEffect(ledController.getLeds1(), NUM_LEDS_PER_STRIP);
  rainbowEffect2 = new RainbowEffect(ledController.getLeds2(), NUM_LEDS_PER_STRIP);
  
  strobeEffect1 = new StrobeEffect(ledController.getLeds1(), NUM_LEDS_PER_STRIP);
  strobeEffect2 = new StrobeEffect(ledController.getLeds2(), NUM_LEDS_PER_STRIP);
  
  // Set the initial mode
  ledController.setMode(config.currentMode);
  
  Serial.println(F("Setup complete!"));
  
  // Show battery status
  float batteryVoltage = powerManager.getBatteryVoltage();
  float batteryPercentage = powerManager.getBatteryPercentage();
  Serial.print(F("Battery: ")); Serial.print(batteryVoltage); Serial.print(F("V, ")); 
  Serial.print(batteryPercentage); Serial.println(F("%"));
}

void loop() {
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
  
  // Read accelerometer and detect impacts
  accelHandler.update();
  
  // If impact detected, trigger flash effect
  if (accelHandler.impactDetected()) {
    ledController.triggerImpactEffect();
    powerManager.resetActivityTimer();
  }
  
  // Update LED effects based on current mode
  switch (config.currentMode) {
    case EFFECT_FIRE:
      fireEffect1->update();
      fireEffect2->update();
      break;
      
    case EFFECT_PULSE:
      pulseEffect1->update();
      pulseEffect2->update();
      break;
      
    case EFFECT_RAINBOW:
      rainbowEffect1->update();
      rainbowEffect2->update();
      break;
      
    case EFFECT_STROBE:
      strobeEffect1->update();
      strobeEffect2->update();
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
