#ifndef STROBE_EFFECT_H
#define STROBE_EFFECT_H

#include <FastLED.h>

// Advanced Strobe Effect with multi-mode capabilities
class StrobeEffect {
private:
  CRGB* ledArray;
  int numLeds;
  uint8_t mode;      // 0=white strobe, 1=color strobe, 2=lightning
  uint8_t speed;     // Controls flash frequency
  uint8_t duty;      // Duty cycle (ratio of on vs off time)
  CRGB color;        // Color for colored strobe mode
  bool active;       // Current state of the strobe
  bool isFolded;     // Whether the LED strip is folded
  uint8_t flashMaxBrightness; // Maximum brightness for flash (to prevent power issues)
  bool initialized;  // New flag to track initialization status
  
  // Simplify the timing mechanism
  unsigned long lastFlashChange;
  bool flashOn;
  
public:
  StrobeEffect(CRGB* leds, int count, bool folded = true) : 
    ledArray(leds), numLeds(count), mode(0), speed(50), duty(10),
    color(CRGB::White), active(false), isFolded(folded), 
    flashMaxBrightness(25), initialized(true),
    lastFlashChange(0), flashOn(false) {
    
    // Validate inputs
    if (!leds || count <= 0) {
      Serial.println("ERROR: StrobeEffect created with invalid parameters");
      initialized = false;
      return;
    }
  }
  
  bool isInitialized() const {
    return initialized && ledArray != nullptr && numLeds > 0;
  }
  
  void setMode(uint8_t m) {
    if (m < 3) mode = m;
  }
  
  void setSpeed(uint8_t s) {
    speed = s;
  }
  
  void setDuty(uint8_t d) {
    if (d < 1) d = 1;
    if (d > 99) d = 99;
    duty = d;
  }
  
  void setColor(CRGB c) {
    color = c;
  }
  
  void setFlashBrightness(uint8_t brightness) {
    flashMaxBrightness = brightness;
  }
  
  void update() {
    // Safety check - make sure we have valid memory and initialization
    if (!isInitialized()) {
      return;
    }
    
    // Use a fixed, very slow strobe rate to debug the issue
    // 500ms on, 500ms off
    unsigned long currentMillis = millis();
    unsigned long interval = 500; // Fixed 500ms interval for debugging
    
    // Check if it's time to change the flash state
    if (currentMillis - lastFlashChange >= interval) {
      lastFlashChange = currentMillis;
      flashOn = !flashOn; // Toggle the flash state
      
      // Set all LEDs to either the flash color or black based on the state
      CRGB targetColor = flashOn ? CRGB(flashMaxBrightness, flashMaxBrightness, flashMaxBrightness) : CRGB::Black;
      
      // Fill the entire strip with the target color
      fill_solid(ledArray, numLeds, targetColor);
    }
  }
};

#endif // STROBE_EFFECT_H