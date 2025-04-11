#ifndef EFFECTS_H
#define EFFECTS_H

// Include all effect implementations
#include "../src/Effects/FireEffect.h"
#include "../src/Effects/PulseEffect.h"
#include "../src/Effects/RainbowEffect.h"
#include "../src/Effects/StrobeEffect.h"

// Effect type enum for better code readability
enum EffectType {
  EFFECT_SOLID = 0,
  EFFECT_FIRE = 1,
  EFFECT_PULSE = 2,
  EFFECT_RAINBOW = 3,
  EFFECT_STROBE = 4,
  NUM_EFFECTS
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