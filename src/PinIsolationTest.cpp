#include <Arduino.h>
#include <FastLED.h>

// Pin and LED configuration
#define LED_PIN D7           // Test pin D7 (GPIO13)
#define NUM_LEDS 200         // Number of LEDs in the strip
#define LED_TYPE WS2812B     // LED strip type
#define COLOR_ORDER GRB      // Color order

// LED array
CRGB leds[NUM_LEDS];

// Test parameters
#define NUM_TEST_PATTERNS 4
uint8_t currentPattern = 0;
uint8_t hue = 0;
uint8_t pos = 0;
unsigned long lastPatternChange = 0;
unsigned long patternDuration = 10000; // 10 seconds per pattern

// Timing parameters
unsigned long lastMicros = 0;
unsigned long lastShow = 0;
unsigned long frameDuration = 0;
unsigned long frameCount = 0;
unsigned long lastFpsUpdate = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("ESP8266 D7 Pin Isolation Test"));
  Serial.println(F("Testing pin D7 (GPIO13) in isolation"));
  
  delay(1000);
  
  // Disable WiFi to reduce potential interference
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  
  Serial.println(F("WiFi disabled for testing"));
  
  // Print system information
  Serial.print(F("ESP8266 Chip ID: 0x")); Serial.println(ESP.getChipId(), HEX);
  Serial.print(F("Flash Chip ID: 0x")); Serial.println(ESP.getFlashChipId(), HEX);
  Serial.print(F("Flash Size: ")); Serial.print(ESP.getFlashChipRealSize() / 1024); Serial.println(F(" KB"));
  
  // Initialize the LED strip - standard configuration
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  
  // Clear all LEDs
  FastLED.clear();
  FastLED.show();
  
  Serial.println(F("Testing different LED patterns:"));
  Serial.println(F("1. Solid color (changing hue)"));
  Serial.println(F("2. Moving dot"));
  Serial.println(F("3. Rainbow"));
  Serial.println(F("4. Alternating stripes"));
  
  Serial.println(F("\nEach pattern will run for 10 seconds"));
  Serial.println(F("Monitoring for timing issues..."));
  
  // Initialize timing variables
  lastMicros = micros();
  lastFpsUpdate = millis();
  lastPatternChange = millis();
}

void loop() {
  // Update pattern
  unsigned long currentMillis = millis();
  
  // Check if it's time to change patterns
  if (currentMillis - lastPatternChange >= patternDuration) {
    currentPattern = (currentPattern + 1) % NUM_TEST_PATTERNS;
    lastPatternChange = currentMillis;
    
    // Print pattern change
    Serial.print(F("Switching to pattern "));
    Serial.print(currentPattern + 1);
    Serial.print(F(" of "));
    Serial.println(NUM_TEST_PATTERNS);
    
    // Reset pattern variables
    pos = 0;
  }
  
  // Update LED pattern based on current test
  switch (currentPattern) {
    case 0: // Solid color
      fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255));
      hue++;
      break;
      
    case 1: // Moving dot
      fadeToBlackBy(leds, NUM_LEDS, 64); // Fade existing LEDs
      leds[pos] = CHSV(hue, 255, 255);
      pos = (pos + 1) % NUM_LEDS;
      hue += 8;
      break;
      
    case 2: // Rainbow
      fill_rainbow(leds, NUM_LEDS, hue, 1);
      hue++;
      break;
      
    case 3: // Alternating stripes
      for (int i = 0; i < NUM_LEDS; i++) {
        if ((i + pos) % 20 < 10) {
          leds[i] = CHSV(hue, 255, 255);
        } else {
          leds[i] = CHSV(hue + 128, 255, 255);
        }
      }
      pos = (pos + 1) % 20; // Move the pattern
      hue += 2;
      break;
  }
  
  // Critical section - measure timing and update LEDs
  unsigned long preShow = micros();
  
  // Method A: Regular show
  //FastLED.show();
  
  // Method B: With interrupt disabling
  noInterrupts();
  FastLED.show();
  interrupts();
  
  // Measure time taken for the show operation
  unsigned long postShow = micros();
  frameDuration = postShow - preShow;
  
  // Calculate update rate
  lastShow = postShow;
  frameCount++;
  
  // Print FPS and timing information every second
  if (currentMillis - lastFpsUpdate >= 1000) {
    unsigned long fps = frameCount;
    frameCount = 0;
    
    Serial.print(F("FPS: ")); Serial.print(fps);
    Serial.print(F(" | Frame time: ")); Serial.print(frameDuration);
    Serial.print(F("µs | Pattern: ")); Serial.print(currentPattern + 1);
    Serial.print(F(" | Free heap: ")); Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes"));
    
    lastFpsUpdate = currentMillis;
  }
  
  // Short delay to prevent watchdog issues
  yield();
  delay(5);
}