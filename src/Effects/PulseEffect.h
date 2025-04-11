#ifndef PULSE_EFFECT_H
#define PULSE_EFFECT_H

#include <FastLED.h>

// Energy Pulse Effect that radiates from center outward
// Accounts for folded LED arrangement where LED 1 and 200 are at the center/hilt,
// and LEDs 100 and 101 are at the far end
class PulseEffect {
private:
  CRGB* ledArray;
  int numLeds;
  uint8_t hue;
  uint8_t baseHue;
  uint8_t hueStep;
  uint8_t waveCount;
  bool isFolded; // Whether the LED strip is folded
  
public:
  PulseEffect(CRGB* leds, int count, bool folded = true) {
    // Validate inputs
    if (!leds || count <= 0) {
      // Handle invalid input
      ledArray = nullptr;
      numLeds = 0;
      Serial.println("ERROR: PulseEffect created with invalid parameters");
      return;
    }
    
    ledArray = leds;
    numLeds = count;
    baseHue = 0;
    hueStep = 1;
    waveCount = 1; // Number of wave pulses
    isFolded = folded;
  }
  
  void setHue(uint8_t newHue) {
    baseHue = newHue;
  }
  
  void setWaveCount(uint8_t count) {
    if (count > 0 && count <= 5) { // Reasonable bounds
      waveCount = count;
    }
  }
  
  void update() {
    // Safety check - make sure we have valid memory
    if (!ledArray || numLeds <= 0) {
      return;
    }
    
    // For folded strips, both ends (LED 0 and LED count-1) are at the center
    // and the middle of the strip (LED count/2) is at the far end
    
    uint8_t midPoint = numLeds / 2; // Physical midpoint of the strip
    
    for (int i = 0; i < numLeds; i++) {
      // Bounds check
      if (i < 0 || i >= numLeds) continue;
      
      // Calculate distance from center based on folded arrangement
      uint8_t distanceFromCenter;
      
      if (isFolded) {
        // In folded arrangement:
        // - LEDs 0 to midPoint-1 go from center to tip
        // - LEDs midPoint to numLeds-1 go from tip back to center
        if (i < midPoint) {
          distanceFromCenter = i; // First half: 0 is at center
        } else {
          distanceFromCenter = numLeds - 1 - i; // Second half: numLeds-1 is at center
        }
      } else {
        // For standard linear arrangement
        distanceFromCenter = abs(i - midPoint);
      }
      
      // Create multiple sine waves with different frequencies
      // Creates a pulse that travels outward from the center
      uint16_t brightness = 0; // Use uint16_t to prevent overflow during addition
      
      for (uint8_t w = 1; w <= waveCount; w++) {
        uint8_t b = beatsin8(10 * w, 0, 255 / max(w, 1), 0, distanceFromCenter * 8);
        brightness += b;
      }
      
      // Constrain brightness to avoid overflow
      brightness = min(brightness, 255);
      
      // Calculate hue variation based on distance from center
      uint8_t hueVar = baseHue + distanceFromCenter;
      
      // Set the LED color
      ledArray[i] = CHSV(hueVar, 255, brightness);
    }
    
    // Slowly change the base hue for variation
    EVERY_N_MILLISECONDS(50) {
      baseHue += hueStep;
    }
  }
};

#endif // PULSE_EFFECT_H