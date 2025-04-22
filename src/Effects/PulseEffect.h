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
  uint8_t waveCount;
  bool initialized;
  LEDController* controller; // Pointer to the LED controller for segment access
  
  // Configuration parameters from EffectsConfig.h
  uint8_t speed;            // Speed of pulse animation
  uint8_t pulseWidth;       // Width of pulse
  uint8_t fadeRate;         // Rate of pulse fade
  uint8_t minBrightness;    // Minimum brightness during pulse
  uint8_t maxBrightness;    // Maximum brightness during pulse
  
public:
  PulseEffect(LEDController* ledController, int segmentLen = 100);
  ~PulseEffect();
  
  bool isInitialized() const;
  void setHue(uint8_t newHue);
  void setSpeed(uint8_t newSpeed);
  void setPulseWidth(uint8_t width);
  void setWaveCount(uint8_t count);
  void update();
  
private:
  void updateSegment(int segmentIndex);
};

#endif // PULSE_EFFECT_H