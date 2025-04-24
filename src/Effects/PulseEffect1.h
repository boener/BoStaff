#ifndef PULSE_EFFECT1_H
#define PULSE_EFFECT1_H

#include <FastLED.h>

// Forward declaration (already in effects.h, but included here for safety)
class LEDController;

// PulseEffect1 - A variation of the Energy Pulse Effect with different colors/speeds
// Creates pulses radiating from center outward
class PulseEffect1 {
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
  PulseEffect1(LEDController* ledController, int segmentLen = 100);
  ~PulseEffect1();
  
  bool isInitialized() const;
  void setHue(uint8_t newHue);
  void setSpeed(uint8_t newSpeed);
  void setPulseWidth(uint8_t width);
  void setWaveCount(uint8_t count);
  void update();
  
private:
  void updateSegment(int segmentIndex);
};

#endif // PULSE_EFFECT1_H