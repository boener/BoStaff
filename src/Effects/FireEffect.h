#ifndef FIRE_EFFECT_H
#define FIRE_EFFECT_H

#include <FastLED.h>

// Advanced Fire Effect with more realistic appearance
// Adapted for folded LED strip arrangement where LEDs at index 0 and (count-1) are at the center/hilt,
// and LEDs at index (count/2-1) and (count/2) are at the far end
class FireEffect {
private:
  CRGB* ledArray;
  int numLeds;
  byte* heat;
  uint8_t cooling;
  uint8_t sparking;
  bool reversed;
  bool isFolded; // Whether the LED strip is folded
  
public:
  FireEffect(CRGB* leds, int count, bool reverse = false, bool folded = true) {
    // Validate inputs
    if (!leds || count <= 0) {
      // Handle invalid input
      ledArray = nullptr;
      numLeds = 0;
      heat = nullptr;
      Serial.println("ERROR: FireEffect created with invalid parameters");
      return;
    }
    
    ledArray = leds;
    numLeds = count;
    reversed = reverse;
    isFolded = folded;
    cooling = 85;   // Increased from 55 to create faster cooling
    sparking = 90;  // Reduced from 120 to control spark generation
    
    // Allocate the heat array
    heat = new byte[numLeds];
    if (heat) {
      // Initialize all elements to zero
      memset(heat, 0, numLeds);
    } else {
      Serial.println("ERROR: FireEffect failed to allocate heat array");
      numLeds = 0; // Mark as invalid
    }
  }
  
  ~FireEffect() {
    if (heat) {
      delete[] heat;
      heat = nullptr;  // Prevent double deletion
    }
  }
  
  void setCooling(uint8_t cool) {
    cooling = cool;
  }
  
  void setSparking(uint8_t spark) {
    sparking = spark;
  }
  
  void update() {
    // Safety check - make sure we have valid memory
    if (!ledArray || !heat || numLeds <= 0) {
      return;
    }
    
    // For a folded strip, we need to treat the 'middle' LED indexes as the physical far end
    // and the 0 and (count-1) as the physical center/hilt
    uint8_t midPoint = numLeds / 2;
    
    // Step 1: Cool down every cell a little
    for (int i = 0; i < numLeds; i++) {
      heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / numLeds) + 2));
    }
  
    // Step 2: Heat from each cell drifts 'up' and diffuses
    // For folded arrangement, heat rises from both ends toward the middle
    if (isFolded) {
      // First half - heat rises from center (0) toward far end (midPoint-1)
      for (int k = midPoint - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
      }
      
      // Second half - heat rises from center (numLeds-1) toward far end (midPoint)
      for (int k = midPoint; k < numLeds - 2; k++) {
        heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2]) / 3;
      }
    } else {
      // Standard upward drift for non-folded arrangement
      for (int k = numLeds - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
      }
    }
    
    // Step 3: Randomly ignite new sparks at the bottom/center
    if (isFolded) {
      // Sparks can start near both ends (center of the physical staff)
      if (random8() < sparking) {
        int y = random8(7); // Near index 0 (center)
        if (y < numLeds) { // Bounds check
          heat[y] = qadd8(heat[y], random8(160, 255));
        }
      }
      
      if (random8() < sparking) {
        int y = numLeds - 1 - random8(7); // Near index numLeds-1 (center)
        if (y >= 0 && y < numLeds) { // Bounds check
          heat[y] = qadd8(heat[y], random8(160, 255));
        }
      }
    } else {
      // Standard sparking at the bottom for non-folded arrangement
      if (random8() < sparking) {
        int y = random8(7);
        if (y < numLeds) { // Bounds check
          heat[y] = qadd8(heat[y], random8(160, 255));
        }
      }
    }
  
    // Step 4: Map from heat cells to LED colors
    for (int j = 0; j < numLeds; j++) {
      CRGB color = HeatColor(heat[j]);
      int pixelnumber;
      
      if (reversed) {
        pixelnumber = (numLeds - 1) - j;
      } else {
        pixelnumber = j;
      }
      
      // Bounds check
      if (pixelnumber >= 0 && pixelnumber < numLeds) {
        ledArray[pixelnumber] = color;
      }
    }
  }
};

#endif // FIRE_EFFECT_H