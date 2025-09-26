#include "BoStaff.h"

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

// Centralized safe strip refresh method
void LEDController::safeStripRefresh() {
  // Add a small delay to ensure any pending I2C operations complete
  delay(1);
  // Disable interrupts during LED update to prevent timing issues
  noInterrupts();
  FastLED.show();
  // Re-enable interrupts
  interrupts();
  // Add another small delay for stability
  delay(1);
}

void LEDController::begin(Config* cfg) {
  config = cfg;
  currentMode = config->currentMode;
  
  // Setup the single LED strip
  DEBUG_PRINTLN("Initializing single LED strip with four segments");
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS_TOTAL).setCorrection(TypicalLEDStrip);
  
  // Set maximum power limit to avoid current issues (3A at 5V = 15W)
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 3000);
  
  // Set initial brightness
  normalBrightness = config->brightness;
  FastLED.setBrightness(normalBrightness);
  currentBrightnessMode = BRIGHTNESS_NORMAL;
  savedBrightness = normalBrightness;
  
  // Clear all LEDs to start
  fill_solid(leds, NUM_LEDS_TOTAL, CRGB::Black);
  
  // Initial show to clear all LEDs
  safeStripRefresh();
  
  // Initialize effect variables
  effectStep = 0;
  effectSpeed = 30; // Default speed
  impactEffectActive = false;
  
  // Initialize frame rate control variables
  lastUpdate = millis();
  
  DEBUG_PRINTLN("LED Controller initialized with single strip");
  DEBUG_PRINT("Brightness set to: "); DEBUG_PRINTLN(normalBrightness);
  DEBUG_PRINT("Impact brightness set to: "); DEBUG_PRINTLN(config->impactBrightness);
}

void LEDController::update() {
  unsigned long currentMillis = millis();
  
  // Handle impact effect if active
  if (impactEffectActive) {
    if (currentMillis - impactEffectStart >= config->impactFlashDuration) {
      // Impact effect is over, restore normal brightness
      impactEffectActive = false;
      restorePreviousBrightness();
      DEBUG_PRINTLN("Impact effect ended, restored normal brightness");
      
      // Clear all LEDs after impact to prevent any artifacts
      fill_solid(leds, NUM_LEDS_TOTAL, CRGB::Black);
      safeStripRefresh();
    } else {
      // Show impact effect with configured color
      FastLED.setBrightness(config->impactBrightness); // Use the impact-specific brightness
      
      // If fade out is enabled, calculate the fade level
      if (config->impactFadeOut) {
        // Calculate how far we are through the impact flash duration
        unsigned long elapsedTime = currentMillis - impactEffectStart;
        float progress = (float)elapsedTime / config->impactFlashDuration;
        
        // Apply fade based on progress and fade rate
        // Higher fade rate means faster fade
        if (progress > 0.5) { // Start fading after half the duration
          float fadeAmount = (progress - 0.5) * 2.0 * config->impactFadeRate;
          fadeAmount = constrain(fadeAmount, 0.0, 1.0);
          
          // Create a faded version of the impact color
          CRGB fadedColor = IMPACT_COLOR;
          fadedColor.fadeToBlackBy(fadeAmount * 255);
          fill_solid(leds, NUM_LEDS_TOTAL, fadedColor);
        } else {
          // First half of the effect uses full impact color
          fill_solid(leds, NUM_LEDS_TOTAL, IMPACT_COLOR);
        }
      } else {
        // No fade, just use the impact color
        fill_solid(leds, NUM_LEDS_TOTAL, IMPACT_COLOR);
      }
      
      safeStripRefresh();
      return; // Don't run other effects during impact
    }
  }
  
  // Only update LED buffer if no impact effect is active
  if (!impactEffectActive) {
    // Handle effects based on current mode - UPDATED FOR NEW EFFECTS
    switch (currentMode) {
      case EFFECT_SLOW_RAINBOW:
        updateSlowRainbowEffect();
        break;
      case EFFECT_SOLID_BLUE:
        updateSolidBlueEffect();
        break;
      case EFFECT_SOLID_GREEN:
        updateSolidGreenEffect();
        break;
      case EFFECT_SOLID_RED:
        updateSolidRedEffect();
        break;
      case EFFECT_SOLID_PURPLE:
        updateSolidPurpleEffect();
        break;
      case EFFECT_SOLID_YELLOW:
        updateSolidYellowEffect();
        break;
      case EFFECT_SOLID_WHITE:
        updateSolidWhiteEffect();
        break;
      default:
        // For modes not handled here, fall back to solid red
        fill_solid(leds, NUM_LEDS_TOTAL, CRGB::Red);
        break;
    }
    
    safeStripRefresh();
    
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
    safeStripRefresh();
    
    DEBUG_PRINT("Mode changed to: ");
    DEBUG_PRINTLN(currentMode);
  }
}

void LEDController::triggerImpactEffect() {
  // Save the current brightness before changing to impact mode
  savedBrightness = getCurrentBrightness();
  impactEffectActive = true;
  impactEffectStart = millis();
  currentBrightnessMode = BRIGHTNESS_IMPACT;
  
  DEBUG_PRINTLN("Impact effect triggered");
  DEBUG_PRINT("Saved normal brightness: "); DEBUG_PRINTLN(savedBrightness);
  DEBUG_PRINT("Impact brightness: "); DEBUG_PRINTLN(config->impactBrightness); 
}

// Enhanced brightness management methods

void LEDController::setBrightness(uint8_t brightness) {
  // Safety bounds check
  brightness = constrain(brightness, 0, 255);
  
  if (!impactEffectActive) {
    normalBrightness = brightness;
    FastLED.setBrightness(brightness);
    config->brightness = brightness;
    
    // If we're manually setting brightness, update mode accordingly
    if (currentBrightnessMode != BRIGHTNESS_IMPACT) {
      currentBrightnessMode = BRIGHTNESS_NORMAL;
    }
    
    DEBUG_PRINT("Brightness set to: "); 
    DEBUG_PRINTLN(brightness);
  } else {
    // If impact effect is active, store for later but don't apply yet
    normalBrightness = brightness;
    config->brightness = brightness;
    DEBUG_PRINT("Brightness updated (will apply after impact effect): "); 
    DEBUG_PRINTLN(brightness);
  }
}

void LEDController::setBrightnessMode(BrightnessMode mode) {
  // Skip if already in this mode
  if (mode == currentBrightnessMode) return;
  
  // Save current brightness for potential restore later
  if (currentBrightnessMode == BRIGHTNESS_NORMAL) {
    savedBrightness = normalBrightness;
  }
  
  // Apply the new brightness mode
  switch (mode) {
    case BRIGHTNESS_NORMAL:
      FastLED.setBrightness(normalBrightness);
      DEBUG_PRINT("Brightness mode set to NORMAL: ");
      DEBUG_PRINTLN(normalBrightness);
      break;
      
    case BRIGHTNESS_IMPACT:
      // This is usually handled by triggerImpactEffect()
      FastLED.setBrightness(config->impactBrightness);
      DEBUG_PRINT("Brightness mode set to IMPACT: ");
      DEBUG_PRINTLN(config->impactBrightness);
      break;
      
    case BRIGHTNESS_LOW_BATTERY:
      FastLED.setBrightness(LOW_BATTERY_BRIGHTNESS);
      DEBUG_PRINT("Brightness mode set to LOW_BATTERY: ");
      DEBUG_PRINTLN(LOW_BATTERY_BRIGHTNESS);
      break;
      
    case BRIGHTNESS_SLEEP:
      FastLED.setBrightness(DIM_BEFORE_SLEEP);
      DEBUG_PRINT("Brightness mode set to SLEEP: ");
      DEBUG_PRINTLN(DIM_BEFORE_SLEEP);
      break;
  }
  
  currentBrightnessMode = mode;
}

void LEDController::restorePreviousBrightness() {
  FastLED.setBrightness(savedBrightness);
  normalBrightness = savedBrightness;
  config->brightness = savedBrightness;
  currentBrightnessMode = BRIGHTNESS_NORMAL;
  
  DEBUG_PRINT("Restored previous brightness: ");
  DEBUG_PRINTLN(savedBrightness);
}

uint8_t LEDController::getCurrentBrightness() {
  return FastLED.getBrightness();
}

// Force a complete refresh of the LED strips
void LEDController::forceRefresh() {
  // Clear all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, CRGB::Black);
  safeStripRefresh();
  
  // Reset effect step counter
  effectStep = 0;
  
  // Ensure impactEffectActive is reset
  impactEffectActive = false;
  
  // Set brightness to correct value
  FastLED.setBrightness(normalBrightness);
  
  DEBUG_PRINTLN("LED strip forcefully refreshed");
}

// EFFECT IMPLEMENTATIONS

// Slow Rainbow Effect (renamed from updateSolidEffect)
void LEDController::updateSlowRainbowEffect() {
  // Use configuration settings
  uint8_t hueChangeRate = SLOW_RAINBOW_HUE_CHANGE_RATE;
  
  // Create color based on configuration
  CRGB color;
  
  if (SLOW_RAINBOW_USE_HUE_SHIFT) {
    // Slowly changing hue
    color = CHSV(effectStep / hueChangeRate, 255, 255);
  } else {
    // Fixed color (fallback)
    color = CRGB::Red;
  }
  
  // Apply the same color to all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, color);
}

// Solid Blue Effect
void LEDController::updateSolidBlueEffect() {
  // Create the configured blue color
  CRGB color = SOLID_BLUE_COLOR;
  
  // Apply brightness scaling if configured differently than FastLED brightness
  if (SOLID_BLUE_BRIGHTNESS != 255) {
    color.fadeToBlackBy(255 - SOLID_BLUE_BRIGHTNESS);
  }
  
  // Apply the color to all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, color);
}

// Solid Green Effect
void LEDController::updateSolidGreenEffect() {
  // Create the configured green color
  CRGB color = SOLID_GREEN_COLOR;
  
  // Apply brightness scaling if configured differently than FastLED brightness
  if (SOLID_GREEN_BRIGHTNESS != 255) {
    color.fadeToBlackBy(255 - SOLID_GREEN_BRIGHTNESS);
  }
  
  // Apply the color to all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, color);
}

// Solid Red Effect
void LEDController::updateSolidRedEffect() {
  // Create the configured red color
  CRGB color = SOLID_RED_COLOR;
  
  // Apply brightness scaling if configured differently than FastLED brightness
  if (SOLID_RED_BRIGHTNESS != 255) {
    color.fadeToBlackBy(255 - SOLID_RED_BRIGHTNESS);
  }
  
  // Apply the color to all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, color);
}

// Solid Purple Effect
void LEDController::updateSolidPurpleEffect() {
  // Create the configured purple color
  CRGB color = SOLID_PURPLE_COLOR;
  
  // Apply brightness scaling if configured differently than FastLED brightness
  if (SOLID_PURPLE_BRIGHTNESS != 255) {
    color.fadeToBlackBy(255 - SOLID_PURPLE_BRIGHTNESS);
  }
  
  // Apply the color to all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, color);
}

// Solid Yellow Effect
void LEDController::updateSolidYellowEffect() {
  // Create the configured yellow color
  CRGB color = SOLID_YELLOW_COLOR;
  
  // Apply brightness scaling if configured differently than FastLED brightness
  if (SOLID_YELLOW_BRIGHTNESS != 255) {
    color.fadeToBlackBy(255 - SOLID_YELLOW_BRIGHTNESS);
  }
  
  // Apply the color to all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, color);
}

// Solid White Effect
void LEDController::updateSolidWhiteEffect() {
  // Create the configured white color
  CRGB color = SOLID_WHITE_COLOR;
  
  // Apply brightness scaling if configured differently than FastLED brightness
  if (SOLID_WHITE_BRIGHTNESS != 255) {
    color.fadeToBlackBy(255 - SOLID_WHITE_BRIGHTNESS);
  }
  
  // Apply the color to all LEDs
  fill_solid(leds, NUM_LEDS_TOTAL, color);
}
