#include <Arduino.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>

// Pin and LED configuration
#define LED_PIN D7           // Test pin D7 (GPIO13)
#define NUM_LEDS 10          // Reduced number of LEDs for testing
#define LED_TYPE WS2812B     // LED strip type
#define COLOR_ORDER GRB      // Color order

// LED array
CRGB leds[NUM_LEDS];

// WiFi states for testing
enum WiFiTestState {
  WIFI_DISABLED,
  WIFI_ENABLED_NOT_CONNECTED,
  WIFI_CONNECTED,
  NUM_WIFI_STATES
};

WiFiTestState currentWiFiState = WIFI_DISABLED;
unsigned long lastStateChange = 0;
unsigned long stateChangeDuration = 30000; // 30 seconds per state

// Color patterns
uint8_t hue = 0;
bool flashState = false;
unsigned long lastHueChange = 0;
unsigned long lastFlash = 0;

// Timing variables
unsigned long lastFrame = 0;
unsigned long frameCount = 0;
unsigned long lastFpsUpdate = 0;

// Function prototypes
void changeWiFiState();
void printWiFiStatus();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("ESP8266 WiFi Timer Interference Test"));
  Serial.println(F("Testing for interference between WiFi and LED timing"));
  
  delay(1000);
  
  // Disable WiFi by default
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  
  // Initialize LED strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  
  // Clear all LEDs
  FastLED.clear();
  FastLED.show();
  
  Serial.println(F("\nWiFi Timer Interference Test"));
  Serial.println(F("This sketch will cycle through three WiFi states:"));
  Serial.println(F("1. WiFi completely disabled"));
  Serial.println(F("2. WiFi enabled but not connected"));
  Serial.println(F("3. WiFi enabled and connected to AP"));
  
  Serial.println(F("\nEach state will run for 30 seconds while LED patterns run"));
  Serial.println(F("Watch for any pattern of LED flickering, especially at 1-second intervals"));
  Serial.println(F("\nTest starting with WiFi DISABLED"));
  
  // Initialize timing variables
  lastStateChange = millis();
  lastHueChange = millis();
  lastFlash = millis();
  lastFpsUpdate = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check if it's time to change WiFi state
  if (currentMillis - lastStateChange >= stateChangeDuration) {
    changeWiFiState();
    lastStateChange = currentMillis;
  }
  
  // Update LED pattern
  // Simple rainbow pattern that should show any timing glitches
  if (currentMillis - lastHueChange >= 50) { // Update hue every 50ms
    hue++;
    lastHueChange = currentMillis;
  }
  
  // Set base pattern - a slowly changing rainbow
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue + (i * 25), 255, 255);
  }
  
  // Critical section - show LEDs with interrupts disabled
  noInterrupts();
  FastLED.show();
  interrupts();
  
  // Increment frame counter
  frameCount++;
  
  // Calculate and display FPS every second
  if (currentMillis - lastFpsUpdate >= 1000) {
    Serial.print(F("FPS: "));
    Serial.print(frameCount);
    Serial.print(F(" | WiFi state: "));
    
    switch (currentWiFiState) {
      case WIFI_DISABLED:
        Serial.print(F("DISABLED"));
        break;
      case WIFI_ENABLED_NOT_CONNECTED:
        Serial.print(F("ENABLED (not connected)"));
        break;
      case WIFI_CONNECTED:
        Serial.print(F("CONNECTED"));
        Serial.print(F(" | RSSI: "));
        Serial.print(WiFi.RSSI());
        Serial.print(F("dBm"));
        break;
      default:
        Serial.print(F("UNKNOWN"));
        break;
    }
    
    Serial.print(F(" | Time in state: "));
    Serial.print((currentMillis - lastStateChange) / 1000);
    Serial.println(F("s"));
    
    // Reset frame counter
    frameCount = 0;
    lastFpsUpdate = currentMillis;
  }
  
  // Short delay to prevent watchdog issues but keep refresh rate high
  yield();
  delay(1);
}

// Change WiFi state to next in sequence
void changeWiFiState() {
  // Move to next state
  currentWiFiState = static_cast<WiFiTestState>((currentWiFiState + 1) % NUM_WIFI_STATES);
  
  // Apply the new state
  switch (currentWiFiState) {
    case WIFI_DISABLED:
      Serial.println(F("\n--- Switching to WiFi DISABLED ---"));
      // Disconnect and turn off WiFi completely
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      WiFi.forceSleepBegin();
      break;
      
    case WIFI_ENABLED_NOT_CONNECTED:
      Serial.println(F("\n--- Switching to WiFi ENABLED (not connected) ---"));
      // Enable WiFi but don't connect
      WiFi.forceSleepWake();
      WiFi.mode(WIFI_STA);
      break;
      
    case WIFI_CONNECTED:
      Serial.println(F("\n--- Switching to WiFi CONNECTED ---"));
      // Try to connect to WiFi
      WiFi.forceSleepWake();
      WiFi.mode(WIFI_STA);
      
      // You need to change these to match your WiFi network
      const char* ssid = "YourWiFiSSID";
      const char* password = "YourWiFiPassword";
      
      Serial.print(F("Connecting to WiFi network: "));
      Serial.println(ssid);
      
      WiFi.begin(ssid, password);
      
      // Wait up to 10 seconds for connection
      int connectionAttempts = 0;
      while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
      }
      
      Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print(F("Connected! IP address: "));
        Serial.println(WiFi.localIP());
      } else {
        Serial.println(F("Failed to connect, continuing test with WiFi in connection attempt mode"));
      }
      break;
  }
  
  // Print new status
  printWiFiStatus();
}

// Print current WiFi status details
void printWiFiStatus() {
  Serial.println(F("\nCurrent WiFi Status:"));
  
  // Get WiFi mode
  uint8_t wifiMode = WiFi.getMode();
  Serial.print(F("WiFi Mode: "));
  switch (wifiMode) {
    case WIFI_OFF:
      Serial.println(F("OFF"));
      break;
    case WIFI_STA:
      Serial.println(F("STATION"));
      break;
    case WIFI_AP:
      Serial.println(F("ACCESS POINT"));
      break;
    case WIFI_AP_STA:
      Serial.println(F("AP+STATION"));
      break;
    default:
      Serial.println(F("UNKNOWN"));
      break;
  }
  
  // Get connection status
  uint8_t status = WiFi.status();
  Serial.print(F("Connection Status: "));
  switch (status) {
    case WL_CONNECTED:
      Serial.println(F("CONNECTED"));
      Serial.print(F("SSID: "));
      Serial.println(WiFi.SSID());
      Serial.print(F("IP: "));
      Serial.println(WiFi.localIP());
      Serial.print(F("RSSI: "));
      Serial.print(WiFi.RSSI());
      Serial.println(F(" dBm"));
      break;
    case WL_NO_SHIELD:
      Serial.println(F("NO SHIELD"));
      break;
    case WL_IDLE_STATUS:
      Serial.println(F("IDLE"));
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println(F("NO SSID AVAILABLE"));
      break;
    case WL_SCAN_COMPLETED:
      Serial.println(F("SCAN COMPLETED"));
      break;
    case WL_CONNECT_FAILED:
      Serial.println(F("CONNECTION FAILED"));
      break;
    case WL_CONNECTION_LOST:
      Serial.println(F("CONNECTION LOST"));
      break;
    case WL_DISCONNECTED:
      Serial.println(F("DISCONNECTED"));
      break;
    default:
      Serial.println(F("UNKNOWN"));
      break;
  }
}