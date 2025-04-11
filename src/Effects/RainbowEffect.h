#ifndef RAINBOW_EFFECT_H
#define RAINBOW_EFFECT_H

#include <FastLED.h>

// Enhanced Rainbow Effect with multiple modes
// Adapted for folded LED strip arrangement where LEDs at index 0 and (count-1) are at the center/hilt,
// and LEDs at index (count/2-1) and (count/2) are at the far end
class RainbowEffect {
private:
  CRGB* ledArray;
  int numLeds;
  uint8_t mode;        // 0=smooth cycle, 1=moving rainbow, 2=twinkle
  uint8_t hue;         // Starting hue
  uint8_t saturation;
  uint8_t speed;
  uint8_t density;     // For twinkle effect
  bool isFolded;       // Whether the LED strip is folded
  
public:
  RainbowEffect(CRGB* leds, int count, bool folded = true) {
    // Validate inputs
    if (!leds || count <= 0) {
      // Handle invalid input
      ledArray = nullptr;
      numLeds = 0;
      Serial.println("ERROR: RainbowEffect created with invalid parameters");
      return;
    }
    
    ledArray = leds;
    numLeds = count;
    mode = 0;
    hue = 0;
    saturation = 240;
    speed = 30;
    density = 50; // Default density for twinkle
    isFolded = folded;
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
    // Safety check - make sure we have valid memory
    if (!ledArray || numLeds <= 0) {
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
    fill_solid(ledArray, numLeds, CHSV(hue, saturation, 255));
  }
  
  void updateMovingRainbow() {
    // Safety check again
    if (!ledArray || numLeds <= 0) return;
    
    uint8_t midPoint = numLeds / 2;
    
    if (isFolded) {
      // For folded arrangement, we want the rainbow to flow from center outward
      // or from one end to the other consistently
      
      // Calculate appropriate hue delta to make the pattern continuous
      uint8_t hueSpread = 128; // Half the color wheel
      
      // First half - from center to far end
      for (int i = 0; i < midPoint; i++) {
        uint8_t pos = i;
        uint8_t hueVal = hue + map(pos, 0, midPoint - 1, 0, hueSpread);
        if (i >= 0 && i < numLeds) { // Bounds check
          ledArray[i] = CHSV(hueVal, saturation, 255);
        }
      }
      
      // Second half - from far end back to center
      // Continue the pattern from where first half ended
      for (int i = midPoint; i < numLeds; i++) {
        uint8_t pos = i - midPoint;
        uint8_t hueVal = hue + map(pos, 0, midPoint - 1, hueSpread, 255);
        if (i >= 0 && i < numLeds) { // Bounds check
          ledArray[i] = CHSV(hueVal, saturation, 255);
        }
      }
    } else {
      // Standard moving rainbow for non-folded arrangement
      uint8_t deltaHue = 255 / numLeds; // Calculate hue change per LED
      for (int i = 0; i < numLeds; i++) {
        if (i >= 0 && i < numLeds) { // Bounds check
          ledArray[i] = CHSV(hue + (i * deltaHue), saturation, 255);
        }
      }
    }
  }
  
  void updateRainbowTwinkle() {
    // Safety check again
    if (!ledArray || numLeds <= 0) return;
    
    // Fade all LEDs slightly each frame
    for (int i = 0; i < numLeds; i++) {
      if (i >= 0 && i < numLeds) { // Bounds check
        ledArray[i].fadeToBlackBy(10);
      }
    }
    
    // Randomly light new LEDs
    for (int i = 0; i < numLeds; i++) {
      if (random8() < density / 10) { // Adjust probability based on density
        // Use a position-dependent hue for a more organized look if folded
        uint8_t positionHue;
        if (isFolded) {
          uint8_t midPoint = numLeds / 2;
          // Calculate distance from center (0 to midPoint)
          uint8_t distFromCenter;
          if (i < midPoint) {
            distFromCenter = i;
          } else {
            distFromCenter = numLeds - 1 - i;
          }
          // Map distance to hue (0-255)
          positionHue = map(distFromCenter, 0, midPoint, 0, 128);
        } else {
          positionHue = 0;
        }
        
        if (i >= 0 && i < numLeds) { // Bounds check
          ledArray[i] = CHSV(hue + positionHue + random8(64), saturation, 255);
        }
      }
    }
  }
};

#endif // RAINBOW_EFFECT_H