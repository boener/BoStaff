#include "BoStaff.h"

// Rest of the existing code remains the same...

void AccelerometerHandler::update() {
  if (!mpuInitialized) {
    Serial.println("Accelerometer not initialized, attempt to restart");
    begin(config); // Try to reinitialize
    return;
  }
  
  // Get new sensor events
  sensors_event_t a, g, temp;
  if (!mpu.getEvent(&a, &g, &temp)) {
    Serial.println("Failed to read from MPU6050");
    return;
  }
  
  // Calculate magnitude of acceleration
  float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                             a.acceleration.y * a.acceleration.y + 
                             a.acceleration.z * a.acceleration.z);
  
  // Convert to raw value comparable with threshold
  uint16_t accelRaw = (uint16_t)(accelMagnitude * 100);
  
  // Hard-coded lower threshold for testing
  uint16_t testThreshold = 10000;  // Adjust this value if needed
  
  // Print accelerometer readings every second (for debugging)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("Accel: "); Serial.print(accelRaw);
    Serial.print(" (X:"); Serial.print(a.acceleration.x);
    Serial.print(" Y:"); Serial.print(a.acceleration.y);
    Serial.print(" Z:"); Serial.print(a.acceleration.z);
    Serial.print(") Threshold: "); Serial.println(testThreshold);
    lastPrint = millis();
  }
  
  // Enhance impact detection logging
  Serial.print("DEBUG: accelRaw = "); Serial.print(accelRaw);
  Serial.print(", Test Threshold = "); Serial.print(testThreshold);
  Serial.print(", Last Impact Time = "); Serial.print(lastImpactTime);
  Serial.print(", Current Time = "); Serial.print(millis());
  Serial.print(", Cooldown = "); Serial.println(impactCooldown);
  
  // Check for impact (with cooldown to prevent multiple triggers)
  if (accelRaw > testThreshold && 
      (millis() - lastImpactTime > impactCooldown)) {
    impactDetectedFlag = true;
    lastImpactTime = millis();
    
    Serial.println("!!! IMPACT DETECTED !!!");
    Serial.print("Magnitude: "); Serial.print(accelRaw);
    Serial.print(" (Threshold: "); Serial.print(testThreshold);
    Serial.println(")");
  } else {
    // Only log when impact is not detected for additional insights
    if (accelRaw <= testThreshold) {
      Serial.println("No impact: Acceleration below threshold");
    }
    
    if (millis() - lastImpactTime <= impactCooldown) {
      Serial.println("No impact: Within cooldown period");
    }
    
    impactDetectedFlag = false;
  }
}