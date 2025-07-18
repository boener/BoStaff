# BoStaff Setup Guide

## Hardware Setup

### Components Needed

- ESP8266MOD D1mini or compatible
- WS2812B LED strips (Total 400 LEDs)
- MPU-6050 accelerometer
- Momentary push button
- 1x 10K resistor (for button pull-up if not using internal pull-up)
- Li-ion battery setup with appropriate voltage regulator
- Wiring and connectors

### Wiring Diagram

```
                         +---------------+
                         |               |
                         |    ESP8266    |
                         |    D1 Mini    |
                         |               |
                         +---+-----------+
                             |
+----------------+           |
|                |           |
| LED Strip 1    |---------->D1 (GPIO5)
| (200 LEDs)     |           |
+----------------+           |
                             |
+----------------+           |
|                |           |
| LED Strip 2    |---------->D2 (GPIO4)
| (200 LEDs)     |           |
+----------------+           |
                             |
                             |
+----------------+           |
|                |           |
| Button         |---------->D6 (GPIO12)
|                |           |
+----------------+           |
                             |
                             |
+----------------+           |
|                |<--------->D2 (SDA)
| MPU-6050       |           |
| Accelerometer  |<--------->D1 (SCL)
|                |           |
+----------------+           |
```

### Power Considerations

The LED strips will draw significant power when at full brightness. A rough calculation:

- 400 LEDs at ~60mA per LED (full white) = 24A max theoretical draw
- Realistically with effects: ~5-10A

The power system needs to be designed to handle this load. A Li-ion battery setup with appropriate voltage regulation is recommended.

## Software Setup

### Development Environment

This project uses PlatformIO for development. To set up the environment:

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install the [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
3. Clone this repository
4. Open the project folder in VS Code
5. PlatformIO will automatically install dependencies

### Building and Uploading

To build and upload the firmware to your ESP8266:

1. Connect your ESP8266 to your computer via USB
2. In VS Code with PlatformIO, click the Upload button or run:
   ```
   pio run -t upload
   ```
3. To monitor serial output, run:
   ```
   pio device monitor
   ```

## Configuration

### Hardware Configuration

The pin definitions can be found in `include/BoStaff.h`. If you need to change the pinout, modify the following definitions:

```cpp
// Pin definitions
#define LED_PIN_1 D1  // GPIO5 - First LED strip
#define LED_PIN_2 D2  // GPIO4 - Second LED strip
#define BTN_PIN   D6  // GPIO12 - Button pin

// MPU6050 accelerometer pins (I2C)
#define SDA_PIN   D2  // GPIO4
#define SCL_PIN   D1  // GPIO5
```

### Effect and Brightness Configuration

**All effect settings and brightness controls are centralized in `include/EffectsConfig.h`** for easy modification. This includes:

#### Brightness Settings
- `DEFAULT_BRIGHTNESS`: Normal operating brightness (0-255)
- `LOW_BATTERY_BRIGHTNESS`: Reduced brightness for low battery
- `DIM_BEFORE_SLEEP`: Very dim level before sleep mode
- `IMPACT_BRIGHTNESS`: Brightness during impact flash effect

#### Impact Effect Settings
- `IMPACT_ACCEL_THRESHOLD`: Accelerometer sensitivity for impact detection
- `IMPACT_GYRO_THRESHOLD`: Gyroscope sensitivity for rotational impacts
- `IMPACT_FLASH_DURATION`: Duration of impact flash in milliseconds
- `IMPACT_COLOR`: Color of the impact flash
- `IMPACT_FADE_OUT`: Whether to fade out after impact
- `IMPACT_FADE_RATE`: Speed of fade effect

#### Individual Effect Settings
Each effect has its own section with customizable parameters:
- **Fire Effects**: Cooling rate, sparking frequency, heat dissipation
- **Pulse Effects**: Speed, width, colors, fade rates
- **Rainbow Effect**: Animation speed, color compression, saturation
- **Strobe Effect**: On/off timing, colors, brightness
- **Solid Effect**: Hue change rate, color shifting behavior

### Adding New Effects

To add a new effect:

1. Create a new header file in `src/Effects/`
2. Add the effect implementation
3. Update the `LEDController.cpp` file to include the new effect
4. Update the effect definitions in `BoStaff.h`
5. Add configuration parameters to `EffectsConfig.h`

## Usage

### Button Controls

- Single Press: Cycle to the next effect

### Effects

1. **Solid Color**: Displays a slowly changing solid color
2. **Fire**: Realistic fire simulation (3 variants with different colors)
3. **Pulse**: Energy waves emanating from the center (3 variants)
4. **Rainbow**: Smooth color transitions
5. **Strobe**: Rapid flashing effect

### Impact Detection

When the dual-sensor system detects an impact above the threshold:

- LED strips will flash bright blue-violet briefly
- Impact type is classified as STAB or ROTATION
- The system will then return to the current effect
- 500ms cooldown prevents multiple triggers

### Customization

**Important**: All customizable parameters are now located in `include/EffectsConfig.h` for centralized control. This includes:

- Brightness levels for different modes
- Impact detection thresholds and behavior
- All effect-specific parameters (colors, speeds, patterns)
- Power management settings

Simply edit the values in `EffectsConfig.h` and rebuild the firmware to apply your changes.

## Troubleshooting

### LEDs Not Working

- Check power supply: LEDs require 5V and sufficient current
- Check data connections: Ensure signal wires are properly connected
- Check ground connections: All components should share a common ground

### Accelerometer Not Detecting Impacts

- Check I2C connections: SDA and SCL should be properly wired
- Verify I2C address: The default address for MPU-6050 is 0x68
- Adjust thresholds: Modify `IMPACT_ACCEL_THRESHOLD` and `IMPACT_GYRO_THRESHOLD` in `EffectsConfig.h`

### Button Not Responding

- Check wiring: Ensure the button is connected to the correct pin
- Check pull-up: The code uses internal pull-up, but external may be needed

## Future Improvements

- Battery voltage monitoring
- External configuration interface
- More advanced effects
- Multiple button support for more control options