#ifndef STROBE_EFFECT_H
#define STROBE_EFFECT_H

#include <FastLED.h>

// Advanced Strobe Effect with multi-mode capabilities
// Adapted for the new single-strip design with four segments
class StrobeEffect {
private:
  CRGB* ledArray;
  int numLedsTotal;
  int segmentLength;
  uint8_t mode;      // 0=white strobe, 1=color strobe, 2=lightning
  uint8_t speed;     // Controls flash frequency
  uint8_t duty;      // Duty cycle (ratio of on vs off time)
  CRGB color;        // Color for colored strobe mode
  uint16_t count;    // Counter for effect timing
  uint8_t chance;    // Lightning strike chance (0-255)
  bool active;       // Current state of the strobe
  uint8_t flashMaxBrightness; // Maximum brightness for flash
  bool initialized;  // Flag to track initialization status
  LEDController* controller; // Pointer to the LED controller for segment access
  
public:
  StrobeEffect(LEDController* ledController, int segmentLen = 100) : 
    ledArray(nullptr), numLedsTotal(0), segmentLength(segmentLen),
    mode(0), speed(50), duty(10), color(CRGB::White), count(0), 
    chance(5), active(false), flashMaxBrightness(25), 
    initialized(false), controller(ledController) {
    
    // Validate inputs
    if (!ledController) {
      Serial.println("ERROR: StrobeEffect created with invalid parameters");
      return;
    }
    
    ledArray = ledController->getLeds();
    numLedsTotal = NUM_LEDS_TOTAL; // Use the global constant
    initialized = true;
    Serial.println("StrobeEffect initialized with single-strip approach");
  }
  
  bool isInitialized() const {
    return initialized && ledArray != nullptr && controller != nullptr;
  }
  
  void setMode(uint8_t m) {
    if (m < 3) mode = m;
  }
  
  void setSpeed(uint8_t s) {
    speed = s;
  }
  
  void setDuty(uint8_t d) {
    // Use explicit cast for comparison to avoid type mismatch
    if (d < uint8_t(1)) d = 1;
    if (d > uint8_t(99)) d = 99;
    duty = d;
  }
  
  void setColor(CRGB c) {
    color = c;
  }
  
  void setFlashBrightness(uint8_t brightness) {
    flashMaxBrightness = brightness;
  }
  
  void update() {
    // Safety check - make sure we have valid memory and initialization
    if (!isInitialized()) {
      static bool errorLogged = false;
      if (!errorLogged) {
        Serial.println("ERROR: StrobeEffect update called on uninitialized effect");
        errorLogged = true;
      }
      return;
    }
    
    // Variables declared outside case statements to avoid C++ scope errors
    CRGB flashColor;
    CRGB scaledColor;
    
    // Different behavior based on mode
    switch (mode) {
      case 0: // Classic white strobe
        flashColor = CRGB(flashMaxBrightness, flashMaxBrightness, flashMaxBrightness); // Dimmed white
        updateClassicStrobe(flashColor);
        break;
        
      case 1: // Color strobe
        // Scale color to maximum brightness
        scaledColor = color;
        scaledColor.nscale8(flashMaxBrightness);
        updateClassicStrobe(scaledColor);
        break;
        
      case 2: // Lightning effect
        updateLightning();
        break;
    }
    
    count++;
  }
  
private:
  void updateClassicStrobe(CRGB flashColor) {
    // Calculate period based on speed
    uint16_t period = 255 - speed; // Higher speed = shorter period
    
    // Use explicit cast for comparison to avoid type mismatch
    if (period < uint16_t(10)) period = 10;  // Prevent ultra-fast flashing
    
    // Calculate timing
    uint16_t onTime = (period * duty) / 100;
    
    // Determine on/off state
    bool isOn = (count % period) < onTime;
    
    // Apply to all segments
    for (int i = 0; i < segmentLength; i++) {
      CRGB pixelColor = isOn ? flashColor : CRGB::Black;
      
      // Update all four segments
      controller->getSegment1LED(i) = pixelColor;
      controller->getSegment2LED(i) = pixelColor;
      controller->getSegment3LED(i) = pixelColor;
      controller->getSegment4LED(i) = pixelColor;
    }
  }
  
  void updateLightning() {
    // Clear all LEDs first
    for (int i = 0; i < segmentLength; i++) {
      controller->getSegment1LED(i) = CRGB::Black;
      controller->getSegment2LED(i) = CRGB::Black;
      controller->getSegment3LED(i) = CRGB::Black;
      controller->getSegment4LED(i) = CRGB::Black;
    }
    
    // Reduced brightness white for lightning
    CRGB lightningColor = CRGB(flashMaxBrightness, flashMaxBrightness, flashMaxBrightness);
    
    // Randomly decide if we should create a lightning flash
    if (random8() < chance) {
      // Determine what type of lightning strike to create
      uint8_t strikeType = random8(3); // 0, 1, or 2
      
      switch (strikeType) {
        case 0: // Strike near center (hilt)
          {
            uint8_t strikeLength = random8(segmentLength / 3);
            for (int i = 0; i < strikeLength; i++) {
              controller->getSegment1LED(i) = lightningColor;
              controller->getSegment2LED(i) = lightningColor;
              controller->getSegment3LED(i) = lightningColor;
              controller->getSegment4LED(i) = lightningColor;
            }
          }
          break;
          
        case 1: // Strike near far end (tip)
          {
            uint8_t strikeLength = random8(segmentLength / 3);
            for (int i = 0; i < strikeLength; i++) {
              int pos = segmentLength - 1 - i;
              controller->getSegment1LED(pos) = lightningColor;
              controller->getSegment2LED(pos) = lightningColor;
              controller->getSegment3LED(pos) = lightningColor;
              controller->getSegment4LED(pos) = lightningColor;
            }
          }
          break;
          
        case 2: // Full staff lightning (but with reduced LEDs to save power)
          for (int i = 0; i < segmentLength; i += 3) { // Only light every 3rd LED
            controller->getSegment1LED(i) = lightningColor;
            controller->getSegment2LED(i) = lightningColor;
            controller->getSegment3LED(i) = lightningColor;
            controller->getSegment4LED(i) = lightningColor;
          }
          break;
      }
      
      // Schedule afterglow
      active = true;
    } else if (active) {
      // Decay the lightning effect with afterglow (reduced brightness)
      uint8_t fade = random8(1, 3);
      
      for (int segment = 1; segment <= 4; segment++) {
        for (int i = 0; i < segmentLength; i++) {
          if (random8() < 80) { // 31% chance
            CRGB afterglowColor = CRGB(fade, fade, fade + random8(1, 2)); // Blue tint
            
            // Apply to the correct segment
            switch (segment) {
              case 1:
                controller->getSegment1LED(i) = afterglowColor;
                break;
              case 2:
                controller->getSegment2LED(i) = afterglowColor;
                break;
              case 3:
                controller->getSegment3LED(i) = afterglowColor;
                break;
              case 4:
                controller->getSegment4LED(i) = afterglowColor;
                break;
            }
          }
        }
      }
      
      // End the lightning
      active = false;
    }
  }
};

#endif // STROBE_EFFECT_H