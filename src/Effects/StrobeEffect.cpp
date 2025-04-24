#include "BoStaff.h"
#include "Effects/StrobeEffect.h"
#include "EffectsConfig.h" // Added include for configuration parameters

StrobeEffect::StrobeEffect(LEDController* ledController, int segmentLen) : 
  ledArray(nullptr), numLedsTotal(0), segmentLength(segmentLen),
  mode(0), count(0), chance(5), active(false), 
  initialized(false), controller(ledController) {
  
  // Initialize values from EffectsConfig.h
  onTime = STROBE_ON_TIME;
  offTime = STROBE_OFF_TIME;
  color = STROBE_COLOR;
  fadeOut = STROBE_FADE_OUT;
  fadeRate = STROBE_FADE_RATE;
  
  // Use the brightness from config
  flashMaxBrightness = STROBE_BRIGHTNESS;
  
  // Calculate speed and duty cycle from on/off times
  uint16_t totalPeriod = onTime + offTime;
  speed = map(totalPeriod, 500, 50, 0, 255); // Inverse mapping: shorter period = higher speed
  duty = (onTime * 100) / totalPeriod;       // Calculate duty cycle as percentage
  
  // Validate inputs
  if (!ledController) {
    Serial.println("ERROR: StrobeEffect created with invalid parameters");
    return;
  }
  
  ledArray = ledController->getLeds();
  numLedsTotal = NUM_LEDS_TOTAL; // Use the global constant
  initialized = true;
  
  Serial.println("StrobeEffect initialized with single-strip approach");
  Serial.print("Strobe Effect Config - On Time: ");
  Serial.print(onTime);
  Serial.print(", Off Time: ");
  Serial.print(offTime);
  Serial.print(", Fade Out: ");
  Serial.print(fadeOut ? "Enabled" : "Disabled");
  Serial.print(", Brightness: ");
  Serial.println(flashMaxBrightness);
}

StrobeEffect::~StrobeEffect() {
  // No dynamic memory to free, but we still need to reset state
  initialized = false;
  ledArray = nullptr; // Don't delete ledArray as it's managed elsewhere
  controller = nullptr;
}

bool StrobeEffect::isInitialized() const {
  return initialized && ledArray != nullptr && controller != nullptr;
}

void StrobeEffect::setMode(uint8_t m) {
  if (m < 3) mode = m;
}

void StrobeEffect::setSpeed(uint8_t s) {
  speed = s;
  
  // Recalculate on/off times based on speed
  uint16_t period = 255 - speed; // Higher speed = shorter period
  if (period < 10) period = 10;  // Prevent ultra-fast flashing
  
  // Preserve duty cycle
  onTime = (period * duty) / 100;
  offTime = period - onTime;
}

void StrobeEffect::setDuty(uint8_t d) {
  // Use explicit cast for comparison to avoid type mismatch
  if (d < uint8_t(1)) d = 1;
  if (d > uint8_t(99)) d = 99;
  duty = d;
  
  // Recalculate on time based on new duty cycle
  uint16_t period = onTime + offTime;
  onTime = (period * duty) / 100;
  offTime = period - onTime;
}

void StrobeEffect::setColor(CRGB c) {
  color = c;
}

void StrobeEffect::setFlashBrightness(uint8_t brightness) {
  flashMaxBrightness = brightness;
}

void StrobeEffect::update() {
  // Safety check - make sure we have valid memory and initialization
  if (!isInitialized()) {
    static bool errorLogged = false;
    if (!errorLogged) {
      Serial.println("ERROR: StrobeEffect update called on uninitialized effect");
      errorLogged = true;
    }
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

void StrobeEffect::updateClassicStrobe(CRGB flashColor) {
  // Use parameters from EffectsConfig.h to determine strobe timing
  
  // Calculate period based on onTime and offTime from config
  uint16_t period = onTime + offTime;
  
  // Determine on/off state using onTime
  bool isOn = (count % period) < onTime;
  
  // Apply to all segments
  for (int i = 0; i < segmentLength; i++) {
    CRGB pixelColor;
    
    if (isOn) {
      pixelColor = flashColor;
    } else {
      // If fade out is enabled, apply progressive dimming
      if (fadeOut && count % period >= onTime && count % period < (onTime + fadeRate)) {
        // Calculate fade level (high at start of fade, low at end)
        uint8_t fadeLevel = 255 - map(count % period - onTime, 0, fadeRate, 0, 255);
        pixelColor = flashColor;
        pixelColor.nscale8(fadeLevel);
      } else {
        pixelColor = CRGB::Black;
      }
    }
    
    // Update all four segments
    controller->getSegment1LED(i) = pixelColor;
    controller->getSegment2LED(i) = pixelColor;
    controller->getSegment3LED(i) = pixelColor;
    controller->getSegment4LED(i) = pixelColor;
  }
}

void StrobeEffect::updateLightning() {
  // Clear all LEDs first
  for (int i = 0; i < segmentLength; i++) {
    controller->getSegment1LED(i) = CRGB::Black;
    controller->getSegment2LED(i) = CRGB::Black;
    controller->getSegment3LED(i) = CRGB::Black;
    controller->getSegment4LED(i) = CRGB::Black;
  }
  
  // Reduced brightness white for lightning
  CRGB lightningColor = CRGB(flashMaxBrightness, flashMaxBrightness, flashMaxBrightness);
  
  // Randomly decide if we should create a lightning flash
  if (random8() < chance) {
    // Determine what type of lightning strike to create
    uint8_t strikeType = random8(3); // 0, 1, or 2
    
    switch (strikeType) {
      case 0: // Strike near center (hilt)
        {
          uint8_t strikeLength = random8(segmentLength / 3);
          for (int i = 0; i < strikeLength; i++) {
            controller->getSegment1LED(i) = lightningColor;
            controller->getSegment2LED(i) = lightningColor;
            controller->getSegment3LED(i) = lightningColor;
            controller->getSegment4LED(i) = lightningColor;
          }
        }
        break;
        
      case 1: // Strike near far end (tip)
        {
          uint8_t strikeLength = random8(segmentLength / 3);
          for (int i = 0; i < strikeLength; i++) {
            int pos = segmentLength - 1 - i;
            controller->getSegment1LED(pos) = lightningColor;
            controller->getSegment2LED(pos) = lightningColor;
            controller->getSegment3LED(pos) = lightningColor;
            controller->getSegment4LED(pos) = lightningColor;
          }
        }
        break;
        
      case 2: // Full staff lightning (but with reduced LEDs to save power)
        for (int i = 0; i < segmentLength; i += 3) { // Only light every 3rd LED
          controller->getSegment1LED(i) = lightningColor;
          controller->getSegment2LED(i) = lightningColor;
          controller->getSegment3LED(i) = lightningColor;
          controller->getSegment4LED(i) = lightningColor;
        }
        break;
    }
    
    // Schedule afterglow
    active = true;
  } else if (active) {
    // Decay the lightning effect with afterglow (reduced brightness)
    // Use fadeRate from config to control the decay speed
    uint8_t fade = random8(1, fadeRate / 10 + 1);
    
    for (int segment = 1; segment <= 4; segment++) {
      for (int i = 0; i < segmentLength; i++) {
        if (random8() < 80) { // 31% chance
          CRGB afterglowColor = CRGB(fade, fade, fade + random8(1, 2)); // Blue tint
          
          // Apply to the correct segment
          switch (segment) {
            case 1:
              controller->getSegment1LED(i) = afterglowColor;
              break;
            case 2:
              controller->getSegment2LED(i) = afterglowColor;
              break;
            case 3:
              controller->getSegment3LED(i) = afterglowColor;
              break;
            case 4:
              controller->getSegment4LED(i) = afterglowColor;
              break;
          }
        }
      }
    }
    
    // End the lightning
    active = false;
  }
}