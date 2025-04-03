# Design Notes for LED Bo Staff Controller

## Project Architecture Overview

The LED Bo Staff Controller is designed with modularity and performance in mind. This document captures important design decisions and architecture notes.

### Core Components

1. **LEDController**: Manages LED strips and effects
2. **ButtonHandler**: Handles button input and debouncing
3. **AccelerometerHandler**: Processes MPU-6050 data and detects impacts
4. **SettingsManager**: Handles persistent storage of settings

### Code Organization

The project follows a component-based approach with clear separation of concerns:

```
├── include/
│   └── BoStaff.h     # Main definitions and class declarations
├── src/
│   ├── main.cpp       # Entry point and global setup
│   ├── LEDController.cpp
│   ├── ButtonHandler.cpp
│   ├── AccelerometerHandler.cpp
│   ├── SettingsManager.cpp
│   └── Effects/       # Specialized effect implementations
│       ├── FireEffect.h
│       ├── PulseEffect.h
│       └── ...
```

## Design Decisions

### 1. Simplified WLED Approach

While WLED offers a comprehensive feature set, we've chosen to create a more focused implementation for several reasons:

- **Performance**: By removing networking, web server, and MQTT, we've reduced memory usage and improved responsiveness
- **Battery Life**: Fewer components means less power consumption
- **Simplicity**: A focused codebase is easier to maintain and modify

### 2. Effects System

We've implemented a simple effect system that allows for:

- Independent effect control for each LED strip
- Smooth transitions between effects
- Custom parameters for each effect

### 3. Hardware Considerations

- **ESP8266**: Chosen for its balance of processing power, small size, and lower power consumption
- **MPU-6050**: Provides 6-axis motion tracking with sufficient sensitivity for impact detection
- **WS2812B LEDs**: Offer good color reproduction, easy control, and reasonable power consumption

### 4. Power Management

The system doesn't include explicit power management beyond what's built into the ESP8266. Future versions could improve this by:

- Adding sleep modes for the ESP8266
- Implementing dynamic brightness based on battery voltage
- Adding low-power modes when inactive

## Performance Considerations

### LED Update Rate

The LED controller updates at approximately 33 frames per second (30ms delay). This provides a good balance between:

- Smooth animation
- CPU processing time
- Power consumption

### Accelerometer Sampling

The accelerometer is sampled on each main loop iteration. To prevent false triggers, we implement:

- A cooldown period between impact detections
- An adjustable threshold for impact sensitivity

## Future Improvements

### Code Enhancements

- Implement a more robust effect system with parameter controls
- Add battery voltage monitoring
- Implement adaptive brightness based on battery level

### Feature Ideas

- Sound reactivity (external microphone)
- Bluetooth control for remote configuration
- Multiple button support for more control options
- Advanced motion-based effects (reacts to spinning, etc.)

### Hardware Upgrades

- Explore ESP32 for more processing power if needed
- Consider adding a gyroscope for rotation-based effects
- Investigate alternative LED types for higher color fidelity

## References and Resources

- [FastLED Library Documentation](https://github.com/FastLED/FastLED/wiki/Overview)
- [Adafruit MPU6050 Library](https://github.com/adafruit/Adafruit_MPU6050)
- [WLED Project](https://github.com/Aircoookie/WLED) - Original inspiration
