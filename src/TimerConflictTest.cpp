#include <Arduino.h>
#include <FastLED.h>
#include <Ticker.h>
#include <user_interface.h> // ESP8266 specific header

// Pin and LED configuration
#define LED_PIN D7           // Test pin D7 (GPIO13)
#define NUM_LEDS 10          // Reduced number of LEDs for testing
#define LED_TYPE WS2812B     // LED strip type
#define COLOR_ORDER GRB      // Color order

// LED array
CRGB leds[NUM_LEDS];

// Testing variables
uint8_t baseColor = 0;       // Base color for all LEDs
bool systemTimerActive = false;
bool osTimerActive = false;
bool wifiTimerActive = false;

// Ticker objects for simulating system timers
Ticker systemTimer;          // Ticker at 1Hz (1 second)
Ticker osTimer;              // Ticker at 10Hz (100ms)
Ticker wifiTimer;            // Ticker at 4Hz (250ms)

// Callback flags
volatile bool systemTimerFlag = false;
volatile bool osTimerFlag = false;
volatile bool wifiTimerFlag = false;

// Timer event counters
unsigned long systemTimerCount = 0;
unsigned long osTimerCount = 0;
unsigned long wifiTimerCount = 0;

// Function prototypes
void systemTimerCallback();
void osTimerCallback();
void wifiTimerCallback();
void toggleTimer(uint8_t timerIndex);
void updateLeds();

// For timing LED updates
unsigned long lastUpdate = 0;
unsigned long updateInterval = 33; // ~30fps

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("ESP8266 Timer Conflict Diagnostic Test"));
  Serial.println(F("Designed to isolate timer-related flashing issues"));
  
  delay(1000);
  
  // Initialize the LED strip with minimal LEDs
  Serial.println(F("Initializing LED strip on D7 with minimal LEDs..."));
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  
  // Clear all LEDs
  FastLED.clear();
  FastLED.show();
  
  Serial.println(F("\nCommands available:"));
  Serial.println(F("1 - Toggle 1Hz system timer (exactly 1 second interval)"));
  Serial.println(F("2 - Toggle 10Hz OS timer (100ms interval)"));
  Serial.println(F("3 - Toggle 4Hz WiFi timer (250ms interval)"));
  Serial.println(F("c - Change base color for LEDs"));
  Serial.println(F("r - Reset all timers"));
  
  Serial.println(F("\nStarting with all timers inactive"));
  Serial.println(F("Current status:"));
  Serial.println(F("- 1Hz system timer: OFF"));
  Serial.println(F("- 10Hz OS timer: OFF"));
  Serial.println(F("- 4Hz WiFi timer: OFF"));
  
  // Initialize timing
  lastUpdate = millis();
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case '1':
        toggleTimer(1);
        break;
      case '2':
        toggleTimer(2);
        break;
      case '3':
        toggleTimer(3);
        break;
      case 'c':
      case 'C':
        baseColor = (baseColor + 32) % 256;
        Serial.print(F("Base color changed to hue: "));
        Serial.println(baseColor);
        break;
      case 'r':
      case 'R':
        // Reset all timers
        systemTimer.detach();
        osTimer.detach();
        wifiTimer.detach();
        systemTimerActive = false;
        osTimerActive = false;
        wifiTimerActive = false;
        Serial.println(F("All timers reset"));
        Serial.println(F("- 1Hz system timer: OFF"));
        Serial.println(F("- 10Hz OS timer: OFF"));
        Serial.println(F("- 4Hz WiFi timer: OFF"));
        break;
    }
  }
  
  // Process timer flags
  if (systemTimerFlag) {
    systemTimerFlag = false;
    systemTimerCount++;
    Serial.print(F("1Hz timer event: "));
    Serial.println(systemTimerCount);
  }
  
  if (osTimerFlag) {
    osTimerFlag = false;
    osTimerCount++;
    
    // Only print every 10 events to avoid flooding serial
    if (osTimerCount % 10 == 0) {
      Serial.print(F("10Hz timer events: "));
      Serial.println(osTimerCount);
    }
  }
  
  if (wifiTimerFlag) {
    wifiTimerFlag = false;
    wifiTimerCount++;
    
    // Only print every 4 events to avoid flooding serial
    if (wifiTimerCount % 4 == 0) {
      Serial.print(F("4Hz timer events: "));
      Serial.println(wifiTimerCount);
    }
  }
  
  // Update LEDs at controlled interval
  if (millis() - lastUpdate >= updateInterval) {
    updateLeds();
    lastUpdate = millis();
  }
  
  // Yield to allow ESP8266 background tasks
  yield();
}

// Update LEDs with current pattern
void updateLeds() {
  // Create a simple pattern
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(baseColor + (i * 256 / NUM_LEDS), 255, 255);
  }
  
  // Disable interrupts during LED update to prevent timer conflicts
  noInterrupts();
  FastLED.show();
  interrupts();
}

// Toggle specified timer
void toggleTimer(uint8_t timerIndex) {
  switch (timerIndex) {
    case 1: // 1Hz system timer
      if (systemTimerActive) {
        systemTimer.detach();
        systemTimerActive = false;
        Serial.println(F("1Hz system timer disabled"));
      } else {
        systemTimer.attach(1.0, systemTimerCallback);
        systemTimerActive = true;
        Serial.println(F("1Hz system timer enabled"));
      }
      break;
      
    case 2: // 10Hz OS timer
      if (osTimerActive) {
        osTimer.detach();
        osTimerActive = false;
        Serial.println(F("10Hz OS timer disabled"));
      } else {
        osTimer.attach(0.1, osTimerCallback);
        osTimerActive = true;
        Serial.println(F("10Hz OS timer enabled"));
      }
      break;
      
    case 3: // 4Hz WiFi timer
      if (wifiTimerActive) {
        wifiTimer.detach();
        wifiTimerActive = false;
        Serial.println(F("4Hz WiFi timer disabled"));
      } else {
        wifiTimer.attach(0.25, wifiTimerCallback);
        wifiTimerActive = true;
        Serial.println(F("4Hz WiFi timer enabled"));
      }
      break;
  }
  
  // Print current status
  Serial.println(F("Current status:"));
  Serial.print(F("- 1Hz system timer: ")); Serial.println(systemTimerActive ? "ON" : "OFF");
  Serial.print(F("- 10Hz OS timer: ")); Serial.println(osTimerActive ? "ON" : "OFF");
  Serial.print(F("- 4Hz WiFi timer: ")); Serial.println(wifiTimerActive ? "ON" : "OFF");
}

// Timer callbacks
void systemTimerCallback() {
  systemTimerFlag = true;
}

void osTimerCallback() {
  osTimerFlag = true;
}

void wifiTimerCallback() {
  wifiTimerFlag = true;
}