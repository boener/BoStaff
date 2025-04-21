#ifndef FIRE_EFFECT_H
#define FIRE_EFFECT_H

#include <FastLED.h>

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
  FireEffect(LEDController* ledController, int segmentLen = 100) : 
    ledArray(nullptr), numLedsTotal(0), segmentLength(segmentLen), heat(nullptr), 
    cooling(85), sparking(90), initialized(false), controller(ledController) {
    
    // Validate inputs
    if (!ledController) {
      Serial.println("ERROR: FireEffect created with invalid parameters");
      return;
    }
    
    ledArray = ledController->getLeds();
    numLedsTotal = NUM_LEDS_TOTAL; // Use the global constant
    
    // Allocate the heat array - we need one array per segment
    heat = new byte[segmentLength * 4]; // 4 segments
    if (heat) {
      // Initialize all elements to zero
      memset(heat, 0, segmentLength * 4);
      initialized = true; // Mark as successfully initialized
      Serial.println("FireEffect initialized with single-strip approach");
    } else {
      Serial.println("ERROR: FireEffect failed to allocate heat arrays");
    }
  }
  
  ~FireEffect() {
    if (heat) {
      delete[] heat;
      heat = nullptr;  // Prevent double deletion
    }
    initialized = false;
    ledArray = nullptr; // Don't delete ledArray as it's managed elsewhere
  }
  
  bool isInitialized() const {
    return initialized && heat != nullptr && ledArray != nullptr && controller != nullptr;
  }
  
  void setCooling(uint8_t cool) {
    cooling = cool;
  }
  
  void setSparking(uint8_t spark) {
    sparking = spark;
  }
  
  void update() {
    // Safety check - make sure we have valid memory and initialization
    if (!isInitialized()) {
      static bool errorLogged = false;
      if (!errorLogged) {
        Serial.println("ERROR: FireEffect update called on uninitialized effect");
        errorLogged = true;
      }
      return;
    }
    
    // We'll process each segment separately
    updateSegment(0); // Segment 1 - counts up from 0
    updateSegment(1); // Segment 2 - counts down from 199
    updateSegment(2); // Segment 3 - counts up from 200
    updateSegment(3); // Segment 4 - counts down from 399
  }
  
private:
  void updateSegment(int segmentIndex) {
    // Get the starting index for this segment's heat array
    int heatOffset = segmentIndex * segmentLength;
    
    // Step 1: Cool down every cell a little
    for (int i = 0; i < segmentLength; i++) {
      heat[heatOffset + i] = qsub8(heat[heatOffset + i], 
                                  random8(0, ((cooling * 10) / segmentLength) + 2));
    }
  
    // Step 2: Heat from each cell drifts 'up' and diffuses
    // For all segments, heat rises from center/hilt (pos 0) toward far end (pos 99)
    for (int k = segmentLength - 1; k >= 2; k--) {
      heat[heatOffset + k] = (heat[heatOffset + k - 1] + 
                             heat[heatOffset + k - 2] + 
                             heat[heatOffset + k - 2]) / 3;
    }
    
    // Step 3: Randomly ignite new sparks at the bottom/center (pos 0)
    if (random8() < sparking) {
      int y = random8(7); // Near the center/hilt
      heat[heatOffset + y] = qadd8(heat[heatOffset + y], random8(160, 255));
    }
  
    // Step 4: Map from heat cells to LED colors
    for (int j = 0; j < segmentLength; j++) {
      CRGB color = HeatColor(heat[heatOffset + j]);
      
      // Use the appropriate segment addressing based on segment index
      switch (segmentIndex) {
        case 0: // Segment 1 - counts up from 0
          controller->getSegment1LED(j) = color;
          break;
        case 1: // Segment 2 - counts down from 199
          controller->getSegment2LED(j) = color;
          break;
        case 2: // Segment 3 - counts up from 200
          controller->getSegment3LED(j) = color;
          break;
        case 3: // Segment 4 - counts down from 399
          controller->getSegment4LED(j) = color;
          break;
      }
    }
  }
};

#endif // FIRE_EFFECT_H