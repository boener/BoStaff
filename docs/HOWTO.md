# Building Your LED Bo Staff

This guide will walk you through the entire process of building an LED bo staff using this code.

## Materials

### Electronics

- ESP8266MOD D1mini microcontroller
- MPU-6050 accelerometer/gyroscope module
- WS2812B LED strip (400 LEDs total - approximately 6.7m of 60 LEDs/m strip)
- Momentary push button
- 10K resistor (for button pull-up)
- 470-1000 ohm resistor (for LED data line)
- 1000μF capacitor (electrolytic, for power stabilization)
- 18650 Li-ion batteries (3-4, depending on desired runtime)
- 18650 battery holder
- TP4056 Li-ion charging module with protection
- 5V buck converter/regulator
- Wire (22-24 AWG silicone wire works well)
- JST connectors
- Perfboard or custom PCB (optional)

### Mechanical/Structural

- Polycarbonate tubes, 1.25" diameter, 0.25" wall thickness, ~3 feet each (x2)
- Aluminum tube/rod for central housing
- 3D printed internal carrier for electronics
- Heat shrink tubing
- Clear epoxy
- Diffusion material (optional)

### Tools

- Soldering iron and solder
- Wire cutters/strippers
- Heat gun
- Multimeter
- Drill with bits
- Files for deburring
- 3D printer (or service)

## Building Process

### 1. Preparing the LED Strip

1. **Calculate LED segments**: You need 400 LEDs total, arranged as 4 segments of 100 LEDs each
2. **Cut and test**: Cut your LED strip into appropriate lengths and test each segment
3. **Plan the folded arrangement**: The strip will run up one side and fold back down, repeated for each half of the staff
4. **Solder connections**: Ensure power and data connections can handle the folding at each end

### 2. Prepare the Polycarbonate Tubes

1. Cut tubes to desired length (typically ~3 feet each for a 6-foot staff)
2. Sand/deburr the ends
3. Drill small holes at one end of each tube for wire pass-through
4. Consider adding diffusion material inside the tubes for better light distribution

### 3. Central Housing

1. 3D print the internal carrier for electronics
2. Design should include:
   - Battery compartment for 3-4 18650 cells
   - Circuit board mount
   - Channels for wiring
   - Button mount with easy external access
   - Secure attachment points to both tube halves

### 4. Electronics Assembly

1. **Power circuit**:
   ```
   Batteries → TP4056 → 5V Buck Converter → ESP8266 & LEDs
   ```
   
2. **Wire the ESP8266 connections**:
   - LED data to D7 (GPIO13) with 470Ω resistor inline
   - MPU-6050 SDA to D2 (GPIO4)
   - MPU-6050 SCL to D1 (GPIO5)
   - MPU-6050 VCC to 3.3V (from ESP8266)
   - MPU-6050 GND to GND
   - Button to D6 (GPIO12) with 10K pull-up to 3.3V
   - Add 1000μF capacitor across LED power rails

3. **LED Strip Wiring**:
   - Connect all 400 LEDs in a single continuous strip
   - Data flows through segments in order: 0→99, 100→199, 200→299, 300→399
   - Ensure adequate power injection every 100-150 LEDs

### 5. Software Setup

1. Install PlatformIO in VS Code
2. Clone this repository
3. Open the project in VS Code
4. Build and upload the firmware:
   ```bash
   pio run -t upload
   ```
5. Monitor serial output to verify all systems are working:
   ```bash
   pio device monitor
   ```

### 6. Final Assembly

1. **Install LED strips** in the tubes following the folded arrangement:
   - First half: Segment 1 (0-99) up, Segment 2 (100-199) back down
   - Second half: Segment 3 (200-299) up, Segment 4 (300-399) back down
2. **Thread wires** through the central housing
3. **Test all connections** with a multimeter before final assembly
4. **Secure electronics** in the central housing
5. **Epoxy** the central housing to both tube halves
6. **Final testing** of all functions

## Configuration and Tuning

### Impact Detection

The dual-sensor system uses fixed thresholds optimized for bo staff movements:
- **Accelerometer**: 75.0 m/s² (detects linear impacts)
- **Gyroscope**: 12.0 rad/s (detects rotational impacts)
- **Classification**: Impacts with gyro > 6.3 rad/s are classified as rotational

No calibration needed - the system is pre-tuned for typical bo staff usage.

### Brightness Settings

Adjust brightness in `include/EffectsConfig.h`:
- `DEFAULT_BRIGHTNESS`: Normal operation (default: 45/255)
- `IMPACT_BRIGHTNESS`: During impact flash (default: 150/255)
- `LOW_BATTERY_BRIGHTNESS`: Power saving mode (default: 25/255)

### Effect Customization

Each effect has configurable parameters in `include/EffectsConfig.h`:
- Fire effects: cooling, sparking, colors
- Pulse effects: speed, width, fade rate
- Rainbow: speed, color compression
- Strobe: on/off timing, fade options

## Usage Tips

### Battery Life

To maximize battery life:
- Use lower brightness settings
- Dark effects (fire) use less power than bright (solid white)
- The system automatically reduces brightness when battery is low

### Performance Tips

- The impact detection has a 500ms cooldown to prevent false triggers
- Different impact types (stab vs rotation) are tracked separately
- Serial monitor shows detailed impact information for debugging

### Maintenance

- Periodically check battery connections
- Inspect tubes for cracks after heavy use
- Keep charging port accessible and clean
- Update firmware for new features and improvements

## Troubleshooting

### Power Issues

- **LEDs flicker**: Check power connections and capacitor
- **Random resets**: Battery voltage may be too low
- **Won't turn on**: Check battery charge and protection circuit

### Impact Detection Problems

- **No detection**: Check MPU-6050 I2C connections (SDA/SCL)
- **False triggers**: Ensure MPU-6050 is firmly mounted
- **I2C errors**: Check pull-up resistors on I2C lines

### LED Problems

- **Wrong colors**: Verify COLOR_ORDER in code matches your LEDs
- **Sections not working**: Check data line continuity
- **Flickering**: Add more power injection points

### Button Issues

- **No response**: Check pull-up resistor and connections
- **Multiple triggers**: Increase debounce time in code

## Advanced Modifications

### Custom Effects

Create new effects by:
1. Copy an existing effect file in `src/Effects/`
2. Modify the update() method for your pattern
3. Add the effect to the effect list in `main.cpp`
4. Update `NUM_EFFECTS` in `effects.h`

### Wireless Control

Add ESP-NOW or WiFi control:
- Use ESP-NOW for low-latency wireless sync with other staffs
- Add web interface for configuration (increases power usage)

### Additional Sensors

Consider adding:
- Magnetometer for absolute orientation
- Barometer for height detection
- Temperature sensor for thermal safety

### Performance Enhancements

- Use DMA for LED updates to reduce CPU usage
- Implement predictive impact detection
- Add gesture recognition using ML

## Safety Considerations

- Always use protected 18650 batteries
- Include a fuse in the main power line
- Ensure proper ventilation in the central housing
- Use flame-retardant materials where possible
- Test thoroughly before public performances

## Community and Support

- Submit issues on GitHub for bugs or feature requests
- Share your builds and modifications
- Join the LED performance art community forums

---

Happy building! May your staff light up the night! 🔥✨
