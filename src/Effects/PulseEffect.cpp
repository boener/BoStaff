#include "BoStaff.h"
#include "Effects/PulseEffect.h"
#include "EffectsConfig.h" // Added include for configuration parameters

PulseEffect::PulseEffect(LEDController* ledController, int segmentLen) : 
  ledArray(nullptr), numLedsTotal(0), segmentLength(segmentLen), 
  baseHue(0), initialized(false), controller(ledController) {
  
  // Initialize values from EffectsConfig.h
  speed = PULSE_SPEED;
  pulseWidth = PULSE_WIDTH;
  fadeRate = PULSE_FADE_RATE;
  minBrightness = PULSE_MIN_BRIGHTNESS;
  maxBrightness = PULSE_MAX_BRIGHTNESS;
  
  // Extract hue from PULSE_COLOR for base color
  CHSV pulseColorHSV = rgb2hsv_approximate(PULSE_COLOR);
  baseHue = pulseColorHSV.hue;
  
  // Default to 1 wave
  waveCount = 1;
  
  // Validate inputs
  if (!ledController) {
    Serial.println("ERROR: PulseEffect created with invalid parameters");
    return;
  }
  
  ledArray = ledController->getLeds();
  numLedsTotal = NUM_LEDS_TOTAL; // Use the global constant
  initialized = true;
  
  Serial.println("PulseEffect initialized with single-strip approach");
  Serial.print("Pulse Effect Config - Speed: ");
  Serial.print(speed);
  Serial.print(", Width: ");
  Serial.print(pulseWidth);
  Serial.print(", Color: ");
  Serial.println(baseHue);
}

PulseEffect::~PulseEffect() {
  // No dynamic memory to free, but we still need to reset state
  initialized = false;
  ledArray = nullptr; // Don't delete ledArray as it's managed elsewhere
  controller = nullptr;
}

bool PulseEffect::isInitialized() const {
  return initialized && ledArray != nullptr && controller != nullptr;
}

void PulseEffect::setHue(uint8_t newHue) {
  baseHue = newHue;
}

void PulseEffect::setSpeed(uint8_t newSpeed) {
  speed = newSpeed;
}

void PulseEffect::setPulseWidth(uint8_t width) {
  pulseWidth = constrain(width, 1, 255);
}

void PulseEffect::setWaveCount(uint8_t count) {
  if (count > 0 && count <= 5) { // Reasonable bounds
    waveCount = count;
  }
}

void PulseEffect::update() {
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
  
  // Slowly change the base hue for variation - use fadeRate from config
  EVERY_N_MILLISECONDS(50) {
    baseHue += 1; // Slow hue shift
  }
}

void PulseEffect::updateSegment(int segmentIndex) {
  for (int i = 0; i < segmentLength; i++) {
    // Calculate distance from end (0 = far end, 99 = center/hilt)
    // MODIFIED: Reversed to make pulses go from center to ends
    uint8_t distanceFromCenter = (segmentLength - 1) - i;
    
    // Create multiple sine waves with different frequencies
    // Creates a pulse that travels outward from the center
    uint16_t brightness = 0;
    
    for (uint8_t w = 1; w <= waveCount; w++) {
      uint8_t divisor = (w > 1) ? w : uint8_t(1);
      // Use speed from config for beat speed
      uint8_t beatSpeed = speed * w;
      // Use pulseWidth to influence beat width
      uint8_t beatOffset = distanceFromCenter * pulseWidth / 10;
      
      // Calculate brightness range based on min/max brightness from config
      uint8_t brightnessRange = maxBrightness - minBrightness;
      uint8_t b = beatsin8(beatSpeed, 0, brightnessRange / divisor, 0, beatOffset);
      brightness += b + minBrightness;
    }
    
    // Cap the brightness at 255
    brightness = (brightness > uint16_t(255)) ? uint16_t(255) : brightness;
    
    // Calculate hue variation based on distance from center
    uint8_t hueVar = baseHue + (distanceFromCenter / 4); // Softer hue transition
    
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