#include "BoStaff.h"

/**
 * Initialize the button handler
 */
void ButtonHandler::begin(Config* cfg) {
  config = cfg;
  
  // Set up button pin
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  // Initialize state variables
  buttonState = false;
  lastButtonState = false;
  lastDebounceTime = 0;
  modeChange = false;
  
  Serial.println(F("Button handler initialized"));
}

/**
 * Handle button input with debouncing
 */
void ButtonHandler::handle() {
  // Read the current state of the button
  // Using INPUT_PULLUP, so pin reads LOW when button is pressed
  bool reading = !digitalRead(BTN_PIN);
  
  // Reset mode change flag
  modeChange = false;
  
  // If the button state changed, due to noise or pressing
  if (reading != lastButtonState) {
    // Reset the debouncing timer
    lastDebounceTime = millis();
  }
  
  // Check if enough time has passed to consider the reading stable
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed
    if (reading != buttonState) {
      buttonState = reading;
      
      // Button press detected (on press, not release)
      if (buttonState) {
        modeChange = true;
        Serial.println(F("Button pressed - mode change requested"));
      }
    }
  }
  
  // Save the reading for next comparison
  lastButtonState = reading;
}

/**
 * Check if a mode change was requested
 */
bool ButtonHandler::modeChangeRequested() {
  bool result = modeChange;
  modeChange = false; // Reset the flag after reading
  return result;
}