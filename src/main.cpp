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

// Variables for calibration trigger
const unsigned long CALIBRATION_LONG_PRESS = 5000; // 5 seconds for calibration trigger
bool calibrationMode = false;
unsigned long buttonPressStart = 0;
bool buttonWasPressed = false;

// I2C and LED timing control variables
unsigned long lastAccelUpdate = 0;
const unsigned long ACCEL_UPDATE_INTERVAL = 25; // Only read accelerometer every 25ms to reduce I2C traffic

// Main loop timing control
unsigned long lastLoopTime = 0;
const unsigned long LOOP_INTERVAL = 5; // 5ms loop interval for consistent timing

// Function to initialize all effect objects
void initializeAllEffects() {
  // Clear any existing effects first
  if (fireEffect1) delete fireEffect1;
  if (fireEffect2) delete fireEffect2;
  if (pulseEffect1) delete pulseEffect1;
  if (pulseEffect2) delete pulseEffect2;
  if (rainbowEffect1) delete rainbowEffect1;
  if (rainbowEffect2) delete rainbowEffect2;
  if (strobeEffect1) delete strobeEffect1;
  if (strobeEffect2) delete strobeEffect2;
  
  // Set all pointers to null (important to prevent dangling pointers)
  fireEffect1 = nullptr;
  fireEffect2 = nullptr;
  pulseEffect1 = nullptr;
  pulseEffect2 = nullptr;
  rainbowEffect1 = nullptr;
  rainbowEffect2 = nullptr;
  strobeEffect1 = nullptr;
  strobeEffect2 = nullptr;
  
  // Now create all effects fresh
  bool allEffectsInitialized = true;
  
  // Try to create FireEffect instances
  fireEffect1 = new FireEffect(ledController.getLeds1(), NUM_LEDS_PER_STRIP, false, true); // not reversed, folded
  fireEffect2 = new FireEffect(ledController.getLeds2(), NUM_LEDS_PER_STRIP, true, true); // reversed, folded
  
  if (!fireEffect1 || !fireEffect1->isInitialized() || !fireEffect2 || !fireEffect2->isInitialized()) {
    Serial.println(F("Error initializing FireEffect!"));
    allEffectsInitialized = false;
  }
  
  // Try to create PulseEffect instances
  pulseEffect1 = new PulseEffect(ledController.getLeds1(), NUM_LEDS_PER_STRIP, true); // folded
  pulseEffect2 = new PulseEffect(ledController.getLeds2(), NUM_LEDS_PER_STRIP, true); // folded
  
  if (!pulseEffect1 || !pulseEffect2) {
    Serial.println(F("Error initializing PulseEffect!"));
    allEffectsInitialized = false;
  }
  
  // Try to create RainbowEffect instances
  rainbowEffect1 = new RainbowEffect(ledController.getLeds1(), NUM_LEDS_PER_STRIP, true); // folded
  rainbowEffect2 = new RainbowEffect(ledController.getLeds2(), NUM_LEDS_PER_STRIP, true); // folded
  
  if (!rainbowEffect1 || !rainbowEffect2) {
    Serial.println(F("Error initializing RainbowEffect!"));
    allEffectsInitialized = false;
  }
  
  // Try to create StrobeEffect instances
  strobeEffect1 = new StrobeEffect(ledController.getLeds1(), NUM_LEDS_PER_STRIP, true); // folded
  strobeEffect2 = new StrobeEffect(ledController.getLeds2(), NUM_LEDS_PER_STRIP, true); // folded
  
  if (!strobeEffect1 || !strobeEffect2) {
    Serial.println(F("Error initializing StrobeEffect!"));
    allEffectsInitialized = false;
  }
  
  // Clear the LED arrays to ensure clean start
  fill_solid(ledController.getLeds1(), NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(ledController.getLeds2(), NUM_LEDS_PER_STRIP, CRGB::Black);
  FastLED.show();
  
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
  Serial.print(F("LED Strip 1: ")); Serial.print(F("D3 (GPIO0)")); Serial.println(F(" - was D1 (GPIO5)"));
  Serial.print(F("LED Strip 2: ")); Serial.print(F("D4 (GPIO2)")); Serial.println(F(" - was D2 (GPIO4)"));
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
  
  // Initialize effect objects with correct folded arrangement
  // true = folded arrangement (LEDs 0 and 199 at center, 99 and 100 at far end)
  // false = linear arrangement
  Serial.println(F("Initializing LED effects for folded strip arrangement"));
  Serial.println(F("(LEDs 0 and 199 at center/hilt, LEDs 99 and 100 at far end)"));
  
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
  
  // Initialize timing variables
  lastAccelUpdate = millis();
  lastLoopTime = millis();
}

void loop() {
  // Add timing control to main loop for consistent performance
  unsigned long currentMillis = millis();
  if (currentMillis - lastLoopTime < LOOP_INTERVAL) {
    // Not enough time has passed, yield to other processes
    yield();
    return;
  }
  lastLoopTime = currentMillis;
  
  // Check for calibration mode trigger (long button press)
  if (digitalRead(BTN_PIN) == LOW) {  // Button pressed (active LOW)
    if (!buttonWasPressed) {
      buttonWasPressed = true;
      buttonPressStart = millis();
    } else if (!calibrationMode && (millis() - buttonPressStart) > CALIBRATION_LONG_PRESS) {
      // Long press detected, enter calibration mode
      calibrationMode = true;
      
      // Disable all interrupts to ensure clean LED operations
      noInterrupts();
      
      // Clear both strips completely before visual feedback
      fill_solid(ledController.getLeds1(), NUM_LEDS_PER_STRIP, CRGB::Black);
      fill_solid(ledController.getLeds2(), NUM_LEDS_PER_STRIP, CRGB::Black);
      FastLED.show();
      delay(100);
      
      // Visual feedback - flash LEDs blue to indicate calibration mode
      for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
        ledController.getLeds1()[i] = CRGB::Blue;
        ledController.getLeds2()[i] = CRGB::Blue;
      }
      FastLED.show();
      delay(500);
      
      // Clear both strips completely 
      fill_solid(ledController.getLeds1(), NUM_LEDS_PER_STRIP, CRGB::Black);
      fill_solid(ledController.getLeds2(), NUM_LEDS_PER_STRIP, CRGB::Black);
      FastLED.show();
      delay(500);
      
      // Re-enable interrupts
      interrupts();
      
      Serial.println(F("\n*** ENTERING CALIBRATION MODE ***"));
      
      // Start the calibration process
      accelHandler.calibrate();
      
      // Save the new threshold value
      settingsManager.saveSettings(&config);
      
      Serial.print(F("New impact threshold saved: "));
      Serial.println(config.impactThreshold);
      
      // Disable interrupts again
      noInterrupts();
      
      // Visual feedback - flash LEDs green to indicate calibration complete
      for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
        ledController.getLeds1()[i] = CRGB::Green;
        ledController.getLeds2()[i] = CRGB::Green;
      }
      FastLED.show();
      delay(1000);
      
      // Clear both strips completely before restoring normal operation
      fill_solid(ledController.getLeds1(), NUM_LEDS_PER_STRIP, CRGB::Black);  
      fill_solid(ledController.getLeds2(), NUM_LEDS_PER_STRIP, CRGB::Black);
      FastLED.show();
      
      // Re-enable interrupts
      interrupts();
      
      // Completely reinitialize all effect objects to ensure clean state
      initializeAllEffects();
      
      // Reset calibration mode
      calibrationMode = false;
      
      // Make sure brightness is restored
      FastLED.setBrightness(config.brightness);
      
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
  
  // Update LED effects based on current mode
  switch (config.currentMode) {
    case EFFECT_FIRE:
      if (fireEffect1 && fireEffect2 && fireEffect1->isInitialized() && fireEffect2->isInitialized()) {
        fireEffect1->update();
        fireEffect2->update();
      } else {
        // Fallback to a simple effect if fire effect is not available
        fill_solid(ledController.getLeds1(), NUM_LEDS_PER_STRIP, CRGB::Red);
        fill_solid(ledController.getLeds2(), NUM_LEDS_PER_STRIP, CRGB::Red);
      }
      break;
      
    case EFFECT_PULSE:
      if (pulseEffect1 && pulseEffect2) {
        pulseEffect1->update();
        pulseEffect2->update();
      } else {
        // Fallback effect
        fill_solid(ledController.getLeds1(), NUM_LEDS_PER_STRIP, CRGB::Blue);
        fill_solid(ledController.getLeds2(), NUM_LEDS_PER_STRIP, CRGB::Blue);
      }
      break;
      
    case EFFECT_RAINBOW:
      if (rainbowEffect1 && rainbowEffect2) {
        rainbowEffect1->update();
        rainbowEffect2->update();
      } else {
        // Fallback effect
        fill_solid(ledController.getLeds1(), NUM_LEDS_PER_STRIP, CRGB::Green);
        fill_solid(ledController.getLeds2(), NUM_LEDS_PER_STRIP, CRGB::Green);
      }
      break;
      
    case EFFECT_STROBE:
      if (strobeEffect1 && strobeEffect2) {
        strobeEffect1->update();
        strobeEffect2->update();
      } else {
        // Fallback effect
        fill_solid(ledController.getLeds1(), NUM_LEDS_PER_STRIP, CRGB::White);
        fill_solid(ledController.getLeds2(), NUM_LEDS_PER_STRIP, CRGB::White);
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