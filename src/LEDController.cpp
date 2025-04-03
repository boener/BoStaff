#include "BoStaff.h"

void LEDController::begin(Config* cfg) {
  config = cfg;
  currentMode = config->currentMode;
  
  // Setup the LED strips with the updated pin assignments
  FastLED.addLeds<WS2812B, LED_PIN_1, GRB>(leds1, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, LED_PIN_2, GRB>(leds2, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  
  // Set initial brightness
  FastLED.setBrightness(config->brightness);
  
  // Clear the LEDs to start
  fill_solid(leds1, NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(leds2, NUM_LEDS_PER_STRIP, CRGB::Black);
  FastLED.show();
  
  // Initialize effect variables
  effectStep = 0;
  effectSpeed = 30; // Default speed
  
  Serial.println("LED Controller initialized");
}

void LEDController::update() {
  unsigned long currentMillis = millis();
  
  // Handle impact effect if active
  if (impactEffectActive) {
    if (currentMillis - impactEffectStart >= config->impactFlashDuration) {
      impactEffectActive = false;
      // Return to the normal effect
    } else {
      // Show impact effect (bright white flash)
      fill_solid(leds1, NUM_LEDS_PER_STRIP, CRGB::White);
      fill_solid(leds2, NUM_LEDS_PER_STRIP, CRGB::White);
      FastLED.show();
      return; // Don't run other effects during impact
    }
  }
  
  // External effect objects now handle the main effects
  // This class only handles the basic solid effect and impact flash
  
  // For compatibility with old code, we'll keep the solid color effect here
  if (currentMode == EFFECT_SOLID) {
    updateSolidEffect();
  }
  
  // FIXED: Update the LEDs for ALL modes, not just SOLID mode
  FastLED.show();
  
  // Increment effect step for animations
  effectStep++;
}

void LEDController::setMode(uint8_t mode) {
  if (mode < config->numModes) {
    currentMode = mode;
    effectStep = 0; // Reset effect animation
    
    Serial.print("Mode changed to: ");
    Serial.println(currentMode);
  }
}

void LEDController::triggerImpactEffect() {
  impactEffectActive = true;
  impactEffectStart = millis();
  Serial.println("Impact effect triggered");
}

void LEDController::setBrightness(uint8_t brightness) {
  FastLED.setBrightness(brightness);
  config->brightness = brightness;
}

// Effect implementation (note: most effects are now implemented in separate classes)
void LEDController::updateSolidEffect() {
  // Solid color effect - slowly changing hue
  CRGB color = CHSV(effectStep/2, 255, 255);
  
  fill_solid(leds1, NUM_LEDS_PER_STRIP, color);
  fill_solid(leds2, NUM_LEDS_PER_STRIP, color);
}
