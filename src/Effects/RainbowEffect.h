#ifndef RAINBOW_EFFECT_H
#define RAINBOW_EFFECT_H

#include <FastLED.h>

// Forward declaration (already in effects.h, but included here for safety)
class LEDController;

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
  RainbowEffect(LEDController* ledController, int segmentLen = 100);
  ~RainbowEffect();
  
  bool isInitialized() const;
  void setMode(uint8_t m);
  void setSaturation(uint8_t s);
  void setSpeed(uint8_t s);
  void setDensity(uint8_t d);
  void update();
  
private:
  void updateSmoothCycle();
  void updateMovingRainbow();
  void updateRainbowTwinkle();
};

#endif // RAINBOW_EFFECT_H