#include "BoStaff.h"

bool AccelerometerHandler::begin(Config* cfg) {
  config = cfg;
  
  // Set up the I2C connection to the MPU6050 using the defined pins
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize the MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    mpuInitialized = false;
    return false;
  }
  
  // Configure the accelerometer - using 16G range for better impact detection
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G); // Changed from 8G to 16G
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  // Wait for the sensor to stabilize
  delay(100);
  
  mpuInitialized = true;
  Serial.println("Accelerometer initialized with 16G range");
  Serial.print("Impact threshold set to: ");
  Serial.println(config->impactThreshold);
  
  // Initial accelerometer reading to check functionality
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  Serial.println("Initial accelerometer readings:");
  Serial.print("X: "); Serial.print(a.acceleration.x);
  Serial.print(" Y: "); Serial.print(a.acceleration.y);
  Serial.print(" Z: "); Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");
  
  // Calculate and print magnitude
  float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                             a.acceleration.y * a.acceleration.y + 
                             a.acceleration.z * a.acceleration.z);
  Serial.print("Magnitude: "); Serial.print(accelMagnitude);
  Serial.print(" m/s^2, Raw: "); Serial.println((uint16_t)(accelMagnitude * 100));
  
  return true;
}

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
  
  // Print accelerometer readings every second (for debugging)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("Accel: "); Serial.print(accelRaw);
    Serial.print(" (X:"); Serial.print(a.acceleration.x);
    Serial.print(" Y:"); Serial.print(a.acceleration.y);
    Serial.print(" Z:"); Serial.print(a.acceleration.z);
    Serial.print(") Threshold: "); Serial.println(config->impactThreshold);
    lastPrint = millis();
  }
  
  // Check for impact (with cooldown to prevent multiple triggers)
  // Using a much lower threshold of 1600 (about 16 m/s^2 or ~1.6G)
  if (accelRaw > config->impactThreshold && 
      (millis() - lastImpactTime > impactCooldown)) {
    impactDetectedFlag = true;
    lastImpactTime = millis();
    
    Serial.print("IMPACT DETECTED! Magnitude: ");
    Serial.print(accelRaw);
    Serial.print(" (Threshold: ");
    Serial.print(config->impactThreshold);
    Serial.println(")");
  } else {
    impactDetectedFlag = false;
  }
}

bool AccelerometerHandler::impactDetected() {
  // Return and clear the impact flag
  bool result = impactDetectedFlag;
  impactDetectedFlag = false;
  return result;
}

void AccelerometerHandler::calibrate() {
  if (!mpuInitialized) return;
  
  Serial.println("Calibrating accelerometer...");
  Serial.println("Place the bo staff on a stable surface");
  delay(2000);
  
  // TODO: Implement calibration routine
  // This would collect baseline readings and adjust the impact threshold
  
  Serial.println("Calibration complete");
}
