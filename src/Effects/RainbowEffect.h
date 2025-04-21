#ifndef RAINBOW_EFFECT_H
#define RAINBOW_EFFECT_H

#include <FastLED.h>

// Enhanced Rainbow Effect with multiple modes
// Adapted for the new single-strip design with four segments
class RainbowEffect {
private:
  CRGB* ledArray;
  int numLedsTotal;
  int segmentLength;
  uint8_t mode;        // 0=smooth cycle, 1=moving rainbow, 2=twinkle
  uint8_t hue;         // Starting hue
  uint8_t saturation;
  uint8_t speed;
  uint8_t density;     // For twinkle effect
  bool initialized;
  LEDController* controller; // Pointer to the LED controller for segment access
  
public:
  RainbowEffect(LEDController* ledController, int segmentLen = 100) : 
    ledArray(nullptr), numLedsTotal(0), segmentLength(segmentLen), 
    mode(0), hue(0), saturation(240), speed(30), density(50), 
    initialized(false), controller(ledController) {
    
    // Validate inputs
    if (!ledController) {
      Serial.println("ERROR: RainbowEffect created with invalid parameters");
      return;
    }
    
    ledArray = ledController->getLeds();
    numLedsTotal = NUM_LEDS_TOTAL; // Use the global constant
    initialized = true;
    Serial.println("RainbowEffect initialized with single-strip approach");
  }
  
  bool isInitialized() const {
    return initialized && ledArray != nullptr && controller != nullptr;
  }
  
  void setMode(uint8_t m) {
    if (m < 3) mode = m;
  }
  
  void setSaturation(uint8_t s) {
    saturation = s;
  }
  
  void setSpeed(uint8_t s) {
    speed = s;
  }
  
  void setDensity(uint8_t d) {
    density = d;
  }
  
  void update() {
    // Safety check - make sure we have valid memory and initialization
    if (!isInitialized()) {
      static bool errorLogged = false;
      if (!errorLogged) {
        Serial.println("ERROR: RainbowEffect update called on uninitialized effect");
        errorLogged = true;
      }
      return;
    }
    
    switch (mode) {
      case 0: // Smooth cycle - entire strip changes color together
        updateSmoothCycle();
        break;
        
      case 1: // Moving rainbow - colors move along the strip
        updateMovingRainbow();
        break;
        
      case 2: // Rainbow twinkle - random pixels change with rainbow hues
        updateRainbowTwinkle();
        break;
    }
    
    // Update hue slowly for next frame
    hue += (speed / 4);
  }
  
private:
  void updateSmoothCycle() {
    // Fill the entire strip with a single changing color
    CRGB color = CHSV(hue, saturation, 255);
    
    // Apply to all segments
    for (int i = 0; i < segmentLength; i++) {
      controller->getSegment1LED(i) = color;
      controller->getSegment2LED(i) = color;
      controller->getSegment3LED(i) = color;
      controller->getSegment4LED(i) = color;
    }
  }
  
  void updateMovingRainbow() {
    // For each segment, create a flowing rainbow pattern
    // Ensure the pattern flows from center (pos 0) to far end (pos 99)
    
    // Calculate appropriate hue delta to make the pattern continuous
    uint8_t hueSpread = 128; // Half the color wheel
    
    // Update all four segments with flowing rainbow patterns
    for (int i = 0; i < segmentLength; i++) {
      // Map position to hue value (0->center, 99->far end)
      uint8_t hueVal = hue + map(i, 0, segmentLength - 1, 0, hueSpread);
      
      // Apply to all four segments
      controller->getSegment1LED(i) = CHSV(hueVal, saturation, 255);
      controller->getSegment2LED(i) = CHSV(hueVal, saturation, 255);
      controller->getSegment3LED(i) = CHSV(hueVal, saturation, 255);
      controller->getSegment4LED(i) = CHSV(hueVal, saturation, 255);
    }
  }
  
  void updateRainbowTwinkle() {
    // Fade all LEDs slightly each frame
    for (int i = 0; i < segmentLength; i++) {
      controller->getSegment1LED(i).fadeToBlackBy(10);
      controller->getSegment2LED(i).fadeToBlackBy(10);
      controller->getSegment3LED(i).fadeToBlackBy(10);
      controller->getSegment4LED(i).fadeToBlackBy(10);
    }
    
    // Randomly light new LEDs across all segments
    for (int segment = 1; segment <= 4; segment++) {
      for (int i = 0; i < segmentLength; i++) {
        // Use uint8_t for division to avoid type mismatches
        uint8_t probability = density / uint8_t(10);
        if (random8() < probability) {
          // Position-dependent hue for a more organized look
          uint8_t positionHue = map(i, 0, segmentLength - 1, 0, 128);
          CRGB color = CHSV(hue + positionHue + random8(64), saturation, 255);
          
          // Apply to the correct segment
          switch (segment) {
            case 1:
              controller->getSegment1LED(i) = color;
              break;
            case 2:
              controller->getSegment2LED(i) = color;
              break;
            case 3:
              controller->getSegment3LED(i) = color;
              break;
            case 4:
              controller->getSegment4LED(i) = color;
              break;
          }
        }
      }
    }
  }
};

#endif // RAINBOW_EFFECT_H