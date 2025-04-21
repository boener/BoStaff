#ifndef PULSE_EFFECT_H
#define PULSE_EFFECT_H

#include <FastLED.h>

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
  PulseEffect(LEDController* ledController, int segmentLen = 100) : 
    ledArray(nullptr), numLedsTotal(0), segmentLength(segmentLen), 
    baseHue(0), hueStep(1), waveCount(1), initialized(false),
    controller(ledController) {
    
    // Validate inputs
    if (!ledController) {
      Serial.println("ERROR: PulseEffect created with invalid parameters");
      return;
    }
    
    ledArray = ledController->getLeds();
    numLedsTotal = NUM_LEDS_TOTAL; // Use the global constant
    initialized = true;
    Serial.println("PulseEffect initialized with single-strip approach");
  }
  
  bool isInitialized() const {
    return initialized && ledArray != nullptr && controller != nullptr;
  }
  
  void setHue(uint8_t newHue) {
    baseHue = newHue;
  }
  
  void setWaveCount(uint8_t count) {
    if (count > 0 && count <= 5) { // Reasonable bounds
      waveCount = count;
    }
  }
  
  void update() {
    // Safety check - make sure we have valid memory and initialization
    if (!isInitialized()) {
      static bool errorLogged = false;
      if (!errorLogged) {
        Serial.println("ERROR: PulseEffect update called on uninitialized effect");
        errorLogged = true;
      }
      return;
    }
    
    // Update each segment
    updateSegment(0); // Segment 1
    updateSegment(1); // Segment 2
    updateSegment(2); // Segment 3
    updateSegment(3); // Segment 4
    
    // Slowly change the base hue for variation
    EVERY_N_MILLISECONDS(50) {
      baseHue += hueStep;
    }
  }
  
private:
  void updateSegment(int segmentIndex) {
    for (int i = 0; i < segmentLength; i++) {
      // Calculate distance from center (0 = center/hilt, 99 = far end)
      uint8_t distanceFromCenter = i;
      
      // Create multiple sine waves with different frequencies
      // Creates a pulse that travels outward from the center
      uint16_t brightness = 0;
      
      for (uint8_t w = 1; w <= waveCount; w++) {
        uint8_t divisor = (w > 1) ? w : uint8_t(1);
        uint8_t b = beatsin8(10 * w, 0, 255 / divisor, 0, distanceFromCenter * 8);
        brightness += b;
      }
      
      // Cap the brightness at 255
      brightness = (brightness > uint16_t(255)) ? uint16_t(255) : brightness;
      
      // Calculate hue variation based on distance from center
      uint8_t hueVar = baseHue + distanceFromCenter;
      
      // Set the LED color in the appropriate segment
      CRGB color = CHSV(hueVar, 255, brightness);
      
      switch (segmentIndex) {
        case 0: // Segment 1 - counts up from 0
          controller->getSegment1LED(i) = color;
          break;
        case 1: // Segment 2 - counts down from 199
          controller->getSegment2LED(i) = color;
          break;
        case 2: // Segment 3 - counts up from 200
          controller->getSegment3LED(i) = color;
          break;
        case 3: // Segment 4 - counts down from 399
          controller->getSegment4LED(i) = color;
          break;
      }
    }
  }
};

#endif // PULSE_EFFECT_H