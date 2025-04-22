#include "BoStaff.h"
#include "Effects/FireEffect.h"
#include "EffectsConfig.h"

FireEffect::FireEffect(LEDController* ledController, int segmentLen) : 
  ledArray(nullptr), numLedsTotal(0), segmentLength(segmentLen), heat(nullptr), 
  cooling(FIRE_COOLING), sparking(FIRE_SPARKING), initialized(false), controller(ledController) {
  
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

FireEffect::~FireEffect() {
  if (heat) {
    delete[] heat;
    heat = nullptr;  // Prevent double deletion
  }
  initialized = false;
  ledArray = nullptr; // Don't delete ledArray as it's managed elsewhere
}

bool FireEffect::isInitialized() const {
  return initialized && heat != nullptr && ledArray != nullptr && controller != nullptr;
}

void FireEffect::setCooling(uint8_t cool) {
  cooling = cool;
}

void FireEffect::setSparking(uint8_t spark) {
  sparking = spark;
}

void FireEffect::update() {
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

void FireEffect::updateSegment(int segmentIndex) {
  // Get the starting index for this segment's heat array
  int heatOffset = segmentIndex * segmentLength;
  
  // Step 1: Cool down every cell a little
  for (int i = 0; i < segmentLength; i++) {
    heat[heatOffset + i] = qsub8(heat[heatOffset + i], 
                                random8(0, ((cooling * 10) / segmentLength) + 2));
  }

  // Step 2: Heat from each cell drifts 'up' and diffuses
  // Using the config setting for heat dissipation
  for (int k = segmentLength - 1; k >= 2; k--) {
    heat[heatOffset + k] = (heat[heatOffset + k - 1] + 
                           heat[heatOffset + k - 2] + 
                           heat[heatOffset + k - 2]) / FIRE_HEAT_DISSIPATION;
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