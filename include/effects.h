#ifndef EFFECTS_H
#define EFFECTS_H

// Forward declaration of the LEDController class
class LEDController;

// Include all effect implementations
#include "../src/Effects/FireEffect.h"
#include "../src/Effects/FireEffect1.h"
#include "../src/Effects/FireEffect2.h"
#include "../src/Effects/PulseEffect.h"
#include "../src/Effects/PulseEffect1.h"
#include "../src/Effects/PulseEffect2.h"
#include "../src/Effects/RainbowEffect.h"
#include "../src/Effects/StrobeEffect.h"

// Effect type enum for better code readability
enum EffectType {
  EFFECT_SLOW_RAINBOW = 0,  // Renamed from EFFECT_SOLID to EFFECT_SLOW_RAINBOW
  EFFECT_SOLID_BLUE = 1,    // New solid blue effect
  EFFECT_SOLID_GREEN = 2,   // New solid green effect
  EFFECT_SOLID_RED = 3,     // New solid red effect
  EFFECT_SOLID_PURPLE = 4,  // New solid purple effect
  EFFECT_SOLID_YELLOW = 5,  // New solid yellow effect
  EFFECT_SOLID_WHITE = 6,   // New solid white effect
  EFFECT_FIRE = 7,          // Updated index
  EFFECT_FIRE1 = 8,         // Updated index
  EFFECT_FIRE2 = 9,         // Updated index
  EFFECT_PULSE = 10,        // Updated index
  EFFECT_PULSE1 = 11,       // Updated index
  EFFECT_PULSE2 = 12,       // Updated index
  EFFECT_RAINBOW = 13,      // Updated index
  EFFECT_STROBE = 14,       // Updated index
  NUM_EFFECTS = 15          // Updated total count
};

// Helper struct to store effect parameters
struct EffectParams {
  uint8_t brightness = 25;  // Reduced from 150 to 25 (~10%)
  uint8_t speed = 128;
  uint8_t intensity = 128;
  uint8_t param1 = 128;  // Custom parameter 1
  uint8_t param2 = 128;  // Custom parameter 2
  CRGB color = CRGB::Red;
};

// Effect names for display/debugging
// Added 'extern' to avoid multiple definitions
extern const char* EFFECT_NAMES[];

#endif // EFFECTS_H
