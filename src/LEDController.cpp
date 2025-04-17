#include "BoStaff.h"

void LEDController::begin(Config* cfg) {
  config = cfg;
  currentMode = config->currentMode;
  
  // Setup the LED strips with the updated pin assignments
  FastLED.addLeds<WS2812B, LED_PIN_1, GRB>(leds1, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, LED_PIN_2, GRB>(leds2, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  
  // Set initial brightness (reduced to 25, approximately 10% of max 255)
  normalBrightness = config->brightness;
  FastLED.setBrightness(normalBrightness);
  
  // Clear the LEDs to start
  fill_solid(leds1, NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(leds2, NUM_LEDS_PER_STRIP, CRGB::Black);
  FastLED.show();
  
  // Initialize effect variables
  effectStep = 0;
  effectSpeed = 30; // Default speed
  impactEffectActive = false;
  
  Serial.println("LED Controller initialized");
  Serial.print("Brightness set to: "); Serial.println(normalBrightness);
  Serial.print("Impact brightness set to: "); Serial.println(config->impactBrightness);
}

void LEDController::update() {
  unsigned long currentMillis = millis();
  
  // Handle impact effect if active
  if (impactEffectActive) {
    if (currentMillis - impactEffectStart >= config->impactFlashDuration) {
      // Impact effect is over, restore normal brightness
      impactEffectActive = false;
      FastLED.setBrightness(normalBrightness);
      
      // Clear both strips after impact to prevent any artifacts
      fill_solid(leds1, NUM_LEDS_PER_STRIP, CRGB::Black);
      fill_solid(leds2, NUM_LEDS_PER_STRIP, CRGB::Black);
      
      // Force a show here to ensure black frame is displayed before next effect starts
      FastLED.show();
    } else {
      // Show impact effect (dim white flash)
      FastLED.setBrightness(config->impactBrightness); // Use the impact-specific brightness
      
      // Use dimmer white (25, 25, 25) instead of full white (255, 255, 255)
      // This ensures the color itself is also dimmer, not just the overall brightness
      CRGB dimWhite = CRGB(25, 25, 25);
      fill_solid(leds1, NUM_LEDS_PER_STRIP, dimWhite);
      fill_solid(leds2, NUM_LEDS_PER_STRIP, dimWhite);
      
      FastLED.show();
      return; // Don't run other effects during impact
    }
  }
  
  // Only update LED buffer if no impact effect is active
  if (!impactEffectActive) {
    // For compatibility with old code, we'll keep the solid color effect here
    if (currentMode == EFFECT_SOLID) {
      updateSolidEffect();
    }
    
    // Update the LEDs for ALL modes, not just SOLID mode
    FastLED.show();
    
    // Increment effect step for animations
    effectStep++;
  }
}

void LEDController::setMode(uint8_t mode) {
  if (mode < config->numModes) {
    // Disable interrupts during mode change to prevent race conditions
    noInterrupts();
    
    currentMode = mode;
    effectStep = 0; // Reset effect animation
    
    // Clear LEDs when changing mode
    fill_solid(leds1, NUM_LEDS_PER_STRIP, CRGB::Black);
    fill_solid(leds2, NUM_LEDS_PER_STRIP, CRGB::Black);
    FastLED.show();
    
    // Re-enable interrupts
    interrupts();
    
    Serial.print("Mode changed to: ");
    Serial.println(currentMode);
  }
}

void LEDController::triggerImpactEffect() {
  // Disable interrupts during impact effect activation to prevent race conditions
  noInterrupts();
  
  impactEffectActive = true;
  impactEffectStart = millis();
  
  // Re-enable interrupts
  interrupts();
  
  // Remove serial printing to avoid interruptions
  // Serial.println("Impact effect triggered");
}

void LEDController::setBrightness(uint8_t brightness) {
  normalBrightness = brightness;
  
  // Only set FastLED brightness directly if no impact effect is active
  if (!impactEffectActive) {
    FastLED.setBrightness(brightness);
  }
  
  config->brightness = brightness;
}

// Force a complete refresh of the LED strips
void LEDController::forceRefresh() {
  // Disable interrupts during refresh to prevent race conditions
  noInterrupts();
  
  // Clear both strips
  fill_solid(leds1, NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(leds2, NUM_LEDS_PER_STRIP, CRGB::Black);
  FastLED.show();
  
  // Reset effect step counter
  effectStep = 0;
  
  // Ensure impactEffectActive is reset
  impactEffectActive = false;
  
  // Set brightness to correct value
  FastLED.setBrightness(normalBrightness);
  
  // Re-enable interrupts
  interrupts();
  
  Serial.println("LED strips forcefully refreshed");
}

// Effect implementation (note: most effects are now implemented in separate classes)
void LEDController::updateSolidEffect() {
  // Solid color effect - slowly changing hue
  CRGB color = CHSV(effectStep/2, 255, 255);
  
  fill_solid(leds1, NUM_LEDS_PER_STRIP, color);
  fill_solid(leds2, NUM_LEDS_PER_STRIP, color);
}