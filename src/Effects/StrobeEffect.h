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
    // Different behavior based on mode
    switch (mode) {
      case 0: // Classic white strobe
        updateClassicStrobe(CRGB(flashMaxBrightness, flashMaxBrightness, flashMaxBrightness)); // Dimmed white
        break;
        
      case 1: // Color strobe
        // Scale color to maximum brightness
        CRGB scaledColor = color;
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
    // Calculate period based on speed
    uint16_t period = 255 - speed; // Higher speed = shorter period
    if (period < 10) period = 10;  // Prevent ultra-fast flashing
    
    // Calculate timing
    uint16_t onTime = (period * duty) / 100;
    
    // Determine on/off state
    bool isOn = (count % period) < onTime;
    
    // Apply to all LEDs
    for (int i = 0; i < numLeds; i++) {
      ledArray[i] = isOn ? flashColor : CRGB::Black;
    }
  }
  
  void updateLightning() {
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
            ledArray[i] = lightningColor; // Starting from one end (center)
            ledArray[numLeds - 1 - i] = lightningColor; // Starting from other end (center)
          }
        } else if (random8() < 128) { // 1/3 chance to strike near tip
          // Strike near the far end (tip)
          uint8_t strikeLength = random8(midPoint / 3);
          for (int i = 0; i < strikeLength; i++) {
            ledArray[midPoint - 1 - i] = lightningColor; // Near middle from first half
            ledArray[midPoint + i] = lightningColor; // Near middle from second half
          }
        } else { // 1/3 chance to strike full staff
          // Full staff lightning (but with reduced LEDs to save power)
          for (int i = 0; i < numLeds; i += 3) { // Only light every 3rd LED
            ledArray[i] = lightningColor;
          }
        }
      } else {
        // Full lightning for non-folded arrangement (with power saving)
        for (int i = 0; i < numLeds; i += 3) { // Only light every 3rd LED
          ledArray[i] = lightningColor;
        }
      }
      
      // Schedule afterglow
      active = true;
    } else if (active) {
      // Decay the lightning effect with afterglow (reduced brightness)
      uint8_t fade = random8(1, 3); // Reduced from 2-7 to 1-3
      for (int i = 0; i < numLeds; i++) {
        if (random8() < 80) { // Reduced from 120 to 80 (31% chance)
          ledArray[i] = CRGB(fade, fade, fade + random8(1, 2)); // Blue tint
        }
      }
      
      // End the lightning
      active = false;
    }
  }
};

#endif // STROBE_EFFECT_H