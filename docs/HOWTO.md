# Building Your LED Bo Staff

This guide will walk you through the entire process of building an LED bo staff using this code.

## Materials

### Electronics

- ESP8266MOD D1mini microcontroller
- MPU-6050 accelerometer
- WS2812B LED strips (5m of 60 LEDs/m = 300 LEDs, you'll need about 6.7m total)
- Momentary push button
- 10K resistor
- 470-1000 ohm resistor (for LED data line)
- 1000μF capacitor (electrolytic, for power stabilization)
- 18650 Li-ion batteries (3-4, depending on desired runtime)
- 18650 battery holder
- TP4056 Li-ion charging module with protection
- 5V buck converter/regulator
- Wire (22-24 AWG silicon wire works well)
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

### 1. Preparing the LED Strips

1. Measure and cut your LED strips to fit inside your polycarbonate tubes
2. Test each strip segment before installation
3. Plan your wiring - you'll need power and data connections that can pass through the central housing

### 2. Prepare the Polycarbonate Tubes

1. Cut tubes to desired length (typically ~3 feet each for a 6-foot staff)
2. Sand/deburr the ends
3. Drill small holes at one end of each tube for wire pass-through

### 3. Central Housing

1. 3D print the internal carrier for electronics
2. Design should include:
   - Battery compartment
   - Circuit board mount
   - Channels for wiring
   - Button mount
   - Secure attachment points to the tubes

### 4. Electronics Assembly

1. Wire the circuit according to the diagram in `docs/setup.md`
2. Basic circuit flow:
   ```
   Batteries → TP4056 → 5V Regulator → ESP8266 & LEDs
   ```
3. Connect the MPU-6050 to the I2C pins (D1/D2)
4. Connect LED data lines to D1 for first strip and D2 for second strip
5. Add a ~470 ohm resistor inline with each LED data line
6. Connect the button to D6 with a pull-up resistor

### 5. Software Setup

1. Follow the instructions in `docs/setup.md` to install the firmware
2. Test the system before final assembly

### 6. Final Assembly

1. Insert LED strips into the polycarbonate tubes
2. Thread wires through the central housing
3. Secure all connections and check for shorts with a multimeter
4. Epoxy the central housing to the tubes
5. Test all functions again

## Usage Tips

### Battery Life

To maximize battery life:

- Lower the brightness in `include/BoStaff.h` (default is 150)
- Use effects with darker colors (fire effect uses less power than solid white)

### Performance Tuning

- Adjust impact threshold based on your use case
- Modify effect parameters in their respective files

### Maintenance

- Check battery connections periodically
- Inspect tubes for cracks or damage
- Keep firmware updated with improvements

## Troubleshooting

### Power Issues

- If LEDs flicker during high-brightness effects, your power supply may be insufficient
- If the system resets randomly, check your battery voltage

### Impact Detection Problems

- If impacts are not detected, try decreasing the threshold
- If false triggers occur, increase the threshold

### LED Problems

- If LEDs show wrong colors or don't light up, check your data connections
- If some LEDs work but others don't, you might have a break in the strip
