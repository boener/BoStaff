#ifndef VERSION_H
#define VERSION_H

// Version information for BoStaff project
#define VERSION "0.1.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// Feature flags
#define FEATURE_FIRE_EFFECT 1
#define FEATURE_PULSE_EFFECT 1
#define FEATURE_RAINBOW_EFFECT 1
#define FEATURE_STROBE_EFFECT 1

// Hardware configuration
#define HW_VERSION "1.0"
#define DEFAULT_BRIGHTNESS 150

// LED configuration
#define LED_PIN_1 D1  // GPIO5
#define LED_PIN_2 D2  // GPIO4
#define LED_COUNT_PER_STRIP 200
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Button configuration
#define BUTTON_PIN D6  // GPIO12
#define BUTTON_ACTIVE_LOW true
#define BUTTON_DEBOUNCE_MS 50

// Accelerometer configuration
#define MPU_I2C_SDA D2  // GPIO4
#define MPU_I2C_SCL D1  // GPIO5
#define IMPACT_THRESHOLD 8000
#define IMPACT_FLASH_DURATION_MS 100
#define IMPACT_COOLDOWN_MS 500

// Power settings
#define POWER_SAVING_MODE 1
#define SLEEP_AFTER_MINS 30

#endif // VERSION_H