#ifndef FIRE_EFFECT_H
#define FIRE_EFFECT_H

#include <FastLED.h>

// Forward declaration (already in effects.h, but included here for safety)
class LEDController;

// Advanced Fire Effect with more realistic appearance
// Adapted for the new single-strip design with four segments
class FireEffect {
private:
  CRGB* ledArray;        // Pointer to the LED array
  int numLedsTotal;      // Total number of LEDs
  int segmentLength;     // Length of each segment (100 LEDs)
  byte* heat;            // Heat array for fire simulation
  uint8_t cooling;       // Fire cooling parameter
  uint8_t sparking;      // Fire sparking parameter
  bool initialized;      // Flag to track initialization status
  LEDController* controller; // Pointer to the LED controller for segment access
  
public:
  FireEffect(LEDController* ledController, int segmentLen = 100);
  ~FireEffect();
  
  bool isInitialized() const;
  void setCooling(uint8_t cool);
  void setSparking(uint8_t spark);
  void update();
  
private:
  void updateSegment(int segmentIndex);
};

#endif // FIRE_EFFECT_H