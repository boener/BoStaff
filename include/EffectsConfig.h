#ifndef EFFECTS_CONFIG_H
#define EFFECTS_CONFIG_H

#include <FastLED.h>

// ---------------------------------------------------------------------------
// BRIGHTNESS SETTINGS
// ---------------------------------------------------------------------------

// Global brightness settings (0-255)
#define DEFAULT_BRIGHTNESS 25         // Normal operating brightness
#define IMPACT_BRIGHTNESS 150         // Brightness when impact detected
#define LOW_BATTERY_BRIGHTNESS 15     // Brightness when battery is low
#define DIM_BEFORE_SLEEP 5            // Dim level before going to sleep

// ---------------------------------------------------------------------------
// FIRE EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Fire effect creates a realistic fire simulation
#define FIRE_COOLING 85               // How quickly fire cools down (higher = faster cooling)
#define FIRE_SPARKING 90              // How many sparks are created (higher = more sparks)
#define FIRE_HEAT_DISSIPATION 3       // How heat dissipates (used in heat averaging formula)
#define FIRE_BASE_COLOR CRGB::OrangeRed // Base color for fire effect

// ---------------------------------------------------------------------------
// PULSE EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Pulse effect creates a smooth pulsing wave
#define PULSE_SPEED 15                // Speed of pulse animation (higher = faster)
#define PULSE_WIDTH 50                // Width of the pulse (1-255)
#define PULSE_COLOR CRGB::Blue        // Default color of pulse effect
#define PULSE_FADE_RATE 5             // How quickly pulse fades (higher = faster fade)
#define PULSE_MIN_BRIGHTNESS 20       // Minimum brightness during pulse (0-255)
#define PULSE_MAX_BRIGHTNESS 200      // Maximum brightness during pulse (0-255)

// ---------------------------------------------------------------------------
// RAINBOW EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Rainbow effect creates a moving rainbow pattern
#define RAINBOW_SPEED 10              // Speed of rainbow animation (higher = faster)
#define RAINBOW_DELTA 5               // Hue change per LED (higher = more compressed rainbow)
#define RAINBOW_SATURATION 240        // Saturation of colors (0-255)
#define RAINBOW_BRIGHTNESS 255        // Value/brightness of colors (0-255)

// ---------------------------------------------------------------------------
// STROBE EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Strobe effect creates a flashing strobe light
#define STROBE_ON_TIME 50             // Milliseconds the strobe is ON
#define STROBE_OFF_TIME 100           // Milliseconds the strobe is OFF
#define STROBE_COLOR CRGB::White      // Color of strobe effect
#define STROBE_FADE_OUT true          // Whether strobe fades out (true) or cuts off (false)
#define STROBE_FADE_RATE 20           // How quickly strobe fades (higher = faster fade)

// ---------------------------------------------------------------------------
// SOLID COLOR EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Solid effect creates a gently shifting solid color
#define SOLID_HUE_CHANGE_RATE 2       // How quickly hue changes (higher = faster change)
#define SOLID_USE_HUE_SHIFT true      // Whether to shift hue (true) or stay fixed (false)

// ---------------------------------------------------------------------------
// POWER MANAGEMENT SETTINGS
// ---------------------------------------------------------------------------

// Timing for activity detection and sleep mode
#define ACTIVITY_TIMEOUT_MINS 30      // Minutes of inactivity before sleep
#define FADE_TO_SLEEP_DURATION 1000   // Time in ms to fade to sleep
#define BATTERY_CHECK_INTERVAL 300000 // Time between battery checks (ms)

#endif // EFFECTS_CONFIG_H