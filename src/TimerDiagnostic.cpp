#include <Arduino.h>
#include <FastLED.h>
#include <Ticker.h>

// Pin definitions
#define LED_PIN D7  // The problematic strip moved from D3 to D7
#define NUM_LEDS 200

// LED array
CRGB leds[NUM_LEDS];

// Ticker for simulating other timing operations
Ticker periodicTimer;
Ticker wifiTimer;

// Variables for diagnostics
volatile bool periodicFlag = false;
volatile bool wifiFlag = false;
unsigned long lastFrameTime = 0;
unsigned long frameCount = 0;
unsigned long lastSecondCount = 0;

// Test modes
enum TestMode {
  SOLID_COLOR,
  MOVING_DOT,
  RAINBOW,
  ALTERNATING_COLORS,
  NUM_MODES
};

TestMode currentMode = SOLID_COLOR;
uint8_t currentModeIdx = 0;
uint8_t hue = 0;
uint8_t dotPosition = 0;
bool alternateState = false;

// Function prototypes
void periodicCallback();
void wifiSimCallback();
void switchMode();
void printModeInfo();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("BoStaff Timer Conflict Diagnostic"));
  Serial.println(F("Testing D7 LED strip with timer simulation"));
  
  delay(1000); // Give time for serial to initialize
  
  // Print system information
  Serial.print(F("ESP8266 CPU Frequency: ")); Serial.print(ESP.getCpuFreqMHz()); Serial.println(F(" MHz"));
  Serial.print(F("ESP8266 SDK Version: ")); Serial.println(ESP.getSdkVersion());
  
  // Initialize the LED strip
  Serial.println(F("Initializing LED strip on D7..."));
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  
  // Clear all LEDs to black
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  // Initialize tickers for simulating timing conflicts
  // One at 1 second interval (common system timer frequency)
  periodicTimer.attach(1.0, periodicCallback);
  
  // Another at 0.25 second interval (to simulate WiFi activity)
  wifiTimer.attach(0.25, wifiSimCallback);
  
  // Initialize timing variables
  lastFrameTime = millis();
  
  // Print initial mode information
  printModeInfo();
  
  Serial.println(F("Press 'm' to switch between test modes"));
  Serial.println(F("Diagnostic running..."));
}

void loop() {
  // Update LEDs based on current test mode
  switch (currentMode) {
    case SOLID_COLOR:
      // Simple solid color that changes hue slowly
      fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255));
      hue++;
      break;
      
    case MOVING_DOT:
      // Single dot moving through the strip
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      leds[dotPosition] = CRGB::White;
      dotPosition = (dotPosition + 1) % NUM_LEDS;
      break;
      
    case RAINBOW:
      // Rainbow pattern
      fill_rainbow(leds, NUM_LEDS, hue, 1);
      hue++;
      break;
      
    case ALTERNATING_COLORS:
      // Alternating colors
      if (alternateState) {
        fill_solid(leds, NUM_LEDS, CRGB::Red);
      } else {
        fill_solid(leds, NUM_LEDS, CRGB::Blue);
      }
      
      // Only change state periodically
      if (frameCount % 30 == 0) {
        alternateState = !alternateState;
      }
      break;
      
    default:
      break;
  }
  
  // Process any timer callbacks
  if (periodicFlag) {
    // This simulates a 1-second timer interrupt
    // Just set a flag, don't do any processing here
    periodicFlag = false;
    Serial.println(F("1-second timer fired"));
  }
  
  if (wifiFlag) {
    // This simulates WiFi or other system activity
    // Just set a flag, don't do any processing here
    wifiFlag = false;
  }
  
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'm' || cmd == 'M') {
      switchMode();
    }
  }
  
  // Method 1: Regular show
  /*
  FastLED.show();
  */
  
  // Method 2: With interrupt disabling
  noInterrupts();
  FastLED.show();
  interrupts();
  
  // Increment frame counter
  frameCount++;
  
  // Calculate and display FPS every second
  if (millis() - lastSecondCount >= 1000) {
    Serial.print(F("FPS: ")); Serial.print(frameCount - lastFrameTime);
    Serial.print(F(" | Mode: ")); Serial.print(currentModeIdx + 1);
    Serial.print(F("/")); Serial.print(NUM_MODES);
    Serial.print(F(" | Heap: ")); Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes"));
    
    lastFrameTime = frameCount;
    lastSecondCount = millis();
  }
  
  // Small delay to prevent watchdog issues but not so long it creates visible effects
  delay(10);
}

// Callback for 1-second periodic timer
void periodicCallback() {
  periodicFlag = true;
}

// Callback for WiFi activity simulation
void wifiSimCallback() {
  wifiFlag = true;
}

// Switch between test modes
void switchMode() {
  currentModeIdx = (currentModeIdx + 1) % NUM_MODES;
  currentMode = static_cast<TestMode>(currentModeIdx);
  printModeInfo();
}

// Print information about current mode
void printModeInfo() {
  Serial.print(F("Switched to mode: "));
  
  switch (currentMode) {
    case SOLID_COLOR:
      Serial.println(F("SOLID COLOR - Slow color changing"));
      break;
    case MOVING_DOT:
      Serial.println(F("MOVING DOT - Single LED moving through strip"));
      break;
    case RAINBOW:
      Serial.println(F("RAINBOW - Rainbow pattern"));
      break;
    case ALTERNATING_COLORS:
      Serial.println(F("ALTERNATING COLORS - Red/Blue alternating"));
      break;
    default:
      Serial.println(F("UNKNOWN"));
      break;
  }
}