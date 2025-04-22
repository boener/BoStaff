#ifndef STROBE_EFFECT_H
#define STROBE_EFFECT_H

#include <FastLED.h>

// Forward declaration (already in effects.h, but included here for safety)
class LEDController;

// Advanced Strobe Effect with multi-mode capabilities
// Adapted for the new single-strip design with four segments
class StrobeEffect {
private:
  CRGB* ledArray;
  int numLedsTotal;
  int segmentLength;
  uint8_t mode;      // 0=white strobe, 1=color strobe, 2=lightning
  uint8_t speed;     // Controls flash frequency
  uint8_t duty;      // Duty cycle (ratio of on vs off time)
  CRGB color;        // Color for colored strobe mode
  uint16_t count;    // Counter for effect timing
  uint8_t chance;    // Lightning strike chance (0-255)
  bool active;       // Current state of the strobe
  uint8_t flashMaxBrightness; // Maximum brightness for flash
  bool initialized;  // Flag to track initialization status
  LEDController* controller; // Pointer to the LED controller for segment access
  
  // Configuration parameters from EffectsConfig.h
  uint16_t onTime;    // Time in ms that the strobe is on
  uint16_t offTime;   // Time in ms that the strobe is off
  bool fadeOut;       // Whether strobe fades out or cuts off
  uint8_t fadeRate;   // How quickly strobe fades
  
public:
  StrobeEffect(LEDController* ledController, int segmentLen = 100);
  ~StrobeEffect();
  
  bool isInitialized() const;
  void setMode(uint8_t m);
  void setSpeed(uint8_t s);
  void setDuty(uint8_t d);
  void setColor(CRGB c);
  void setFlashBrightness(uint8_t brightness);
  void update();
  
private:
  void updateClassicStrobe(CRGB flashColor);
  void updateLightning();
};

#endif // STROBE_EFFECT_H