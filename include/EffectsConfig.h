#ifndef EFFECTS_CONFIG_H
#define EFFECTS_CONFIG_H

#include <FastLED.h>

// ---------------------------------------------------------------------------
// BRIGHTNESS SETTINGS
// ---------------------------------------------------------------------------

// Global brightness settings (0-255)
#define DEFAULT_BRIGHTNESS 45         // Normal operating brightness
#define IMPACT_BRIGHTNESS 150         // Brightness when impact detected
#define LOW_BATTERY_BRIGHTNESS 25     // Brightness when battery is low
#define DIM_BEFORE_SLEEP 5            // Dim level before going to sleep

// ---------------------------------------------------------------------------
// FIRE EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Fire effect creates a realistic fire simulation
#define FIRE_COOLING 65               // How quickly fire cools down (higher = faster cooling)
#define FIRE_SPARKING 90              // How many sparks are created (higher = more sparks)
#define FIRE_HEAT_DISSIPATION 3       // How heat dissipates (used in heat averaging formula)
#define FIRE_BASE_COLOR CRGB::OrangeRed // Base color for fire effect

// ---------------------------------------------------------------------------
// FIRE EFFECT 1 SETTINGS
// ---------------------------------------------------------------------------

// Fire effect 1 - creates a blue fire simulation
#define FIRE1_COOLING 65              // How quickly fire cools down (higher = faster cooling)
#define FIRE1_SPARKING 90             // How many sparks are created (higher = more sparks)
#define FIRE1_HEAT_DISSIPATION 3      // How heat dissipates (used in heat averaging formula)
#define FIRE1_BASE_COLOR CRGB::DarkGreen   // Base color for fire effect 1

// ---------------------------------------------------------------------------
// FIRE EFFECT 2 SETTINGS
// ---------------------------------------------------------------------------

// Fire effect 2 - creates a green fire simulation
#define FIRE2_COOLING 65              // How quickly fire cools down (higher = faster cooling)
#define FIRE2_SPARKING 90            // How many sparks are created (higher = more sparks)
#define FIRE2_HEAT_DISSIPATION 3      // How heat dissipates (used in heat averaging formula)
#define FIRE2_BASE_COLOR CRGB::DarkBlue  // Base color for fire effect 2

// ---------------------------------------------------------------------------
// PULSE EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Pulse effect creates a smooth pulsing wave
#define PULSE_SPEED 200                // Speed of pulse animation (higher = faster)
#define PULSE_WIDTH 75                // Width of the pulse (1-255 higher = shorter pulses)
#define PULSE_COLOR CRGB::Blue        // Default color of pulse effect
#define PULSE_FADE_RATE 50             // How quickly pulse fades (higher = faster fade)
#define PULSE_MIN_BRIGHTNESS 20       // Minimum brightness during pulse (0-255)
#define PULSE_MAX_BRIGHTNESS 200      // Maximum brightness during pulse (0-255)

// ---------------------------------------------------------------------------
// PULSE EFFECT 1 SETTINGS
// ---------------------------------------------------------------------------

// Pulse effect 1 - creates a faster, more colorful pulsing wave
#define PULSE1_SPEED 200               // Speed of pulse animation (higher = faster)
#define PULSE1_WIDTH 75                // Width of the pulse (1-255, higher = shorter pulses)
#define PULSE1_COLOR CRGB::Green       // Default color of pulse effect 1
#define PULSE1_FADE_RATE 50            // How quickly pulse fades (higher = faster fade)
#define PULSE1_MIN_BRIGHTNESS 20       // Minimum brightness during pulse (0-255)
#define PULSE1_MAX_BRIGHTNESS 200      // Maximum brightness during pulse (0-255)

// ---------------------------------------------------------------------------
// PULSE EFFECT 2 SETTINGS
// ---------------------------------------------------------------------------

// Pulse effect 2 - creates a double-pulse wave pattern
#define PULSE2_SPEED 200               // Speed of pulse animation (higher = faster)
#define PULSE2_WIDTH 75                // Width of the pulse (1-255, higher = shorter pulses)
#define PULSE2_COLOR CRGB::Red         // Default color of pulse effect 2
#define PULSE2_FADE_RATE 50            // How quickly pulse fades (higher = faster fade)
#define PULSE2_MIN_BRIGHTNESS 20       // Minimum brightness during pulse (0-255)
#define PULSE2_MAX_BRIGHTNESS 200      // Maximum brightness during pulse (0-255)

// ---------------------------------------------------------------------------
// RAINBOW EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Rainbow effect creates a moving rainbow pattern
#define RAINBOW_SPEED 55              // Speed of rainbow animation (higher = faster)
#define RAINBOW_DELTA 5               // Hue change per LED (higher = more compressed rainbow)
#define RAINBOW_SATURATION 240        // Saturation of colors (0-255)
#define RAINBOW_BRIGHTNESS 255        // Value/brightness of colors (0-255)

// ---------------------------------------------------------------------------
// STROBE EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Strobe effect creates a flashing strobe light
#define STROBE_ON_TIME 2              // Counts the strobe is ON, about 15ms each
#define STROBE_OFF_TIME 3             // Counts the strobe is OFF, about 15ms each
#define STROBE_COLOR CRGB::White      // Color of strobe effect
#define STROBE_FADE_OUT true          // Whether strobe fades out (true) or cuts off (false)
#define STROBE_FADE_RATE 1            // How quickly strobe fades (higher = faster fade)
#define STROBE_BRIGHTNESS 150         // Brightness of the strobe effect (0-255)

// ---------------------------------------------------------------------------
// SOLID COLOR EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Solid effect creates a gently shifting solid color - settings are weirs, change rate at min is way to slow, WHY?
#define SOLID_HUE_CHANGE_RATE 1       // How quickly hue changes (higher = slower change) FIX THIS, 1 is still too slow!
#define SOLID_USE_HUE_SHIFT true      // Whether to shift hue (true) or stay fixed (false)

// ---------------------------------------------------------------------------
// POWER MANAGEMENT SETTINGS
// ---------------------------------------------------------------------------

// Timing for activity detection and sleep mode
#define ACTIVITY_TIMEOUT_MINS 30      // Minutes of inactivity before sleep
#define FADE_TO_SLEEP_DURATION 1000   // Time in ms to fade to sleep
#define BATTERY_CHECK_INTERVAL 300000 // Time between battery checks (ms)

#endif // EFFECTS_CONFIG_H