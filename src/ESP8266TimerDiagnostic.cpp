#include <Arduino.h>
#include <FastLED.h>
#include <Ticker.h>
#include <user_interface.h> // ESP8266 specific header

// Pin definitions
#define LED_PIN D7          // GPIO13
#define PIN_TO_MONITOR D7   // Monitor the pin we're using for LEDs
#define TEST_LED LED_BUILTIN // Built-in LED for diagnostics

// LED configuration
#define NUM_LEDS 10       // Using fewer LEDs to reduce power/complexity for testing
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// LED data array
CRGB leds[NUM_LEDS];

// Timer frequencies to test
#define NUM_TEST_FREQUENCIES 4
const float testFrequencies[NUM_TEST_FREQUENCIES] = {
  1.0,    // 1Hz - exactly the 1-second interval we're seeing flashing at
  2.0,    // 2Hz
  0.5,    // 0.5Hz
  10.0    // 10Hz
};

// Testing state
int currentFrequencyIndex = 0;
bool systemTickerActive = false;
Ticker systemTicker;
os_timer_t osTimer;
uint32_t tickerCounter = 0;
uint32_t osTimerCounter = 0;

// Variables for monitoring pin behavior
volatile uint32_t pinChangeCount = 0;
uint32_t lastPinChangeCount = 0;
unsigned long lastPinCheck = 0;
bool lastPinState = false;

// Variables for LED pattern
uint8_t hue = 0;
unsigned long lastHueUpdate = 0;
unsigned long lastLedShow = 0;
unsigned long frameCount = 0;
unsigned long lastFpsUpdate = 0;

// Function prototypes
void IRAM_ATTR onPinChange();
void systemTickerCallback();
void osTimerCallback(void *pArg);
void toggleSystemTicker();
void startOsTimer(float frequency);
void stopOsTimer();
void printTimerStatus();
void updateLeds();

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println(F("ESP8266 Timer Diagnostic for D7 Pin"));
  Serial.println(F("Investigating 1-second flashing issue"));
  
  // Initialize built-in LED for visual feedback
  pinMode(TEST_LED, OUTPUT);
  digitalWrite(TEST_LED, HIGH); // ESP8266 built-in LED is active LOW
  
  // Initialize pin monitoring
  pinMode(PIN_TO_MONITOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_TO_MONITOR), onPinChange, CHANGE);
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  
  // Clear LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  Serial.println(F("\nThis diagnostic will:"));
  Serial.println(F("1. Test different timer frequencies to see if they cause LED flashing"));
  Serial.println(F("2. Monitor the D7 pin for unexpected state changes"));
  Serial.println(F("3. Experiment with different LED update methods"));
  
  Serial.println(F("\nCommands available:"));
  Serial.println(F("t - Toggle system ticker"));
  Serial.println(F("o - Start OS timer at current frequency"));
  Serial.println(F("s - Stop OS timer"));
  Serial.println(F("n - Next test frequency"));
  Serial.println(F("r - Reset all timers and counters"));
  
  Serial.println(F("\nStarting with all timers inactive"));
  
  // Initialize timing variables
  lastHueUpdate = millis();
  lastPinCheck = millis();
  lastFpsUpdate = millis();
  
  // Print initial status
  printTimerStatus();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 't': // Toggle system ticker
        toggleSystemTicker();
        break;
      
      case 'o': // Start OS timer
        startOsTimer(testFrequencies[currentFrequencyIndex]);
        break;
        
      case 's': // Stop OS timer
        stopOsTimer();
        break;
        
      case 'n': // Next test frequency
        currentFrequencyIndex = (currentFrequencyIndex + 1) % NUM_TEST_FREQUENCIES;
        Serial.print(F("Changed test frequency to: "));
        Serial.print(testFrequencies[currentFrequencyIndex]);
        Serial.println(F("Hz"));
        
        // If system ticker is active, update its frequency
        if (systemTickerActive) {
          systemTicker.detach();
          systemTicker.attach(1.0 / testFrequencies[currentFrequencyIndex], systemTickerCallback);
          Serial.println(F("System ticker updated to new frequency"));
        }
        break;
        
      case 'r': // Reset all timers and counters
        // Stop all timers
        systemTicker.detach();
        stopOsTimer();
        systemTickerActive = false;
        
        // Reset counters
        tickerCounter = 0;
        osTimerCounter = 0;
        pinChangeCount = 0;
        lastPinChangeCount = 0;
        
        Serial.println(F("All timers and counters reset"));
        printTimerStatus();
        break;
    }
  }
  
  // Update LED pattern - simple rainbow for easy visual inspection
  if (currentMillis - lastHueUpdate >= 50) { // 20Hz hue update
    hue++;
    lastHueUpdate = currentMillis;
  }
  
  // Display simple rainbow pattern
  fill_rainbow(leds, NUM_LEDS, hue, 8);
  
  // Update LEDs with interrupt disabling
  if (currentMillis - lastLedShow >= 33) { // ~30fps update rate
    noInterrupts();
    FastLED.show();
    interrupts();
    
    lastLedShow = currentMillis;
    frameCount++;
  }
  
  // Check pin change rate every second
  if (currentMillis - lastPinCheck >= 1000) {
    uint32_t pinChanges = pinChangeCount - lastPinChangeCount;
    lastPinChangeCount = pinChangeCount;
    
    Serial.print(F("D7 pin changes in last second: "));
    Serial.println(pinChanges);
    
    lastPinCheck = currentMillis;
  }
  
  // Display FPS and timer stats every second
  if (currentMillis - lastFpsUpdate >= 1000) {
    Serial.print(F("LED update FPS: "));
    Serial.print(frameCount);
    
    // Print additional diagnostic information
    Serial.print(F(" | System ticker count: "));
    Serial.print(tickerCounter);
    Serial.print(F(" | OS timer count: "));
    Serial.println(osTimerCounter);
    
    // Reset frame counter
    frameCount = 0;
    lastFpsUpdate = currentMillis;
  }
  
  // Toggle built-in LED based on system ticker
  digitalWrite(TEST_LED, systemTickerActive && (tickerCounter % 2 == 0) ? LOW : HIGH);
  
  // Ensure ESP8266 background tasks get processed
  yield();
}

// ISR for monitoring pin state changes
void IRAM_ATTR onPinChange() {
  pinChangeCount++;
}

// System ticker callback
void systemTickerCallback() {
  tickerCounter++;
  
  // Print ticker event every 10 ticks (to avoid flooding serial)
  if (tickerCounter % 10 == 0) {
    // Note: Serial prints from callbacks can cause stability issues
    // But for diagnostic purposes, we'll do it sparingly
    Serial.print(F("Ticker event: "));
    Serial.println(tickerCounter);
  }
}

// OS timer callback
void osTimerCallback(void *pArg) {
  osTimerCounter++;
  
  // Print timer event every 10 ticks (to avoid flooding serial)
  if (osTimerCounter % 10 == 0) {
    // Again, Serial prints from callbacks can cause stability issues
    // But for diagnostics it's useful
    Serial.print(F("OS Timer event: "));
    Serial.println(osTimerCounter);
  }
}

// Toggle system ticker
void toggleSystemTicker() {
  if (systemTickerActive) {
    systemTicker.detach();
    systemTickerActive = false;
    Serial.println(F("System ticker stopped"));
  } else {
    float frequency = testFrequencies[currentFrequencyIndex];
    systemTicker.attach(1.0 / frequency, systemTickerCallback);
    systemTickerActive = true;
    Serial.print(F("System ticker started at "));
    Serial.print(frequency);
    Serial.println(F("Hz"));
  }
  
  printTimerStatus();
}

// Start OS timer at specified frequency
void startOsTimer(float frequency) {
  // Convert frequency to microseconds
  uint32_t usInterval = (uint32_t)(1000000.0 / frequency);
  
  // Initialize OS timer
  os_timer_disarm(&osTimer);
  os_timer_setfn(&osTimer, osTimerCallback, NULL);
  os_timer_arm_us(&osTimer, usInterval, true); // true = repeat
  
  Serial.print(F("OS timer started at "));
  Serial.print(frequency);
  Serial.print(F("Hz ("));
  Serial.print(usInterval);
  Serial.println(F("µs)"));
  
  printTimerStatus();
}

// Stop OS timer
void stopOsTimer() {
  os_timer_disarm(&osTimer);
  Serial.println(F("OS timer stopped"));
  
  printTimerStatus();
}

// Print current timer status
void printTimerStatus() {
  Serial.println(F("\nCurrent Status:"));
  Serial.print(F("Test frequency: "));
  Serial.print(testFrequencies[currentFrequencyIndex]);
  Serial.println(F("Hz"));
  
  Serial.print(F("System ticker: "));
  Serial.println(systemTickerActive ? F("ACTIVE") : F("INACTIVE"));
  Serial.print(F("OS timer: "));
  Serial.println(os_timer_pending(&osTimer) ? F("ACTIVE") : F("INACTIVE"));
  
  Serial.print(F("System ticker counter: "));
  Serial.println(tickerCounter);
  Serial.print(F("OS timer counter: "));
  Serial.println(osTimerCounter);
  Serial.print(F("D7 pin change counter: "));
  Serial.println(pinChangeCount);
  
  Serial.println();
}