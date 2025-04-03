#ifndef PULSE_EFFECT_H
#define PULSE_EFFECT_H

#include <FastLED.h>

// Energy Pulse Effect that radiates from center outward
class PulseEffect {
private:
  CRGB* ledArray;
  int numLeds;
  uint8_t hue;
  uint8_t baseHue;
  uint8_t hueStep;
  uint8_t waveCount;
  
public:
  PulseEffect(CRGB* leds, int count) {
    ledArray = leds;
    numLeds = count;
    baseHue = 0;
    hueStep = 1;
    waveCount = 1; // Number of wave pulses
  }
  
  void setHue(uint8_t newHue) {
    baseHue = newHue;
  }
  
  void setWaveCount(uint8_t count) {
    waveCount = count;
  }
  
  void update() {
    uint8_t center = numLeds / 2;
    
    for (int i = 0; i < numLeds; i++) {
      // Distance from center (0 to center)
      uint8_t distance = abs(i - center);
      
      // Create multiple sine waves with different frequencies
      // Creates a pulse that travels outward from the center
      uint8_t brightness = 0;
      
      for (uint8_t w = 1; w <= waveCount; w++) {
        uint8_t b = beatsin8(10 * w, 0, 255 / w, 0, distance * 8);
        brightness += b;
      }
      
      // Constrain brightness to avoid overflow
      brightness = qsub8(brightness, brightness > 255 ? brightness - 255 : 0);
      
      // Calculate hue variation based on distance from center
      uint8_t hueVar = baseHue + distance;
      
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
