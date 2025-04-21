#include "BoStaff.h"

// FastLED configuration
// Note: These must be defined before FastLED.h is included,
// but since BoStaff.h already includes it, these might not take effect
// We'll use the settings already defined in FastLED library

// Helper function to map positions in the virtual 0-99 range to the actual folded LED positions
int LEDController::mapToFoldedIndex(int virtualPos, int segment) {
  // Ensure virtualPos is in the valid range 0-99
  virtualPos = constrain(virtualPos, 0, 99);
  
  switch (segment) {
    case 1: // First segment (counts up)
      return virtualPos; // Direct mapping 0-99
    case 2: // Second segment (counts down)
      return 199 - virtualPos; // Maps 0->199, 1->198, ..., 99->100
    case 3: // Third segment (counts up)
      return 200 + virtualPos; // Maps 0->200, 1->201, ..., 99->299
    case 4: // Fourth segment (counts down)
      return 399 - virtualPos; // Maps 0->399, 1->398, ..., 99->300
    default:
      return 0; // Fallback
  }
}

// Functions to access LED segments with the correct folding logic
CRGB& LEDController::getSegment1LED(int pos) {
  return leds[mapToFoldedIndex(pos, 1)];
}

CRGB& LEDController::getSegment2LED(int pos) {
  return leds[mapToFoldedIndex(pos, 2)];
}

CRGB& LEDController::getSegment3LED(int pos) {
  return leds[mapToFoldedIndex(pos, 3)];
}

CRGB& LEDController::getSegment4LED(int pos) {
  return leds[mapToFoldedIndex(pos, 4)];
}

void LEDController::begin(Config* cfg) {
  config = cfg;
  currentMode = config->currentMode;
  
  // Setup the single LED strip
  Serial.println("Initializing single LED strip with four segments");
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS_TOTAL).setCorrection(TypicalLEDStrip);
  
  // Set maximum power limit to avoid current issues (3A at 5V = 15W)
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 3000);
  
  // Set initial brightness
  normalBrightness = config->brightness;
  FastLED.setBrightness(normalBrightness);
  
  // Clear all LEDs to start
  fill_solid(leds, NUM_LEDS_TOTAL, CRGB::Black);
  
  // Initial show to clear all LEDs - with improved timing approach
  delay(1);
  noInterrupts();  // Disable interrupts during LED update
  FastLED.show();
  interrupts();    // Re-enable interrupts
  delay(1);
  
  // Initialize effect variables
  effectStep = 0;
  effectSpeed = 30; // Default speed
  impactEffectActive = false;
  
  // Initialize frame rate control variables
  lastUpdate = millis();
  
  Serial.println("LED Controller initialized with single strip");
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
      Serial.println("Impact effect ended, restored normal brightness");
      
      // Clear all LEDs after impact to prevent any artifacts
      fill_solid(leds, NUM_LEDS_TOTAL, CRGB::Black);
      
      // Update LEDs with improved timing approach
      delay(1);
      noInterrupts();
      FastLED.show();
      interrupts();
      delay(1);
    } else {
      // Show impact effect (dim white flash)
      FastLED.setBrightness(config->impactBrightness); // Use the impact-specific brightness
      
      // Use dimmer white (25, 25, 25) instead of full white (255, 255, 255)
      // This ensures the color itself is also dimmer, not just the overall brightness
      CRGB dimWhite = CRGB(25, 25, 25);
      fill_solid(leds, NUM_LEDS_TOTAL, dimWhite);
      
      // Update LEDs with improved timing approach
      delay(1);
      noInterrupts();
      FastLED.show();
      interrupts();
      delay(1);
      return; // Don't run other effects during impact
    }
  }
  
  // Only update LED buffer if no impact effect is active
  if (!impactEffectActive) {
    // For compatibility with old code, we'll keep the solid color effect here
    if (currentMode == EFFECT_SOLID) {
      updateSolidEffect();
    }
    
    // Update LEDs with improved timing approach
    delay(1);
    noInterrupts();
    FastLED.show();
    interrupts();
    delay(1);
    
    // Increment effect step for animations
    effectStep++;
  }
}

void LEDController::setMode(uint8_t mode) {
  if (mode < config->numModes) {
    currentMode = mode;
    effectStep = 0; // Reset effect animation
    
    // Clear all LEDs when changing mode
    fill_solid(leds, NUM_LEDS_TOTAL, CRGB::Black);
    
    // Update LEDs with improved timing approach
    delay(1);
    noInterrupts();
    FastLED.show();
    interrupts();
    delay(1);
    
    Serial.print("Mode changed to: ");
    Serial.println(currentMode);
  }
}

void LEDController::triggerImpactEffect() {
  impactEffectActive = true;
  impactEffectStart = millis();
  
  Serial.println("Impact effect triggered");
  Serial.print("Normal brightness: "); Serial.println(normalBrightness);
  Serial.print("Impact brightness: "); Serial.println(config->impactBrightness); 
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
  // Clear all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, CRGB::Black);
  
  // Update LEDs with improved timing approach
  delay(1);
  noInterrupts();
  FastLED.show();
  interrupts();
  delay(1);
  
  // Reset effect step counter
  effectStep = 0;
  
  // Ensure impactEffectActive is reset
  impactEffectActive = false;
  
  // Set brightness to correct value
  FastLED.setBrightness(normalBrightness);
  
  Serial.println("LED strip forcefully refreshed");
}

// Effect implementation for solid color
void LEDController::updateSolidEffect() {
  // Solid color effect - slowly changing hue
  CRGB color = CHSV(effectStep/2, 255, 255);
  
  // Apply the same color to all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, color);
}