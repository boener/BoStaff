#ifndef VERSION_H
#define VERSION_H

// Version information for BoStaff project
#define VERSION "0.1.3"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// Feature flags
#define FEATURE_FIRE_EFFECT 1
#define FEATURE_PULSE_EFFECT 1
#define FEATURE_RAINBOW_EFFECT 1
#define FEATURE_STROBE_EFFECT 1

// Hardware configuration
#define HW_VERSION "1.1"
#define DEFAULT_BRIGHTNESS 75   // Reduced from 150 to 75 (50%)
#define DEFAULT_IMPACT_BRIGHTNESS 100 // New setting for impact flash (lower than previous 255)

// LED configuration - UPDATED PIN ASSIGNMENTS
#define LED_PIN_1 D3  // GPIO0 - First LED strip (was D1/GPIO5)
#define LED_PIN_2 D4  // GPIO2 - Second LED strip (was D2/GPIO4)
#define LED_COUNT_PER_STRIP 200
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Button configuration
#define BUTTON_PIN D6  // GPIO12
#define BUTTON_ACTIVE_LOW true
#define BUTTON_DEBOUNCE_MS 50

// Accelerometer configuration
#define MPU_I2C_SDA D2  // GPIO4 - Default I2C data pin
#define MPU_I2C_SCL D1  // GPIO5 - Default I2C clock pin
#define IMPACT_THRESHOLD 1600   // Reduced from 8000 to 1600 for better sensitivity
#define IMPACT_FLASH_DURATION_MS 100
#define IMPACT_COOLDOWN_MS 500

// Power settings
#define POWER_SAVING_MODE 1
#define SLEEP_AFTER_MINS 30

#endif // VERSION_H