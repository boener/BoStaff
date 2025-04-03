#ifndef FIRE_EFFECT_H
#define FIRE_EFFECT_H

#include <FastLED.h>

// Advanced Fire Effect with more realistic appearance
class FireEffect {
private:
  CRGB* ledArray;
  int numLeds;
  byte* heat;
  uint8_t cooling;
  uint8_t sparking;
  bool reversed;
  
public:
  FireEffect(CRGB* leds, int count, bool reverse = false) {
    ledArray = leds;
    numLeds = count;
    reversed = reverse;
    cooling = 55;   // Default cooling value
    sparking = 120; // Default sparking value
    
    // Allocate the heat array
    heat = new byte[numLeds];
    memset(heat, 0, numLeds);
  }
  
  ~FireEffect() {
    delete[] heat;
  }
  
  void setCooling(uint8_t cool) {
    cooling = cool;
  }
  
  void setSparking(uint8_t spark) {
    sparking = spark;
  }
  
  void update() {
    // Step 1: Cool down every cell a little
    for (int i = 0; i < numLeds; i++) {
      heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / numLeds) + 2));
    }
  
    // Step 2: Heat from each cell drifts up and diffuses
    for (int k = numLeds - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }
    
    // Step 3: Randomly ignite new sparks near the bottom
    if (random8() < sparking) {
      int y = random8(7);
      heat[y] = qadd8(heat[y], random8(160, 255));
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
      
      ledArray[pixelnumber] = color;
    }
  }
};

#endif // FIRE_EFFECT_H
