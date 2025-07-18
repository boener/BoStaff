#include "hardware.h"
#include "EffectsConfig.h" // Include the config file

// Power management implementation
void PowerManager::begin() {
  // Initialize power management
  lastActiveTime = millis();
  lastBatteryCheck = millis();
  batteryVoltage = readBatteryVoltage();
  lowBatteryMode = (batteryVoltage < BATTERY_MIN_VOLTAGE);
  
  // Set up pins for power monitoring
  pinMode(BATTERY_PIN, INPUT);
  
  // Initialize power management settings from config
  originalBrightness = DEFAULT_BRIGHTNESS;
  brightnessChangeRequested = false;
  preparingForSleep = false;
  
  Serial.println(F("Power Manager initialized"));
  Serial.print(F("Battery voltage: ")); Serial.print(batteryVoltage); Serial.println(F("V"));
  Serial.print(F("Activity timeout: ")); Serial.print(ACTIVITY_TIMEOUT_MINS); Serial.println(F(" minutes"));
}

float PowerManager::readBatteryVoltage() {
  // Read battery voltage through voltage divider
  int rawValue = analogRead(BATTERY_PIN);
  float voltage = (rawValue / 1023.0) * 3.3 * BATTERY_DIVIDER;
  return voltage;
}

void PowerManager::update() {
  // Check battery voltage at controlled intervals
  if (millis() - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
    batteryVoltage = readBatteryVoltage();
    lastBatteryCheck = millis();
    
    // Check for low battery condition
    if (batteryVoltage < BATTERY_MIN_VOLTAGE && !lowBatteryMode) {
      lowBatteryMode = true;
      
      // Request low battery brightness mode instead of direct brightness change
      requestedBrightnessMode = BRIGHTNESS_LOW_BATTERY;
      brightnessChangeRequested = true;
      
      Serial.println(F("Low battery mode activated"));
    } else if (batteryVoltage > (BATTERY_MIN_VOLTAGE + 0.2) && lowBatteryMode) {
      // Restore normal operation when voltage is back up
      lowBatteryMode = false;
      
      // Request normal brightness mode
      requestedBrightnessMode = BRIGHTNESS_NORMAL;
      brightnessChangeRequested = true;
      
      Serial.println(F("Normal power mode restored"));
    }
  }
  
  // Check for inactivity timeout using the setting from EffectsConfig.h
  if (POWER_SAVING_MODE && (millis() - lastActiveTime > ACTIVITY_TIMEOUT_MINS * 60000)) {
    // Enter sleep mode to save power
    Serial.println(F("Entering sleep mode"));
    
    // Request sleep brightness mode
    requestedBrightnessMode = BRIGHTNESS_SLEEP;
    brightnessChangeRequested = true;
    
    // Allow main loop to process the brightness change first
    preparingForSleep = true;
    sleepPrepStartTime = millis();
  }
  
  // Handle sleep preparation with timing from config
  if (preparingForSleep) {
    // Give system time to process brightness change based on config
    if (millis() - sleepPrepStartTime >= FADE_TO_SLEEP_DURATION) {
      // Put ESP into deep sleep
      Serial.println(F("Going to deep sleep now"));
      ESP.deepSleep(0);
    }
  }
}

void PowerManager::resetActivityTimer() {
  lastActiveTime = millis();
}

float PowerManager::getBatteryVoltage() {
  return batteryVoltage;
}

float PowerManager::getBatteryPercentage() {
  // Calculate battery percentage based on voltage
  float percentage = (batteryVoltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100.0;
  return constrain(percentage, 0.0, 100.0);
}

bool PowerManager::isLowBattery() {
  return lowBatteryMode;
}

bool PowerManager::needsBrightnessChange() {
  return brightnessChangeRequested;
}

// Updated to use brightness modes instead of direct values
BrightnessMode PowerManager::getRequestedBrightnessMode() {
  return requestedBrightnessMode;
}

void PowerManager::clearBrightnessRequest() {
  brightnessChangeRequested = false;
}
