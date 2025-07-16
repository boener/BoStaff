# Pin Assignments for Bo Staff Controller

## Overview

This document details the pin assignments for the Bo Staff LED controller, explaining which pins are used for each component and why they were chosen.

## Current Pin Assignment (Single LED Strip)

| Component | Pin | GPIO | Description |
|-----------|-----|------|-------------|
| LED Strip | D7 | GPIO13 | Data pin for single LED strip (400 LEDs) |
| Button | D6 | GPIO12 | Mode selection button (active LOW with pull-up) |
| MPU-6050 SCL | D1 | GPIO5 | I2C clock line for accelerometer/gyroscope |
| MPU-6050 SDA | D2 | GPIO4 | I2C data line for accelerometer/gyroscope |

## Rationale for Pin Selection

### I2C Pins for MPU-6050

We're using the default I2C pins on the ESP8266:
- **D1 (GPIO5)**: SCL (clock line)
- **D2 (GPIO4)**: SDA (data line)

Using the default I2C pins provides the following advantages:
1. Better compatibility with libraries that assume default I2C pins
2. Hardware I2C support for more reliable communication
3. Simpler code that doesn't require custom Wire initialization
4. Proven stability with the MPU-6050 module

### LED Strip Data Pin

We selected D7 (GPIO13) for the single LED strip because:
1. It's a general-purpose pin with no boot mode restrictions
2. It supports the high-speed data requirements of WS2812B LEDs
3. It's physically separated from the I2C pins to minimize interference
4. GPIO13 is reliable for continuous data output

### Button Pin

- **D6 (GPIO12)**: Mode selection button

This pin was selected because:
1. It supports internal pull-up resistors
2. It doesn't affect boot mode (unlike GPIO0 or GPIO2)
3. It's physically separated from other used pins on the board
4. Provides stable input readings with proper debouncing

## LED Strip Configuration

The single 400 LED strip is logically divided into 4 segments:
- **Segment 1**: LEDs 0-99 (physical position: up first half)
- **Segment 2**: LEDs 100-199 (physical position: down first half, folded)
- **Segment 3**: LEDs 200-299 (physical position: up second half)
- **Segment 4**: LEDs 300-399 (physical position: down second half, folded)

This folded arrangement allows for symmetrical effects while using a single continuous data line.

## Important Notes

### Power Distribution

With 400 LEDs, proper power distribution is critical:
1. Inject power every 100-150 LEDs to prevent voltage drop
2. Use thick gauge wire (18-20 AWG) for power lines
3. Place capacitors near power injection points
4. Consider the 3A (15W) power limit set in software

### I2C Configuration

The I2C bus is configured with:
- Clock speed: 100 kHz (standard mode for reliability)
- Timeout: 50ms (prevents lockups)
- Retry count: 3 attempts for failed operations

### Signal Integrity

1. Keep the LED data line as short as possible
2. Use a 470Ω resistor in series with the LED data line
3. Avoid running the LED data line parallel to power lines
4. Consider using a level shifter if experiencing signal issues

## Wiring Recommendations

1. **LED Data Line**: 
   - Add a 470Ω resistor between D7 and the LED strip data input
   - Keep wire length under 30cm if possible

2. **Power Supply**:
   - Place a 1000μF capacitor across power rails near the LED strip
   - Use separate power and ground wires for LED power injection

3. **MPU-6050 Module**:
   - Use short I2C wires (under 20cm)
   - Add 4.7kΩ pull-up resistors if not included on the module
   - Ensure solid mounting to prevent vibration-induced noise

4. **Button**:
   - Use a 10kΩ pull-up resistor to 3.3V
   - Add a small capacitor (0.1μF) across the button for hardware debouncing

## Troubleshooting Pin Issues

### LED Strip Not Working
- Verify 470Ω resistor is in place
- Check data line continuity with multimeter
- Ensure ground is common between ESP8266 and LED strip

### I2C Communication Errors
- Verify pull-up resistors are present
- Check wire length and quality
- Use I2C scanner sketch to verify MPU-6050 address

### Button Not Responding
- Check pull-up resistor connection
- Verify button grounds properly when pressed
- Increase debounce time if getting multiple triggers
