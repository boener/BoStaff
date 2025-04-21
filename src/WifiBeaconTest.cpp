#include <Arduino.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>

// Pin definitions
#define LED_PIN D7  // GPIO13 - The problematic strip on D7
#define NUM_LEDS 10 // Using fewer LEDs for testing

// LED configuration
CRGB leds[NUM_LEDS];

// Test modes
enum TestMode {
  WIFI_OFF,
  WIFI_STATION_DISCONNECTED,
  WIFI_STATION_CONNECTED,
  WIFI_AP_MODE
};

TestMode currentMode = WIFI_OFF;
const char* modeNames[] = {
  "WiFi Off",
  "WiFi Station (Not Connected)",
  "WiFi Station (Connected)",
  "WiFi Access Point"
};

// Test parameters
unsigned long lastModeChange = 0;
const unsigned long MODE_DURATION = 30000; // 30 seconds per mode
bool modeChanged = true;

// LED pattern variables
uint8_t hue = 0;
unsigned long lastHueUpdate = 0;
unsigned long lastLedUpdate = 0;
unsigned long frameCount = 0;
unsigned long lastSecondMark = 0;

// Diagnostic counters
unsigned long renderTime = 0;
unsigned long maxRenderTime = 0;

// Function prototypes
void setupWiFiMode(TestMode mode);
void printWiFiStatus();

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println(F("ESP8266 WiFi Beacon Interference Test"));
  Serial.println(F("Testing whether WiFi beacon timing affects LED strip"));
  
  // Initialize FastLED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  FastLED.clear();
  FastLED.show();
  
  // Start with WiFi off
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  
  Serial.println(F("\nThis test will cycle through 4 WiFi modes:"));
  Serial.println(F("1. WiFi completely off"));
  Serial.println(F("2. WiFi station mode (not connected)"));
  Serial.println(F("3. WiFi station mode (connected)"));
  Serial.println(F("4. WiFi access point mode"));
  
  Serial.println(F("\nEach mode will run for 30 seconds."));
  Serial.println(F("Watch the LEDs for regular flashing patterns."));
  Serial.println(F("Press 'n' to manually switch to the next mode."));
  
  // Initialize timing variables
  lastModeChange = millis();
  lastHueUpdate = millis();
  lastLedUpdate = millis();
  lastSecondMark = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check for mode change
  if (currentMillis - lastModeChange >= MODE_DURATION) {
    // Auto-advance to next mode
    currentMode = static_cast<TestMode>((currentMode + 1) % 4);
    modeChanged = true;
    lastModeChange = currentMillis;
  }
  
  // Check for manual mode change via serial
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'n' || cmd == 'N') {
      // Manual advance to next mode
      currentMode = static_cast<TestMode>((currentMode + 1) % 4);
      modeChanged = true;
      lastModeChange = currentMillis;
    }
  }
  
  // Set up the new WiFi mode if needed
  if (modeChanged) {
    setupWiFiMode(currentMode);
    modeChanged = false;
  }
  
  // Update LED hue - slowly changing rainbow pattern
  if (currentMillis - lastHueUpdate >= 50) {
    hue++;
    lastHueUpdate = currentMillis;
  }
  
  // Set the LED pattern - simple rainbow
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue + (i * 10), 255, 255);
  }
  
  // Update LEDs at a controlled rate
  if (currentMillis - lastLedUpdate >= 33) { // ~30fps
    // Measure the time it takes to update LEDs
    unsigned long startTime = micros();
    
    // Update LEDs with interrupt disabling
    noInterrupts();
    FastLED.show();
    interrupts();
    
    // Calculate render time
    renderTime = micros() - startTime;
    if (renderTime > maxRenderTime) {
      maxRenderTime = renderTime;
    }
    
    lastLedUpdate = currentMillis;
    frameCount++;
  }
  
  // Print stats every second
  if (currentMillis - lastSecondMark >= 1000) {
    Serial.print(F("Mode: "));
    Serial.print(modeNames[currentMode]);
    Serial.print(F(" | Time in mode: "));
    Serial.print((currentMillis - lastModeChange) / 1000);
    Serial.print(F("s | FPS: "));
    Serial.print(frameCount);
    Serial.print(F(" | Render time: "));
    Serial.print(renderTime);
    Serial.print(F("µs (Max: "));
    Serial.print(maxRenderTime);
    Serial.println(F("µs)"));
    
    // Print WiFi status
    printWiFiStatus();
    
    // Reset counters
    frameCount = 0;
    lastSecondMark = currentMillis;
  }
  
  // Ensure ESP8266 background tasks get processed
  yield();
}

// Set up the specified WiFi mode
void setupWiFiMode(TestMode mode) {
  Serial.print(F("\n--- Switching to mode: "));
  Serial.print(modeNames[mode]);
  Serial.println(F(" ---"));
  
  switch (mode) {
    case WIFI_OFF:
      // Completely disable WiFi
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      WiFi.forceSleepBegin();
      break;
      
    case WIFI_STATION_DISCONNECTED:
      // Enable WiFi in station mode but don't connect
      WiFi.forceSleepWake();
      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      break;
      
    case WIFI_STATION_CONNECTED:
      // Enable WiFi and try to connect to network
      WiFi.forceSleepWake();
      WiFi.mode(WIFI_STA);
      
      // Replace with your actual WiFi credentials
      const char* ssid = "YourWiFiSSID";
      const char* password = "YourWiFiPassword";
      
      Serial.print(F("Connecting to: "));
      Serial.println(ssid);
      
      WiFi.begin(ssid, password);
      
      // Wait up to 10 seconds for connection
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      Serial.println();
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print(F("Connected! IP: "));
        Serial.println(WiFi.localIP());
      } else {
        Serial.println(F("Connection failed, continuing test anyway"));
      }
      break;
      
    case WIFI_AP_MODE:
      // Set up as an access point
      WiFi.forceSleepWake();
      WiFi.mode(WIFI_AP);
      
      // Create a simple AP with no password
      WiFi.softAP("BoStaff-Diagnostics", "");
      
      Serial.print(F("Access Point started! IP: "));
      Serial.println(WiFi.softAPIP());
      break;
  }
  
  // Reset diagnostics
  maxRenderTime = 0;
}

// Print current WiFi status information
void printWiFiStatus() {
  Serial.println(F("WiFi Status:"));
  
  // Get WiFi mode
  int wifiMode = WiFi.getMode();
  Serial.print(F("- Mode: "));
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
  
  // Print connection details if in station mode
  if (wifiMode == WIFI_STA || wifiMode == WIFI_AP_STA) {
    Serial.print(F("- Connection: "));
    
    int status = WiFi.status();
    switch (status) {
      case WL_CONNECTED:
        Serial.println(F("CONNECTED"));
        Serial.print(F("  SSID: "));
        Serial.println(WiFi.SSID());
        Serial.print(F("  RSSI: "));
        Serial.print(WiFi.RSSI());
        Serial.println(F(" dBm"));
        Serial.print(F("  IP: "));
        Serial.println(WiFi.localIP());
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
  
  // Print AP details if in AP mode
  if (wifiMode == WIFI_AP || wifiMode == WIFI_AP_STA) {
    Serial.print(F("- AP Info: "));
    Serial.print(F("SSID: BoStaff-Diagnostics | IP: "));
    Serial.print(WiFi.softAPIP());
    Serial.print(F(" | Stations connected: "));
    Serial.println(WiFi.softAPgetStationNum());
  }
  
  // Print power saving status
  Serial.print(F("- Power save mode: "));
  Serial.println(WiFi.getSleepMode() == WIFI_NONE_SLEEP ? F("NONE") : 
                 WiFi.getSleepMode() == WIFI_LIGHT_SLEEP ? F("LIGHT") : F("MODEM"));
                 
  Serial.println();
}