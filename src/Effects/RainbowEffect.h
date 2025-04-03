#ifndef RAINBOW_EFFECT_H
#define RAINBOW_EFFECT_H

#include <FastLED.h>

// Enhanced Rainbow Effect with multiple modes
class RainbowEffect {
private:
  CRGB* ledArray;
  int numLeds;
  uint8_t mode;        // 0=smooth cycle, 1=moving rainbow, 2=twinkle
  uint8_t hue;         // Starting hue
  uint8_t saturation;
  uint8_t speed;
  uint8_t density;     // For twinkle effect
  
public:
  RainbowEffect(CRGB* leds, int count) {
    ledArray = leds;
    numLeds = count;
    mode = 0;
    hue = 0;
    saturation = 240;
    speed = 30;
    density = 50; // Default density for twinkle
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
    // Create a moving rainbow pattern along the strip
    uint8_t deltaHue = 255 / (numLeds / 2); // Calculate hue change per LED
    
    for (int i = 0; i < numLeds; i++) {
      // Calculate hue based on position and current base hue
      ledArray[i] = CHSV(hue + (i * deltaHue), saturation, 255);
    }
  }
  
  void updateRainbowTwinkle() {
    // Fade all LEDs slightly each frame
    for (int i = 0; i < numLeds; i++) {
      ledArray[i].fadeToBlackBy(10);
    }
    
    // Randomly light new LEDs
    for (int i = 0; i < numLeds; i++) {
      if (random8() < density / 10) { // Adjust probability based on density
        ledArray[i] = CHSV(hue + random8(64), saturation, 255);
      }
    }
  }
};

#endif // RAINBOW_EFFECT_H