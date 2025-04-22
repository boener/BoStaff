#ifndef PULSE_EFFECT_H
#define PULSE_EFFECT_H

#include <FastLED.h>

// Forward declaration (already in effects.h, but included here for safety)
class LEDController;

// Energy Pulse Effect that radiates from center outward
// Adapted for the new single-strip design with four segments
class PulseEffect {
private:
  CRGB* ledArray;
  int numLedsTotal;
  int segmentLength;
  uint8_t baseHue;
  uint8_t hueStep;
  uint8_t waveCount;
  bool initialized;
  LEDController* controller; // Pointer to the LED controller for segment access
  
public:
  PulseEffect(LEDController* ledController, int segmentLen = 100);
  ~PulseEffect();
  
  bool isInitialized() const;
  void setHue(uint8_t newHue);
  void setWaveCount(uint8_t count);
  void update();
  
private:
  void updateSegment(int segmentIndex);
};

#endif // PULSE_EFFECT_H