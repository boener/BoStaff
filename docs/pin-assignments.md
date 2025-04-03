# Pin Assignments for Bo Staff Controller

## Overview

This document details the pin assignments for the Bo Staff LED controller, explaining which pins are used for each component and why they were chosen.

## Pin Assignment Table

| Component | Pin | GPIO | Description |
|-----------|-----|------|-------------|
| LED Strip 1 | D3 | GPIO0 | Data pin for first LED strip (200 LEDs) |
| LED Strip 2 | D4 | GPIO2 | Data pin for second LED strip (200 LEDs) |
| Button | D6 | GPIO12 | Mode selection button (active LOW with pull-up) |
| MPU-6050 SCL | D1 | GPIO5 | I2C clock line for accelerometer |
| MPU-6050 SDA | D2 | GPIO4 | I2C data line for accelerometer |

## Rationale for Pin Selection

### I2C Pins for MPU-6050

We're using the default I2C pins on the ESP8266:
- **D1 (GPIO5)**: SCL (clock line)
- **D2 (GPIO4)**: SDA (data line)

Using the default I2C pins provides the following advantages:
1. Better compatibility with libraries that assume default I2C pins
2. Hardware I2C support for more reliable communication
3. Simpler code that doesn't require custom Wire initialization

### LED Strip Data Pins

We selected these pins for the LED strips:
- **D3 (GPIO0)**: First LED strip
- **D4 (GPIO2)**: Second LED strip

These pins were chosen because:
1. They're available general-purpose pins not already used for I2C
2. They support the high-speed data requirements of WS2812B LEDs
3. GPIO0 and GPIO2 are suitable for output during normal operation

### Button Pin

- **D6 (GPIO12)**: Mode selection button

This pin was selected because:
1. It supports internal pull-up resistors
2. It doesn't affect boot mode (unlike GPIO0 or GPIO2)
3. It's physically separated from other used pins on the board

## Important Notes

### GPIO0 and GPIO2 Special Considerations

GPIO0 and GPIO2 (D3 and D4) are used for boot mode selection. When using these pins for LED strips, be aware of the following:

1. During normal operation, these pins function as standard GPIOs
2. These pins should be HIGH at boot time for normal operation
3. When flashing firmware, the LED strips should be disconnected or the board should be powered through USB only

### I2C Addressing

The MPU-6050 typically uses I2C address 0x68. If you have address conflicts with other I2C devices, you might need to change the address by connecting the AD0 pin to 3.3V (which changes the address to 0x69).

## Wiring Recommendations

1. Use short signal wires for the LED data lines
2. Add a 470 ohm resistor in series with each LED data line
3. Place a 1000μF capacitor across power and ground near the LED strips
4. Use level shifters if your power supply is higher than 3.3V for the LEDs (5V)
