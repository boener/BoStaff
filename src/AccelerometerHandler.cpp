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
  
  // Configure the accelerometer
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  // Wait for the sensor to stabilize
  delay(100);
  
  mpuInitialized = true;
  Serial.println("Accelerometer initialized");
  return true;
}

void AccelerometerHandler::update() {
  if (!mpuInitialized) return;
  
  // Get new sensor events
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Calculate magnitude of acceleration
  float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                             a.acceleration.y * a.acceleration.y + 
                             a.acceleration.z * a.acceleration.z);
  
  // Convert to raw value comparable with threshold
  uint16_t accelRaw = (uint16_t)(accelMagnitude * 100);
  
  // Check for impact (with cooldown to prevent multiple triggers)
  if (accelRaw > config->impactThreshold && 
      (millis() - lastImpactTime > impactCooldown)) {
    impactDetectedFlag = true;
    lastImpactTime = millis();
    
    Serial.print("Impact detected! Magnitude: ");
    Serial.println(accelRaw);
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
