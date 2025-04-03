#include "BoStaff.h"
#include <EEPROM.h>

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
    cfg->impactThreshold = EEPROM.read(addr++) | (EEPROM.read(addr++) << 8); 
    cfg->impactFlashDuration = EEPROM.read(addr++) | (EEPROM.read(addr++) << 8); 
    
    // Validate settings
    if (cfg->currentMode >= cfg->numModes) {
      cfg->currentMode = EFFECT_FIRE; // Default if invalid
    }
    
    if (cfg->brightness == 0) {
      cfg->brightness = 150; // Default if invalid
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
  EEPROM.write(addr++, cfg->impactThreshold & 0xFF); 
  EEPROM.write(addr++, (cfg->impactThreshold >> 8) & 0xFF);
  EEPROM.write(addr++, cfg->impactFlashDuration & 0xFF);
  EEPROM.write(addr++, (cfg->impactFlashDuration >> 8) & 0xFF);
  
  // Commit the changes
  EEPROM.commit();
  
  Serial.println("Settings saved to EEPROM");
}
