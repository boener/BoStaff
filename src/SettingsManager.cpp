#include "BoStaff.h"
#include <EEPROM.h>
#include "../src/version.h" // Include version.h for constants

// EEPROM size for ESP8266
#define EEPROM_SIZE 512

// Magic bytes to check if settings are valid
#define SETTINGS_MAGIC_BYTE_1 0xAB
#define SETTINGS_MAGIC_BYTE_2 0xCD

// NEW DUAL-SENSOR SYSTEM VERSION - UPDATED MAGIC BYTES TO FORCE RESET
#define DUAL_SENSOR_MAGIC_BYTE_1 0xDC  // Different magic bytes for dual-sensor system
#define DUAL_SENSOR_MAGIC_BYTE_2 0xBA  // This will force settings reset on first boot

void SettingsManager::begin() {
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("Settings Manager initialized for dual-sensor system");
}

bool SettingsManager::loadSettings(Config* cfg) {
  // Read magic bytes to check if stored settings are valid
  byte magic1 = EEPROM.read(configAddress);
  byte magic2 = EEPROM.read(configAddress + 1);
  
  // Check for NEW dual-sensor system magic bytes
  if (magic1 == DUAL_SENSOR_MAGIC_BYTE_1 && magic2 == DUAL_SENSOR_MAGIC_BYTE_2) {
    // Read the stored settings for dual-sensor system
    int addr = configAddress + 2; // Skip magic bytes
    
    cfg->currentMode = EEPROM.read(addr++); 
    cfg->brightness = EEPROM.read(addr++); 
    cfg->impactBrightness = EEPROM.read(addr++);
    
    // Read impact flash duration (16-bit)
    byte lowByte = EEPROM.read(addr++);
    byte highByte = EEPROM.read(addr++);
    cfg->impactFlashDuration = lowByte | (highByte << 8);
    
    // Read impact classification counters (32-bit each)
    // Total stab impacts (4 bytes)
    cfg->totalStabImpacts = 0;
    cfg->totalStabImpacts |= ((uint32_t)EEPROM.read(addr++));
    cfg->totalStabImpacts |= ((uint32_t)EEPROM.read(addr++) << 8);
    cfg->totalStabImpacts |= ((uint32_t)EEPROM.read(addr++) << 16);
    cfg->totalStabImpacts |= ((uint32_t)EEPROM.read(addr++) << 24);
    
    // Total rotation impacts (4 bytes)
    cfg->totalRotationImpacts = 0;
    cfg->totalRotationImpacts |= ((uint32_t)EEPROM.read(addr++));
    cfg->totalRotationImpacts |= ((uint32_t)EEPROM.read(addr++) << 8);
    cfg->totalRotationImpacts |= ((uint32_t)EEPROM.read(addr++) << 16);
    cfg->totalRotationImpacts |= ((uint32_t)EEPROM.read(addr++) << 24);
    
    // Validate settings
    if (cfg->currentMode >= cfg->numModes) {
      cfg->currentMode = EFFECT_FIRE; // Default if invalid
    }
    
    if (cfg->brightness == 0) {
      cfg->brightness = DEFAULT_BRIGHTNESS; // Use config default
    }
    
    if (cfg->impactBrightness == 0) {
      cfg->impactBrightness = IMPACT_BRIGHTNESS; // Use config default
    }
    
    Serial.println("Dual-sensor settings loaded from EEPROM");
    Serial.print("  Impact counts - Stabs: "); Serial.print(cfg->totalStabImpacts);
    Serial.print(", Rotations: "); Serial.println(cfg->totalRotationImpacts);
    return true;
  } 
  else if (magic1 == SETTINGS_MAGIC_BYTE_1 && magic2 == SETTINGS_MAGIC_BYTE_2) {
    // OLD SINGLE-SENSOR SYSTEM DETECTED - FORCE MIGRATION
    Serial.println("Old single-sensor settings detected - migrating to dual-sensor system");
    
    // Read basic settings from old format but ignore old threshold
    int addr = configAddress + 2; // Skip magic bytes
    
    cfg->currentMode = EEPROM.read(addr++); 
    cfg->brightness = EEPROM.read(addr++); 
    cfg->impactBrightness = EEPROM.read(addr++);
    
    // Skip old impactThreshold (2 bytes) - no longer used
    addr += 2;
    
    // Read impact flash duration
    byte lowByte = EEPROM.read(addr++);
    byte highByte = EEPROM.read(addr++);
    cfg->impactFlashDuration = lowByte | (highByte << 8);
    
    // Initialize new dual-sensor fields with defaults
    cfg->lastImpactType = IMPACT_NONE;
    cfg->totalStabImpacts = 0;
    cfg->totalRotationImpacts = 0;
    
    // Validate migrated settings
    if (cfg->currentMode >= cfg->numModes) {
      cfg->currentMode = EFFECT_FIRE;
    }
    
    if (cfg->brightness == 0) {
      cfg->brightness = DEFAULT_BRIGHTNESS;
    }
    
    if (cfg->impactBrightness == 0) {
      cfg->impactBrightness = IMPACT_BRIGHTNESS;
    }
    
    Serial.println("Migration complete - saving new dual-sensor format");
    saveSettings(cfg); // Save in new format
    return true;
  }
  else {
    // No valid settings found, use defaults
    Serial.println("No valid settings found, using dual-sensor system defaults");
    
    // Initialize with defaults
    cfg->lastImpactType = IMPACT_NONE;
    cfg->totalStabImpacts = 0;
    cfg->totalRotationImpacts = 0;
    
    saveSettings(cfg); // Save defaults for next boot
    return false;
  }
}

void SettingsManager::saveSettings(Config* cfg) {
  // Write NEW dual-sensor magic bytes
  EEPROM.write(configAddress, DUAL_SENSOR_MAGIC_BYTE_1);
  EEPROM.write(configAddress + 1, DUAL_SENSOR_MAGIC_BYTE_2);
  
  // Write the settings for dual-sensor system
  int addr = configAddress + 2; // Skip magic bytes
  
  EEPROM.write(addr++, cfg->currentMode);
  EEPROM.write(addr++, cfg->brightness);
  EEPROM.write(addr++, cfg->impactBrightness);
  
  // Write impact flash duration (16-bit)
  EEPROM.write(addr++, cfg->impactFlashDuration & 0xFF);
  EEPROM.write(addr++, (cfg->impactFlashDuration >> 8) & 0xFF);
  
  // Write impact classification counters (32-bit each)
  // Total stab impacts (4 bytes)
  EEPROM.write(addr++, cfg->totalStabImpacts & 0xFF);
  EEPROM.write(addr++, (cfg->totalStabImpacts >> 8) & 0xFF);
  EEPROM.write(addr++, (cfg->totalStabImpacts >> 16) & 0xFF);
  EEPROM.write(addr++, (cfg->totalStabImpacts >> 24) & 0xFF);
  
  // Total rotation impacts (4 bytes)
  EEPROM.write(addr++, cfg->totalRotationImpacts & 0xFF);
  EEPROM.write(addr++, (cfg->totalRotationImpacts >> 8) & 0xFF);
  EEPROM.write(addr++, (cfg->totalRotationImpacts >> 16) & 0xFF);
  EEPROM.write(addr++, (cfg->totalRotationImpacts >> 24) & 0xFF);
  
  // Commit the changes
  EEPROM.commit();
  
  Serial.println("Dual-sensor settings saved to EEPROM");
  Serial.print("  Impact counts - Stabs: "); Serial.print(cfg->totalStabImpacts);
  Serial.print(", Rotations: "); Serial.println(cfg->totalRotationImpacts);
}