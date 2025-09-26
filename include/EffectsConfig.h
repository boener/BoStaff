#ifndef EFFECTS_CONFIG_H
#define EFFECTS_CONFIG_H

#include <FastLED.h>

// ---------------------------------------------------------------------------
// SERIAL DEBUG CONTROL SYSTEM
// ---------------------------------------------------------------------------

// MASTER DEBUG FLAG - Comment out this line to disable ALL serial debugging for maximum performance
// #define ENABLE_SERIAL_DEBUG  // Uncomment this line to enable serial debugging

// Debug output macros - these completely eliminate debug code when ENABLE_SERIAL_DEBUG is not defined
#ifdef ENABLE_SERIAL_DEBUG //The IF in this ifDEFINED is whether the ENABLE_SERIAL_DEBUG aboive is commented out or not
  #define DEBUG_PRINT(x)         Serial.print(x)
  #define DEBUG_PRINTLN(x)       Serial.println(x)
  #define DEBUG_PRINTF(...)      Serial.printf(__VA_ARGS__)
  #define DEBUG_BEGIN(x)         Serial.begin(x)
  #define DEBUG_PRINT_F(x)       Serial.print(F(x))
  #define DEBUG_PRINTLN_F(x)     Serial.println(F(x))
#else
  #define DEBUG_PRINT(x)         // No-op when debugging disabled
  #define DEBUG_PRINTLN(x)       // No-op when debugging disabled  
  #define DEBUG_PRINTF(...)      // No-op when debugging disabled
  #define DEBUG_BEGIN(x)         // No-op when debugging disabled
  #define DEBUG_PRINT_F(x)       // No-op when debugging disabled
  #define DEBUG_PRINTLN_F(x)     // No-op when debugging disabled
#endif

// Performance-critical debug flag for high-frequency operations (like accelerometer updates)
// This can be disabled separately for ultra-high performance while keeping other debug output
// #define ENABLE_PERFORMANCE_DEBUG // Uncomment this line to enable performance-critical debug output
// Note: This is separate from ENABLE_SERIAL_DEBUG above to allow high-frequency debug without slowing down other operations

#ifdef ENABLE_PERFORMANCE_DEBUG
  #define PERF_DEBUG_PRINT(x)    DEBUG_PRINT(x)
  #define PERF_DEBUG_PRINTLN(x)  DEBUG_PRINTLN(x)
#else
  #define PERF_DEBUG_PRINT(x)    // No-op when performance debugging disabled
  #define PERF_DEBUG_PRINTLN(x)  // No-op when performance debugging disabled
#endif

// ---------------------------------------------------------------------------
// BRIGHTNESS SETTINGS
// ---------------------------------------------------------------------------

// Global brightness settings (0-255)
#define DEFAULT_BRIGHTNESS 45         // Normal operating brightness
#define LOW_BATTERY_BRIGHTNESS 25     // Brightness when battery is low
#define DIM_BEFORE_SLEEP 5            // Dim level before going to sleep

// ---------------------------------------------------------------------------
// IMPACT EFFECT SETTINGS
// ---------------------------------------------------------------------------

// Impact detection and effect settings
#define IMPACT_BRIGHTNESS 150         // Brightness when impact detected (0-255)
#define IMPACT_FLASH_DURATION 100     // Duration of impact flash in milliseconds

// OLD SINGLE-SENSOR THRESHOLD - REPLACED WITH DUAL-SENSOR SYSTEM
// #define IMPACT_THRESHOLD 1600         // Default impact detection threshold (~1.6G)

// NEW DUAL-SENSOR IMPACT DETECTION THRESHOLDS
#define IMPACT_ACCEL_THRESHOLD 3500    // Total magnitude threshold for side swings (orig 4500)
#define IMPACT_INDIVIDUAL_AXIS_THRESHOLD 2000  // Individual axis threshold for straight stabs (orig 2700)
#define IMPACT_GYRO_DELTA_THRESHOLD 600   // 4.0 rad/s change in raw format (gyroscope delta threshold for sudden rotation changes)
#define ROTATION_CLASSIFICATION_THRESHOLD 730  // 6.3 rad/s in raw format (rotation vs stab classification)

#define IMPACT_COOLDOWN 330           // Minimum time between impacts in milliseconds - Was 500
#define IMPACT_COLOR CRGB::BlueViolet // Color of the impact flash (dimmed white)
#define IMPACT_FADE_OUT true          // Whether to fade out after impact (true) or cut off (false)
#define IMPACT_FADE_RATE 10           // How quickly impact effect fades (higher = faster fade)

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
#define STROBE_OFF_TIME 2             // Counts the strobe is OFF, about 15ms each
#define STROBE_COLOR CRGB::White      // Color of strobe effect
#define STROBE_FADE_OUT true          // Whether strobe fades out (true) or cuts off (false)
#define STROBE_FADE_RATE 1            // How quickly strobe fades (higher = faster fade)
#define STROBE_BRIGHTNESS 200         // Brightness of the strobe effect (0-255)

// ---------------------------------------------------------------------------
// SLOW RAINBOW EFFECT SETTINGS (renamed from SOLID COLOR EFFECT)
// ---------------------------------------------------------------------------

// Slow rainbow effect creates a gently shifting solid color through all hues
#define SLOW_RAINBOW_HUE_CHANGE_RATE 1  // How quickly hue changes (higher = slower change)
#define SLOW_RAINBOW_USE_HUE_SHIFT true // Whether to shift hue (true) or stay fixed (false)

// ---------------------------------------------------------------------------
// SOLID COLOR EFFECT SETTINGS (NEW - for true solid colors)
// ---------------------------------------------------------------------------

// Solid color effects - these are completely static colors with configurable brightness
// Each solid color can have its brightness adjusted independently

// Solid Blue Effect
#define SOLID_BLUE_COLOR CRGB::Blue     // Color for solid blue effect
#define SOLID_BLUE_BRIGHTNESS 255       // Brightness for solid blue effect (0-255)

// Solid Green Effect  
#define SOLID_GREEN_COLOR CRGB::Green   // Color for solid green effect
#define SOLID_GREEN_BRIGHTNESS 255      // Brightness for solid green effect (0-255)

// Solid Red Effect
#define SOLID_RED_COLOR CRGB::Red       // Color for solid red effect
#define SOLID_RED_BRIGHTNESS 255        // Brightness for solid red effect (0-255)

// Solid Purple Effect
#define SOLID_PURPLE_COLOR CRGB::Purple // Color for solid purple effect
#define SOLID_PURPLE_BRIGHTNESS 255     // Brightness for solid purple effect (0-255)

// Solid Yellow Effect
#define SOLID_YELLOW_COLOR CRGB::Yellow // Color for solid yellow effect
#define SOLID_YELLOW_BRIGHTNESS 255     // Brightness for solid yellow effect (0-255)

// Solid White Effect
#define SOLID_WHITE_COLOR CRGB::White   // Color for solid white effect
#define SOLID_WHITE_BRIGHTNESS 255      // Brightness for solid white effect (0-255)

// ---------------------------------------------------------------------------
// POWER MANAGEMENT SETTINGS
// ---------------------------------------------------------------------------

// Timing for activity detection and sleep mode
#define ACTIVITY_TIMEOUT_MINS 30      // Minutes of inactivity before sleep
#define FADE_TO_SLEEP_DURATION 1000   // Time in ms to fade to sleep
#define BATTERY_CHECK_INTERVAL 300000 // Time between battery checks (ms)

#endif // EFFECTS_CONFIG_H