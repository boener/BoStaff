#ifndef STROBE_EFFECT_H
#define STROBE_EFFECT_H

#include <FastLED.h>

// Advanced Strobe Effect with multi-mode capabilities
// Adapted for folded LED strip arrangement where LEDs at index 0 and (count-1) are at the center/hilt,
// and LEDs at index (count/2-1) and (count/2) are at the far end
class StrobeEffect {
private:
  CRGB* ledArray;
  int numLeds;
  uint8_t mode;      // 0=white strobe, 1=color strobe, 2=lightning
  uint8_t speed;     // Controls flash frequency
  uint8_t duty;      // Duty cycle (ratio of on vs off time)
  CRGB color;        // Color for colored strobe mode
  uint16_t count;    // Counter for effect timing
  uint8_t chance;    // Lightning strike chance (0-255)
  bool active;       // Current state of the strobe
  bool isFolded;     // Whether the LED strip is folded
  uint8_t flashMaxBrightness; // Maximum brightness for flash (to prevent power issues)
  
public:
  StrobeEffect(CRGB* leds, int count, bool folded = true) {
    // Validate inputs
    if (!leds || count <= 0) {
      // Handle invalid input
      ledArray = nullptr;
      numLeds = 0;
      Serial.println("ERROR: StrobeEffect created with invalid parameters");
      return;
    }
    
    ledArray = leds;
    numLeds = count;
    mode = 0;
    speed = 50;
    duty = 10;     // 10% on, 90% off by default
    color = CRGB::White;
    count = 0;
    chance = 5;    // Low chance of lightning by default
    active = false;
    isFolded = folded;
    flashMaxBrightness = 80;  // Reduced from 255 to 80 (approximately 30%)
  }
  
  void setMode(uint8_t m) {
    if (m < 3) mode = m;
  }
  
  void setSpeed(uint8_t s) {
    speed = s;
  }
  
  void setDuty(uint8_t d) {
    duty = constrain(d, 1, 99); // Prevent 0% or 100%
  }
  
  void setColor(CRGB c) {
    color = c;
  }
  
  void setFlashBrightness(uint8_t brightness) {
    flashMaxBrightness = brightness;
  }
  
  void update() {
    // Safety check - make sure we have valid memory
    if (!ledArray || numLeds <= 0) {
      return;
    }
    
    // Variables declared outside case statements to avoid C++ scope errors
    CRGB flashColor;
    CRGB scaledColor;
    
    // Different behavior based on mode
    switch (mode) {
      case 0: // Classic white strobe
        flashColor = CRGB(flashMaxBrightness, flashMaxBrightness, flashMaxBrightness); // Dimmed white
        updateClassicStrobe(flashColor);
        break;
        
      case 1: // Color strobe
        // Scale color to maximum brightness
        scaledColor = color;
        scaledColor.nscale8(flashMaxBrightness);
        updateClassicStrobe(scaledColor);
        break;
        
      case 2: // Lightning effect
        updateLightning();
        break;
    }
    
    count++;
  }
  
private:
  void updateClassicStrobe(CRGB flashColor) {
    // Safety check again
    if (!ledArray || numLeds <= 0) return;
    
    // Calculate period based on speed
    uint16_t period = 255 - speed; // Higher speed = shorter period
    if (period < 10) period = 10;  // Prevent ultra-fast flashing
    
    // Calculate timing
    uint16_t onTime = (period * duty) / 100;
    
    // Determine on/off state
    bool isOn = (count % period) < onTime;
    
    // Apply to all LEDs
    for (int i = 0; i < numLeds; i++) {
      if (i >= 0 && i < numLeds) { // Bounds check
        ledArray[i] = isOn ? flashColor : CRGB::Black;
      }
    }
  }
  
  void updateLightning() {
    // Safety check again
    if (!ledArray || numLeds <= 0) return;
    
    // Reset all LEDs to black first
    fill_solid(ledArray, numLeds, CRGB::Black);
    
    uint8_t midPoint = numLeds / 2;
    
    // Reduced brightness white for lightning
    CRGB lightningColor = CRGB(flashMaxBrightness, flashMaxBrightness, flashMaxBrightness);
    
    // Randomly decide if we should create a lightning flash
    if (random8() < chance) {
      if (isFolded) {
        // For folded arrangement, lightning can appear at different places
        if (random8() < 85) { // 1/3 chance to strike near center
          // Strike near the center (hilt)
          uint8_t strikeLength = random8(midPoint / 3);
          for (int i = 0; i < strikeLength; i++) {
            if (i >= 0 && i < numLeds) { // Bounds check
              ledArray[i] = lightningColor; // Starting from one end (center)
            }
            
            int opposite = numLeds - 1 - i;
            if (opposite >= 0 && opposite < numLeds) { // Bounds check
              ledArray[opposite] = lightningColor; // Starting from other end (center)
            }
          }
        } else if (random8() < 128) { // 1/3 chance to strike near tip
          // Strike near the far end (tip)
          uint8_t strikeLength = random8(midPoint / 3);
          for (int i = 0; i < strikeLength; i++) {
            int idx1 = midPoint - 1 - i;
            int idx2 = midPoint + i;
            
            if (idx1 >= 0 && idx1 < numLeds) { // Bounds check
              ledArray[idx1] = lightningColor; // Near middle from first half
            }
            
            if (idx2 >= 0 && idx2 < numLeds) { // Bounds check
              ledArray[idx2] = lightningColor; // Near middle from second half
            }
          }
        } else { // 1/3 chance to strike full staff
          // Full staff lightning (but with reduced LEDs to save power)
          for (int i = 0; i < numLeds; i += 3) { // Only light every 3rd LED
            if (i >= 0 && i < numLeds) { // Bounds check
              ledArray[i] = lightningColor;
            }
          }
        }
      } else {
        // Full lightning for non-folded arrangement (with power saving)
        for (int i = 0; i < numLeds; i += 3) { // Only light every 3rd LED
          if (i >= 0 && i < numLeds) { // Bounds check
            ledArray[i] = lightningColor;
          }
        }
      }
      
      // Schedule afterglow
      active = true;
    } else if (active) {
      // Decay the lightning effect with afterglow (reduced brightness)
      uint8_t fade = random8(1, 3); // Reduced from 2-7 to 1-3
      for (int i = 0; i < numLeds; i++) {
        if (random8() < 80 && i >= 0 && i < numLeds) { // Reduced from 120 to 80 (31% chance) + bounds check
          ledArray[i] = CRGB(fade, fade, fade + random8(1, 2)); // Blue tint
        }
      }
      
      // End the lightning
      active = false;
    }
  }
};

#endif // STROBE_EFFECT_H