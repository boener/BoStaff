#include "hardware.h"

// Power management implementation
void PowerManager::begin() {
  // Initialize power management
  lastActiveTime = millis();
  lastBatteryCheck = millis();
  batteryVoltage = readBatteryVoltage();
  lowBatteryMode = (batteryVoltage < BATTERY_MIN_VOLTAGE);
  
  // Set up pins for power monitoring
  pinMode(BATTERY_PIN, INPUT);
  
  Serial.println(F("Power Manager initialized"));
  Serial.print(F("Battery voltage: ")); Serial.print(batteryVoltage); Serial.println(F("V"));
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
      originalBrightness = FastLED.getBrightness();
      
      // Instead of directly changing brightness, set a flag
      requestedBrightness = originalBrightness / 2;
      brightnessChangeRequested = true;
      
      Serial.println(F("Low battery mode activated"));
    } else if (batteryVoltage > (BATTERY_MIN_VOLTAGE + 0.2) && lowBatteryMode) {
      // Restore normal operation when voltage is back up
      lowBatteryMode = false;
      
      // Instead of directly changing brightness, set a flag
      requestedBrightness = originalBrightness;
      brightnessChangeRequested = true;
      
      Serial.println(F("Normal power mode restored"));
    }
  }
  
  // Check for inactivity timeout
  if (POWER_SAVING_MODE && (millis() - lastActiveTime > SLEEP_AFTER_MINS * 60000)) {
    // Enter sleep mode to save power
    Serial.println(F("Entering sleep mode"));
    
    // Fixed issue: Don't use delays and direct FastLED calls 
    // Instead, request a gradual brightness reduction through LEDController
    
    // Set brightness to 0 before sleeping
    requestedBrightness = 0;
    brightnessChangeRequested = true;
    
    // Allow main loop to process the brightness change first
    // We'll set a flag and check it in the next update call
    preparingForSleep = true;
    sleepPrepStartTime = millis();
  }
  
  // Handle sleep preparation
  if (preparingForSleep) {
    // Give system time to process brightness change (1 second)
    if (millis() - sleepPrepStartTime >= 1000) {
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

uint8_t PowerManager::getRequestedBrightness() {
  return requestedBrightness;
}

void PowerManager::clearBrightnessRequest() {
  brightnessChangeRequested = false;
}