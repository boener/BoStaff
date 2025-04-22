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
  uint8_t density;     // For twinkle effect
  bool initialized;
  LEDController* controller; // Pointer to the LED controller for segment access
  
  // Configuration parameters from EffectsConfig.h
  uint8_t speed;        // Speed of rainbow animation
  uint8_t deltaHue;     // Hue change per LED (rainbow compression)
  uint8_t saturation;   // Color saturation
  uint8_t brightness;   // Color brightness/value
  
public:
  RainbowEffect(LEDController* ledController, int segmentLen = 100);
  ~RainbowEffect();
  
  bool isInitialized() const;
  void setMode(uint8_t m);
  void setSaturation(uint8_t s);
  void setSpeed(uint8_t s);
  void setDeltaHue(uint8_t delta);
  void setDensity(uint8_t d);
  void update();
  
private:
  void updateSmoothCycle();
  void updateMovingRainbow();
  void updateRainbowTwinkle();
};

#endif // RAINBOW_EFFECT_H