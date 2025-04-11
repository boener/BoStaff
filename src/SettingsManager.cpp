#include "BoStaff.h"
#include <EEPROM.h>
#include "../src/version.h" // Include version.h for constants

// EEPROM size for ESP8266
#define EEPROM_SIZE 512

// Magic bytes to check if settings are valid
#define SETTINGS_MAGIC_BYTE_1 0xAB
#define SETTINGS_MAGIC_BYTE_2 0xCD

void SettingsManager::begin() {
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("Settings Manager initialized");
}

bool SettingsManager::loadSettings(Config* cfg) {
  // Read magic bytes to check if stored settings are valid
  byte magic1 = EEPROM.read(configAddress);
  byte magic2 = EEPROM.read(configAddress + 1);
  
  if (magic1 == SETTINGS_MAGIC_BYTE_1 && magic2 == SETTINGS_MAGIC_BYTE_2) {
    // Read the stored settings
    int addr = configAddress + 2; // Skip magic bytes
    
    cfg->currentMode = EEPROM.read(addr++); 
    cfg->brightness = EEPROM.read(addr++); 
    cfg->impactBrightness = EEPROM.read(addr++); // Added impact brightness
    
    // Read 16-bit values
    byte lowByte = EEPROM.read(addr++);
    byte highByte = EEPROM.read(addr++);
    cfg->impactThreshold = lowByte | (highByte << 8);
    
    lowByte = EEPROM.read(addr++);
    highByte = EEPROM.read(addr++);
    cfg->impactFlashDuration = lowByte | (highByte << 8);
    
    // Validate settings
    if (cfg->currentMode >= cfg->numModes) {
      cfg->currentMode = EFFECT_FIRE; // Default if invalid
    }
    
    if (cfg->brightness == 0) {
      cfg->brightness = 25; // Use updated default if invalid (reduced from 75 to 25)
    }
    
    if (cfg->impactBrightness == 0) {
      cfg->impactBrightness = 100; // Use hardcoded default if invalid
    }
    
    Serial.println("Settings loaded from EEPROM");
    return true;
  } else {
    // No valid settings found, use defaults
    Serial.println("No valid settings found, using defaults");
    saveSettings(cfg); // Save defaults for next boot
    return false;
  }
}

void SettingsManager::saveSettings(Config* cfg) {
  // Write magic bytes
  EEPROM.write(configAddress, SETTINGS_MAGIC_BYTE_1);
  EEPROM.write(configAddress + 1, SETTINGS_MAGIC_BYTE_2);
  
  // Write the settings
  int addr = configAddress + 2; // Skip magic bytes
  
  EEPROM.write(addr++, cfg->currentMode);
  EEPROM.write(addr++, cfg->brightness);
  EEPROM.write(addr++, cfg->impactBrightness); // Added impact brightness
  EEPROM.write(addr++, cfg->impactThreshold & 0xFF); 
  EEPROM.write(addr++, (cfg->impactThreshold >> 8) & 0xFF);
  EEPROM.write(addr++, cfg->impactFlashDuration & 0xFF);
  EEPROM.write(addr++, (cfg->impactFlashDuration >> 8) & 0xFF);
  
  // Commit the changes
  EEPROM.commit();
  
  Serial.println("Settings saved to EEPROM");
}