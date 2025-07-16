# LED Arrangement Details

## Overview

The LED Bo Staff uses a "folded" LED strip arrangement with a single continuous 400 LED strip. This arrangement allows the strip to cover both halves of the staff symmetrically while only needing one data connection point at the center housing.

<div align="center">
  <img src="images/led-arrangement.svg" alt="LED Arrangement Diagram" width="80%">
</div>

## Single Strip Folded Configuration

The 400 LED strip is physically arranged into 4 segments, creating a symmetrical pattern across both halves of the staff:

### First Half (Left Side)
- **Segment 1 (LEDs 0-99)**: 
  - LED 0: Located at the center housing
  - LEDs 1-98: Run along the tube from center toward the far end
  - LED 99: At the far end of the first half
  
- **Segment 2 (LEDs 100-199)**: 
  - LED 100: At the far end (folded back, adjacent to LED 99)
  - LEDs 101-198: Run back along the tube toward the center
  - LED 199: Located at the center housing

### Second Half (Right Side)
- **Segment 3 (LEDs 200-299)**: 
  - LED 200: Located at the center housing
  - LEDs 201-298: Run along the tube from center toward the far end
  - LED 299: At the far end of the second half
  
- **Segment 4 (LEDs 300-399)**: 
  - LED 300: At the far end (folded back, adjacent to LED 299)
  - LEDs 301-398: Run back along the tube toward the center
  - LED 399: Located at the center housing

## Segment Access Methods

The `LEDController` class provides helper methods to access each segment with proper mapping:

```cpp
// Access methods handle the folded logic automatically
CRGB& getSegment1LED(int pos); // Maps 0-99 to actual LEDs 0-99
CRGB& getSegment2LED(int pos); // Maps 0-99 to actual LEDs 199-100 (reversed)
CRGB& getSegment3LED(int pos); // Maps 0-99 to actual LEDs 200-299
CRGB& getSegment4LED(int pos); // Maps 0-99 to actual LEDs 399-300 (reversed)
```

## Effect Considerations

This folded arrangement enables several design patterns:

1. **Symmetrical Effects**: Effects can be mirrored across both halves by treating segments 1&2 and 3&4 as pairs.

2. **Center-Emanating Effects**: Pulse effects naturally emanate from the center by starting at positions 0, 199, 200, and 399.

3. **Tip Effects**: Effects at the staff tips target positions around 99/100 and 299/300.

4. **Continuous Flow**: Effects can flow continuously through all 400 LEDs or be synchronized across segments.

## Physical Distance Calculations

When calculating effects based on physical position along the staff:

- **For Segments 1 & 3** (counting up): Physical distance from center = virtual position
- **For Segments 2 & 4** (counting down): Physical distance from center = 99 - virtual position

Example for a center-pulse effect:
```cpp
// For each segment, calculate brightness based on distance from center
for (int pos = 0; pos < 100; pos++) {
    // Segments 1 & 3: distance increases with position
    uint8_t distance = pos;
    uint8_t brightness = calculatePulseBrightness(distance);
    
    getSegment1LED(pos) = CHSV(hue, 255, brightness);
    getSegment3LED(pos) = CHSV(hue, 255, brightness);
    
    // Segments 2 & 4: same distance for symmetry
    getSegment2LED(pos) = CHSV(hue, 255, brightness);
    getSegment4LED(pos) = CHSV(hue, 255, brightness);
}
```

## Hardware Assembly Notes

When assembling the LED strip:

1. **Power Injection**: With 400 LEDs, inject power at:
   - Start (LED 0)
   - First fold point (LED 100)
   - Center crossing (LED 200)
   - Second fold point (LED 300)

2. **Folding Points**: 
   - Be gentle when folding at positions 99/100 and 299/300
   - Consider using flexible PCB connectors at fold points
   - Leave a small service loop to prevent stress

3. **Data Line Routing**:
   - Single data line runs through all 400 LEDs
   - Keep data wire away from power lines to reduce interference
   - Use shielded cable if experiencing signal issues

4. **Testing Sequence**:
   - Test each 100 LED segment individually before final assembly
   - Verify fold points aren't causing data issues
   - Run a sequential test pattern to ensure proper ordering

## Advantages of Single Strip Design

1. **Simpler Wiring**: Only one data pin needed (D7)
2. **Better Synchronization**: No timing differences between strips
3. **Easier Effects**: Single array simplifies effect calculations
4. **More Reliable**: Fewer connections mean fewer failure points

## Troubleshooting Tips

- **Dead sections**: Check connections at fold points (99/100, 299/300)
- **Wrong colors**: Verify COLOR_ORDER matches your LED type
- **Effects reversed**: Check segment mapping in effect code
- **Flickering at ends**: Add power injection if voltage drop is excessive
