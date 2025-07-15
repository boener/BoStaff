#include "BoStaff.h"

// New centralized I2C configuration method
void AccelerometerHandler::configureI2C() {
  // Set up the I2C connection with proper clock speed
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(I2C_CLOCK_SPEED);
  
  // Set timeout for I2C operations to prevent lockups
  // ESP8266 Wire library uses setTimeout instead of setTimeOut
  Wire.setTimeout(I2C_TIMEOUT);
  
  // Small delay to allow I2C bus to stabilize
  delay(5);
  yield();
  
  Serial.println("I2C bus configured with:");
  Serial.print("  Clock: "); Serial.print(I2C_CLOCK_SPEED); Serial.println(" Hz");
  Serial.print("  Timeout: "); Serial.print(I2C_TIMEOUT); Serial.println(" ms");
}

bool AccelerometerHandler::begin(Config* cfg) {
  config = cfg;
  
  // Reset error counters and flags
  consecutiveErrors = 0;
  lastRecoveryAttempt = 0;
  mpuInitialized = false;
  
  // Configure I2C with centralized method
  configureI2C();
  
  Serial.println("Initializing dual-sensor accelerometer system...");
  
  // Allow the I2C bus to stabilize
  delay(10);
  yield(); // Give other processes a chance to run
  
  // Initialize MPU with improved error handling
  if (!setupMPU()) {
    Serial.println("WARNING: Failed to initialize MPU6050 - will retry in update loop");
    return false;
  }
  
  // If we got here, the MPU initialized successfully
  mpuInitialized = true;
  Serial.println("Dual-sensor system initialized with 16G accel range and 500°/s gyro range");
  Serial.println("NEW THRESHOLDS:");
  Serial.print("  Accelerometer: "); Serial.println(IMPACT_ACCEL_THRESHOLD);
  Serial.print("  Gyroscope: "); Serial.println(IMPACT_GYRO_THRESHOLD);
  Serial.print("  Rotation Classification: "); Serial.println(ROTATION_CLASSIFICATION_THRESHOLD);
  
  // Get initial reading for verification
  sensors_event_t a, g, temp;
  if (readMPUData(&a, &g, &temp)) {
    // Calculate accelerometer magnitude
    float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                              a.acceleration.y * a.acceleration.y + 
                              a.acceleration.z * a.acceleration.z);
    uint16_t accelRaw = (uint16_t)(accelMagnitude * 100);
    
    // Calculate gyroscope magnitude
    float gyroMagnitude = sqrt(g.gyro.x * g.gyro.x + 
                              g.gyro.y * g.gyro.y + 
                              g.gyro.z * g.gyro.z);
    uint16_t gyroRaw = (uint16_t)(gyroMagnitude * 100);
    
    Serial.println("Initial sensor readings:");
    Serial.print("  Accel: X="); Serial.print(a.acceleration.x);
    Serial.print(" Y="); Serial.print(a.acceleration.y);
    Serial.print(" Z="); Serial.print(a.acceleration.z);
    Serial.print(" m/s^2, Mag="); Serial.print(accelMagnitude);
    Serial.print(" m/s^2, Raw="); Serial.println(accelRaw);
    
    Serial.print("  Gyro: X="); Serial.print(g.gyro.x);
    Serial.print(" Y="); Serial.print(g.gyro.y);
    Serial.print(" Z="); Serial.print(g.gyro.z);
    Serial.print(" rad/s, Mag="); Serial.print(gyroMagnitude);
    Serial.print(" rad/s, Raw="); Serial.println(gyroRaw);
  }
  else {
    Serial.println("WARNING: Initial sensor reading failed");
  }
  
  return mpuInitialized;
}

bool AccelerometerHandler::setupMPU() {
  // Try to initialize the MPU with multiple attempts
  for (int attempt = 0; attempt < I2C_RETRY_COUNT; attempt++) {
    if (attempt > 0) {
      Serial.print("Retrying MPU setup (attempt ");
      Serial.print(attempt + 1);
      Serial.print(" of ");
      Serial.print(I2C_RETRY_COUNT);
      Serial.println(")");
      
      // Short delay and yield between attempts
      delay(10 * attempt); // Increasing delay for each retry
      yield();
      
      // Reconfigure I2C bus before retry
      configureI2C();
    }
    
    if (mpu.begin()) {
      // Successfully initialized, now configure the settings
      // (I2C is already configured by configureI2C())
      
      // Configure the accelerometer with error handling
      bool configSuccess = true;
      
      // Set the accelerometer range with retry
      bool accelRangeSet = false;
      for (int i = 0; i < I2C_RETRY_COUNT && !accelRangeSet; i++) {
        if (i > 0) {
          Serial.print("Retrying accelerometer range setting (attempt ");
          Serial.print(i + 1); Serial.println(")");
          delay(10 * i); // Increasing delay for each retry
          yield();
        }
        
        // The setter method returns void, so we need to verify by reading back
        mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
        
        // Add short delay to allow I2C operation to complete
        delay(5);
        yield();
        
        // Verify setting by reading back the register
        // This is the key improvement - verification step
        sensors_event_t a, g, temp;
        if (mpu.getEvent(&a, &g, &temp)) {
          // Successfully read sensor data, assume setting worked
          accelRangeSet = true;
          
          // Debug output
          Serial.println("Accelerometer range set successfully");
        }
      }
      
      // Set the gyro range with retry
      bool gyroRangeSet = false;
      for (int i = 0; i < I2C_RETRY_COUNT && !gyroRangeSet; i++) {
        if (i > 0) {
          Serial.print("Retrying gyro range setting (attempt ");
          Serial.print(i + 1); Serial.println(")");
          delay(10 * i);
          yield();
        }
        
        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        
        // Add short delay to allow I2C operation to complete
        delay(5);
        yield();
        
        // Verify setting by reading back sensor data
        sensors_event_t a, g, temp;
        if (mpu.getEvent(&a, &g, &temp)) {
          gyroRangeSet = true;
          Serial.println("Gyro range set successfully");
        }
      }
      
      // Set the filter bandwidth with retry
      bool filterBandwidthSet = false;
      for (int i = 0; i < I2C_RETRY_COUNT && !filterBandwidthSet; i++) {
        if (i > 0) {
          Serial.print("Retrying filter bandwidth setting (attempt ");
          Serial.print(i + 1); Serial.println(")");
          delay(10 * i);
          yield();
        }
        
        mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        
        // Add short delay to allow I2C operation to complete
        delay(5);
        yield();
        
        // Verify setting by reading back sensor data
        sensors_event_t a, g, temp;
        if (mpu.getEvent(&a, &g, &temp)) {
          filterBandwidthSet = true;
          Serial.println("Filter bandwidth set successfully");
        }
      }
      
      // Only consider configuration successful if all three settings were applied
      configSuccess = accelRangeSet && gyroRangeSet && filterBandwidthSet;
      
      if (!configSuccess) {
        Serial.println("WARNING: Some MPU6050 settings could not be verified");
      }
      
      // Wait for the sensor to stabilize, with yield to prevent WDT reset
      for (int i = 0; i < 10; i++) {
        delay(10);
        yield();
      }
      
      // Return true since all configuration appeared to succeed
      return configSuccess;
    }
    
    // If we get here, initialization failed on this attempt
    Serial.println("MPU initialization attempt failed");
  }
  
  // If we've tried all attempts and still failed
  Serial.println("ERROR: Failed to find or initialize MPU6050 after multiple attempts");
  return false;
}

bool AccelerometerHandler::readMPUData(sensors_event_t* a, sensors_event_t* g, sensors_event_t* temp) {
  // Don't even try if not initialized
  if (!mpuInitialized) {
    return false;
  }
  
  // Try to read data with retry
  for (int attempt = 0; attempt < I2C_RETRY_COUNT; attempt++) {
    if (attempt > 0) {
      // Small delay between retries with yield
      delay(5);
      yield(); 
      
      // Ensure I2C timeout is set for each attempt
      Wire.setTimeout(I2C_TIMEOUT);
    }
    
    // Try to get event
    if (mpu.getEvent(a, g, temp)) {
      // Successful read, reset error counter
      if (consecutiveErrors > 0) {
        consecutiveErrors = 0;
        Serial.println("I2C communication recovered");
      }
      return true;
    }
  }
  
  // If we get here, all read attempts failed
  consecutiveErrors++;
  
  // If we have too many consecutive errors, try recovery
  if (consecutiveErrors >= 5) {
    // Don't try recovery too frequently
    if (millis() - lastRecoveryAttempt > 5000) {
      Serial.print("WARNING: Multiple I2C read failures (");
      Serial.print(consecutiveErrors);
      Serial.println(") - attempting recovery");
      
      // Try to recover the I2C bus
      if (recoverI2C()) {
        // Reset error counter if recovery successful
        consecutiveErrors = 0; 
      }
      
      lastRecoveryAttempt = millis();
    }
  }
  
  return false;
}

bool AccelerometerHandler::recoverI2C() {
  Serial.println("Attempting I2C bus recovery...");
  
  // Try to reset the MPU6050 by re-initializing
  mpuInitialized = false;
  
  // The ESP8266 Wire library doesn't have an end() method
  // Instead, we'll just reinitialize
  delay(50);
  yield();
  
  // Reconfigure the I2C bus using centralized method
  configureI2C();
  
  delay(50);
  yield();
  
  // Try to re-initialize the MPU
  bool success = setupMPU();
  
  if (success) {
    mpuInitialized = true;
    Serial.println("I2C bus recovery successful");
  } else {
    Serial.println("WARNING: I2C bus recovery failed, will retry later");
  }
  
  return success;
}

void AccelerometerHandler::update() {
  // If not initialized, attempt to initialize
  if (!mpuInitialized) {
    // Don't attempt initialization too frequently
    static unsigned long lastInitAttempt = 0;
    if (millis() - lastInitAttempt > 5000) {
      Serial.println("Accelerometer not initialized, attempting to restart");
      begin(config);
      lastInitAttempt = millis();
    }
    return;
  }
  
  // Get new sensor events with enhanced error handling
  sensors_event_t a, g, temp;
  
  if (!readMPUData(&a, &g, &temp)) {
    // Failed to read data
    Serial.println("Failed to read from MPU6050");
    return;
  }
  
  // DUAL-SENSOR IMPACT DETECTION IMPLEMENTATION
  
  // Calculate accelerometer magnitude
  float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                             a.acceleration.y * a.acceleration.y + 
                             a.acceleration.z * a.acceleration.z);
  uint16_t accelRaw = (uint16_t)(accelMagnitude * 100);
  
  // Calculate gyroscope magnitude (NEW)
  float gyroMagnitude = sqrt(g.gyro.x * g.gyro.x + 
                            g.gyro.y * g.gyro.y + 
                            g.gyro.z * g.gyro.z);
  uint16_t gyroRaw = (uint16_t)(gyroMagnitude * 100);
  
  // NEW DUAL-SENSOR DETECTION LOGIC: (AccelMag > 7500) || (GyroMag > 1200)
  bool accelThresholdExceeded = (accelRaw > IMPACT_ACCEL_THRESHOLD);
  bool gyroThresholdExceeded = (gyroRaw > IMPACT_GYRO_THRESHOLD);
  bool impactDetectedByEither = accelThresholdExceeded || gyroThresholdExceeded;
  
  // Check for impact with cooldown to prevent multiple triggers
  if (impactDetectedByEither && (millis() - lastImpactTime > impactCooldown)) {
    
    // IMPACT CLASSIFICATION LOGIC
    ImpactType detectedImpactType;
    const char* impactTypeString;
    
    if (gyroRaw >= ROTATION_CLASSIFICATION_THRESHOLD) {
      detectedImpactType = IMPACT_ROTATION;
      impactTypeString = "ROTATION";
      config->totalRotationImpacts++;
    } else {
      detectedImpactType = IMPACT_STAB;
      impactTypeString = "STAB";
      config->totalStabImpacts++;
    }
    
    // Update config with impact classification
    config->lastImpactType = detectedImpactType;
    
    // Set impact detected flag
    impactDetectedFlag = true;
    lastImpactTime = millis();
    
    // ENHANCED SERIAL OUTPUT
    Serial.println("!!! DUAL-SENSOR IMPACT DETECTED !!!");
    Serial.print("  Type: "); Serial.println(impactTypeString);
    Serial.print("  Accel: "); Serial.print(accelRaw);
    Serial.print(" (Threshold: "); Serial.print(IMPACT_ACCEL_THRESHOLD);
    Serial.print(", Exceeded: "); Serial.print(accelThresholdExceeded ? "YES" : "NO");
    Serial.println(")");
    Serial.print("  Gyro: "); Serial.print(gyroRaw);
    Serial.print(" (Threshold: "); Serial.print(IMPACT_GYRO_THRESHOLD);
    Serial.print(", Exceeded: "); Serial.print(gyroThresholdExceeded ? "YES" : "NO");
    Serial.println(")");
    Serial.print("  Classification: GyroMag "); Serial.print(gyroRaw);
    Serial.print(gyroRaw >= ROTATION_CLASSIFICATION_THRESHOLD ? " >= " : " < ");
    Serial.print(ROTATION_CLASSIFICATION_THRESHOLD); Serial.print(" = "); Serial.println(impactTypeString);
    Serial.print("  Impact Counts - Stabs: "); Serial.print(config->totalStabImpacts);
    Serial.print(", Rotations: "); Serial.println(config->totalRotationImpacts);
    
  } else {
    // No impact detected
    impactDetectedFlag = false;
    
    // Optional debug output (commented out for performance)
    /*
    Serial.print("No impact - Accel: "); Serial.print(accelRaw);
    Serial.print(", Gyro: "); Serial.print(gyroRaw);
    Serial.print(", Cooldown: "); Serial.println(millis() - lastImpactTime <= impactCooldown ? "ACTIVE" : "INACTIVE");
    */
  }
  
  // Make sure we don't hog the CPU
  yield();
}

bool AccelerometerHandler::impactDetected() {
  // Return and clear the impact flag
  bool result = impactDetectedFlag;
  if (result) {
    Serial.println("Impact flag checked and returned TRUE");
  }
  impactDetectedFlag = false;
  return result;
}

// OLD CALIBRATION METHODS REMOVED FOR DUAL-SENSOR SYSTEM
/*
void AccelerometerHandler::calibrate() {
  // CALIBRATION SYSTEM REMOVED - Dual-sensor system uses fixed thresholds
  // based on extensive data analysis. No calibration needed.
  Serial.println("NOTICE: Calibration system removed in dual-sensor implementation.");
  Serial.println("Using optimized fixed thresholds based on data analysis:");
  Serial.print("  Accelerometer threshold: "); Serial.println(IMPACT_ACCEL_THRESHOLD);
  Serial.print("  Gyroscope threshold: "); Serial.println(IMPACT_GYRO_THRESHOLD);
  Serial.print("  Rotation classification: "); Serial.println(ROTATION_CLASSIFICATION_THRESHOLD);
}

void AccelerometerHandler::waitForButtonPress() {
  // Helper method removed with calibration system
}
*/