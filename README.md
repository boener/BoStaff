# LED Bo Staff Controller

This project is a simplified and focused firmware for an LED bo staff, based on components from the WLED project but streamlined for this specific use case.

## Hardware

- ESP8266MOD D1mini microcontroller
- WS2812B or similar addressable LED strips (400 LEDs total)
- MPU-6050 accelerometer for impact detection
- Single button for mode cycling
- Battery-powered with Li-ion batteries

## Features

- Multiple LED effects optimized for a bo staff
- Impact detection with the MPU-6050 accelerometer
- Single button control for cycling through effects
- Mode persistence across power cycles
- Power optimization for battery operation

## Physical Construction

The bo staff consists of:
- Two polycarbonate tubes (6 feet total length)
- Central aluminum housing for electronics
- 3D printed internal carrier for components
- 200 LEDs per section, with the LED strip routing up one side and down the other

## Pin Configuration

- D1 (GPIO5): First LED strand (200 LEDs)
- D2 (GPIO4): Second LED strand (200 LEDs)
- D6 (GPIO12): Button input (active low with pull-up)
- D2 (GPIO4): MPU-6050 SDA
- D1 (GPIO5): MPU-6050 SCL

## LED Effects

1. Fire effect - Realistic fire simulation
2. Energy pulse - Wave propagation from center
3. Solid colors - Basic color options
4. Rainbow cycle - Smooth transitioning colors
5. Strobe/flash effects - Attention-grabbing patterns

## Impact Detection

When an impact is detected by the accelerometer:
- Triggers a bright flash effect
- Returns to the previous effect after the flash
- Threshold is calibrated to avoid false positives

## Development Approach

This firmware is a highly modified and stripped-down version of WLED that:
1. Removes all network connectivity components
2. Removes web server functionality
3. Preserves only the necessary LED driving capabilities
4. Adds custom button handling for cycling through effects
5. Integrates MPU-6050 accelerometer for impact detection

## Credits

- Based on components from the [WLED project](https://github.com/Aircoookie/WLED)
- Uses the FastLED library
- Uses the MPU6050 library