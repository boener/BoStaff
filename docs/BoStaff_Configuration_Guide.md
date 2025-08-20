# BoStaff LED Controller - Configuration Guide

## Overview
This guide explains the pin assignments, configuration variables, and debug system for the ESP8266-based BoStaff LED controller project.

---

## Pin Assignments (ESP8266)

### LED Strip
- **Pin D7 (GPIO13)**: Single LED strip output
  - Controls 400 LEDs total
  - Arranged in 4 segments of 100 LEDs each

### Button Control
- **Pin D6 (GPIO12)**: Mode change button
  - Active LOW (press connects to ground)
  - Used for cycling through LED effects

### MPU6050 Accelerometer (I2C)
- **Pin D1 (GPIO5)**: SCL (I2C Clock)
- **Pin D2 (GPIO4)**: SDA (I2C Data)
- **I2C Settings**:
  - Clock Speed: 100 kHz (standard)
  - Timeout: 50ms
  - Retry Count: 3 attempts

### Power Monitoring
- **Pin A0**: Battery voltage monitoring
  - Uses voltage divider (ratio: 2)
  - Range: 3.3V (minimum) to 4.2V (maximum)

---

## LED Strip Configuration

### Physical Layout
The staff uses a single 400-LED strip folded into 4 segments:

```
Segment 1: LEDs 0-99     (counts up: 0→99)
Segment 2: LEDs 100-199  (counts down: 199→100)  
Segment 3: LEDs 200-299  (counts up: 200→299)
Segment 4: LEDs 300-399  (counts down: 399→300)
```

### Configurable Variables
- `NUM_LEDS_TOTAL`: Total LEDs (400)
- `NUM_LEDS_SEGMENT`: LEDs per segment (100)

---

## Available LED Effects

### Effect Types (EffectsConfig.h)
1. **EFFECT_SOLID**: Solid color with optional hue shifting
2. **EFFECT_FIRE**: Orange/red fire simulation
3. **EFFECT_FIRE1**: Green fire simulation  
4. **EFFECT_FIRE2**: Blue fire simulation
5. **EFFECT_PULSE**: Blue pulsing wave
6. **EFFECT_PULSE1**: Green pulsing wave
7. **EFFECT_PULSE2**: Red pulsing wave
8. **EFFECT_RAINBOW**: Moving rainbow pattern
9. **EFFECT_STROBE**: White strobe light

---

## Brightness Settings (EffectsConfig.h)

### Global Brightness (0-255)
- `DEFAULT_BRIGHTNESS`: Normal operation (45)
- `LOW_BATTERY_BRIGHTNESS`: Low battery mode (25)
- `DIM_BEFORE_SLEEP`: Pre-sleep dimming (5)
- `IMPACT_BRIGHTNESS`: Impact flash brightness (150)

### Effect-Specific Brightness
- `PULSE_MIN_BRIGHTNESS`: Minimum pulse brightness (20)
- `PULSE_MAX_BRIGHTNESS`: Maximum pulse brightness (200)
- `STROBE_BRIGHTNESS`: Strobe effect brightness (200)

---

## Impact Detection System (Dual-Sensor)

### Thresholds (EffectsConfig.h)
- `IMPACT_ACCEL_THRESHOLD`: Total acceleration magnitude (4500)
- `IMPACT_INDIVIDUAL_AXIS_THRESHOLD`: Single axis threshold (2700)  
- `IMPACT_GYRO_DELTA_THRESHOLD`: Gyroscope sudden stop (600)
- `ROTATION_CLASSIFICATION_THRESHOLD`: Rotation vs stab detection (730)

### Impact Effect Settings
- `IMPACT_COOLDOWN`: Time between impacts (330ms)
- `IMPACT_FLASH_DURATION`: Flash duration (100ms)
- `IMPACT_COLOR`: Flash color (BlueViolet)
- `IMPACT_FADE_OUT`: Enable fade out (true)
- `IMPACT_FADE_RATE`: Fade speed (10)

---

## Fire Effect Settings (EffectsConfig.h)

### Fire Effect (Orange/Red)
- `FIRE_COOLING`: Cooling rate (65)
- `FIRE_SPARKING`: Spark generation (90)
- `FIRE_HEAT_DISSIPATION`: Heat spread rate (3)
- `FIRE_BASE_COLOR`: Base color (OrangeRed)

### Fire Effect 1 (Green)
- `FIRE1_COOLING`: Cooling rate (65)
- `FIRE1_SPARKING`: Spark generation (90)
- `FIRE1_HEAT_DISSIPATION`: Heat spread rate (3)
- `FIRE1_BASE_COLOR`: Base color (DarkGreen)

### Fire Effect 2 (Blue)
- `FIRE2_COOLING`: Cooling rate (65)
- `FIRE2_SPARKING`: Spark generation (90)
- `FIRE2_HEAT_DISSIPATION`: Heat spread rate (3)
- `FIRE2_BASE_COLOR`: Base color (DarkBlue)

---

## Pulse Effect Settings (EffectsConfig.h)

### Pulse Effect (Blue)
- `PULSE_SPEED`: Animation speed (200)
- `PULSE_WIDTH`: Pulse width (75)
- `PULSE_COLOR`: Color (Blue)
- `PULSE_FADE_RATE`: Fade speed (50)

### Pulse Effect 1 (Green)
- `PULSE1_SPEED`: Animation speed (200)
- `PULSE1_WIDTH`: Pulse width (75)
- `PULSE1_COLOR`: Color (Green)
- `PULSE1_FADE_RATE`: Fade speed (50)

### Pulse Effect 2 (Red)
- `PULSE2_SPEED`: Animation speed (200)
- `PULSE2_WIDTH`: Pulse width (75)
- `PULSE2_COLOR`: Color (Red)
- `PULSE2_FADE_RATE`: Fade speed (50)

---

## Rainbow Effect Settings (EffectsConfig.h)
- `RAINBOW_SPEED`: Animation speed (55)
- `RAINBOW_DELTA`: Hue change per LED (5)
- `RAINBOW_SATURATION`: Color saturation (240)
- `RAINBOW_BRIGHTNESS`: Color brightness (255)

---

## Strobe Effect Settings (EffectsConfig.h)
- `STROBE_ON_TIME`: On duration in cycles (2)
- `STROBE_OFF_TIME`: Off duration in cycles (2)
- `STROBE_COLOR`: Strobe color (White)
- `STROBE_FADE_OUT`: Enable fade out (true)
- `STROBE_FADE_RATE`: Fade speed (1)

---

## Solid Color Effect Settings (EffectsConfig.h)
- `SOLID_HUE_CHANGE_RATE`: Hue shift speed (1)
- `SOLID_USE_HUE_SHIFT`: Enable hue shifting (true)

---

## Power Management Settings (EffectsConfig.h)

### Timing Settings
- `ACTIVITY_TIMEOUT_MINS`: Inactivity timeout (30 minutes)
- `FADE_TO_SLEEP_DURATION`: Sleep fade time (1000ms)
- `BATTERY_CHECK_INTERVAL`: Battery check frequency (300000ms = 5 minutes)

### Battery Monitoring (hardware.h)
- `BATTERY_MIN_VOLTAGE`: Minimum safe voltage (3.3V)
- `BATTERY_MAX_VOLTAGE`: Maximum charge voltage (4.2V)
- `BATTERY_DIVIDER`: Voltage divider ratio (2)

---

## Debug System (EffectsConfig.h)

### How to Control Debug Output

The debug system uses preprocessor macros that can be completely disabled for maximum performance.

#### Main Debug Control
```cpp
// MASTER DEBUG FLAG - Comment out this line to disable ALL serial debugging
#define ENABLE_SERIAL_DEBUG  // Uncomment this line to enable serial debugging
```

**To ENABLE debugging**: Uncomment the line (remove the `//`)
**To DISABLE debugging**: Comment out the line (add `//` at the beginning)

#### Performance Debug Control
```cpp
// Performance-critical debug flag for high-frequency operations
#define ENABLE_PERFORMANCE_DEBUG // Uncomment this line to enable performance debug
```

This controls high-frequency debug output (like accelerometer readings) separately from general debug output.

#### Debug Macros Available
When debugging is enabled, these macros are available:
- `DEBUG_PRINT(x)`: Print without newline
- `DEBUG_PRINTLN(x)`: Print with newline
- `DEBUG_PRINTF(...)`: Formatted print
- `DEBUG_PRINT_F(x)`: Print from flash memory
- `DEBUG_PRINTLN_F(x)`: Print from flash with newline
- `PERF_DEBUG_PRINT(x)`: Performance-critical print
- `PERF_DEBUG_PRINTLN(x)`: Performance-critical print with newline

#### Serial Settings
- **Baud Rate**: 115200 (defined by `SERIAL_BAUD` in hardware.h)

#### What Gets Debugged
- **Startup**: Initialization messages, pin configurations, version info
- **Mode Changes**: Effect switching, button presses
- **Impact Detection**: Accelerometer readings, impact classifications
- **Power Management**: Battery voltage, sleep mode transitions
- **I2C Communication**: MPU6050 errors, recovery attempts
- **Performance**: High-frequency sensor data (when PERF debug enabled)

#### Performance Impact
- **Debug DISABLED**: Zero performance impact, all debug code eliminated
- **Debug ENABLED**: Adds serial communication overhead
- **Performance Debug**: Additional overhead for high-frequency operations

---

## Timing Configuration

### Update Intervals (main.cpp)
- `ACCEL_UPDATE_INTERVAL`: Accelerometer reading frequency (40ms)
- `LED_UPDATE_INTERVAL`: LED effect update frequency (15ms)  
- `POWER_UPDATE_INTERVAL`: Power management update frequency (500ms)

### I2C Settings (BoStaff.h)
- `I2C_CLOCK_SPEED`: I2C bus speed (100000 Hz)
- `I2C_TIMEOUT`: I2C operation timeout (50ms)
- `I2C_RETRY_COUNT`: Number of retry attempts (3)

---

## Quick Reference: Most Common Changes

### To Change Brightness
Edit these values in `EffectsConfig.h`:
- `DEFAULT_BRIGHTNESS` (normal operation)
- `IMPACT_BRIGHTNESS` (impact flash)

### To Adjust Impact Sensitivity  
Edit these values in `EffectsConfig.h`:
- `IMPACT_ACCEL_THRESHOLD` (lower = more sensitive)
- `IMPACT_INDIVIDUAL_AXIS_THRESHOLD` (lower = more sensitive)

### To Enable/Disable Debug Output
In `EffectsConfig.h`, comment or uncomment:
- `#define ENABLE_SERIAL_DEBUG`
- `#define ENABLE_PERFORMANCE_DEBUG`

### To Change Effect Colors
Edit the `_COLOR` defines for each effect in `EffectsConfig.h`

### To Adjust Battery Thresholds
Edit these values in `hardware.h`:
- `BATTERY_MIN_VOLTAGE`
- `BATTERY_MAX_VOLTAGE`

---

## Notes
- All timing values are in milliseconds unless otherwise specified
- Color values use FastLED's CRGB color format
- Brightness values range from 0 (off) to 255 (maximum)
- Impact thresholds are in raw sensor units (multiply by 100 for m/s² equivalents)
- When debug is disabled, all debug code is completely removed at compile time for maximum performance
