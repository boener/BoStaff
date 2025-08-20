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
  
  DEBUG_PRINTLN("I2C bus configured with:");
  DEBUG_PRINT("  Clock: "); DEBUG_PRINT(I2C_CLOCK_SPEED); DEBUG_PRINTLN(" Hz");
  DEBUG_PRINT("  Timeout: "); DEBUG_PRINT(I2C_TIMEOUT); DEBUG_PRINTLN(" ms");
}

bool AccelerometerHandler::begin(Config* cfg) {
  config = cfg;
  
  // Reset error counters and flags
  consecutiveErrors = 0;
  lastRecoveryAttempt = 0;
  mpuInitialized = false;
  
  // Initialize gyro delta detection variables
  previousGyroRaw = 0;
  firstReading = true;
  
  // Configure I2C with centralized method
  configureI2C();
  
  DEBUG_PRINTLN("Initializing dual-sensor accelerometer system...");
  
  // Allow the I2C bus to stabilize
  delay(10);
  yield(); // Give other processes a chance to run
  
  // Initialize MPU with improved error handling
  if (!setupMPU()) {
    DEBUG_PRINTLN("WARNING: Failed to initialize MPU6050 - will retry in update loop");
    return false;
  }
  
  // If we got here, the MPU initialized successfully
  mpuInitialized = true;
  DEBUG_PRINTLN("Dual-sensor system initialized with 16G accel range and 500°/s gyro range");
  DEBUG_PRINTLN("HYBRID ACCELEROMETER DETECTION THRESHOLDS:");
  DEBUG_PRINT("  Total Magnitude: "); DEBUG_PRINTLN(IMPACT_ACCEL_THRESHOLD);
  DEBUG_PRINT("  Individual Axis: "); DEBUG_PRINTLN(IMPACT_INDIVIDUAL_AXIS_THRESHOLD);
  DEBUG_PRINT("  Gyroscope SUDDEN STOP: "); DEBUG_PRINTLN(IMPACT_GYRO_DELTA_THRESHOLD);
  DEBUG_PRINT("  Rotation Classification: "); DEBUG_PRINTLN(ROTATION_CLASSIFICATION_THRESHOLD);
  
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
    
    // Initialize previous reading for delta detection
    previousGyroRaw = gyroRaw;
    
    DEBUG_PRINTLN("Initial sensor readings:");
    DEBUG_PRINT("  Accel: X="); DEBUG_PRINT(a.acceleration.x);
    DEBUG_PRINT(" Y="); DEBUG_PRINT(a.acceleration.y);
    DEBUG_PRINT(" Z="); DEBUG_PRINT(a.acceleration.z);
    DEBUG_PRINT(" m/s^2, Mag="); DEBUG_PRINT(accelMagnitude);
    DEBUG_PRINT(" m/s^2, Raw="); DEBUG_PRINTLN(accelRaw);
    
    DEBUG_PRINT("  Gyro: X="); DEBUG_PRINT(g.gyro.x);
    DEBUG_PRINT(" Y="); DEBUG_PRINT(g.gyro.y);
    DEBUG_PRINT(" Z="); DEBUG_PRINT(g.gyro.z);
    DEBUG_PRINT(" rad/s, Mag="); DEBUG_PRINT(gyroMagnitude);
    DEBUG_PRINT(" rad/s, Raw="); DEBUG_PRINTLN(gyroRaw);
    DEBUG_PRINTLN("  Hybrid accelerometer detection initialized");
  }
  else {
    DEBUG_PRINTLN("WARNING: Initial sensor reading failed");
  }
  
  return mpuInitialized;
}

bool AccelerometerHandler::setupMPU() {
  // Try to initialize the MPU with multiple attempts
  for (int attempt = 0; attempt < I2C_RETRY_COUNT; attempt++) {
    if (attempt > 0) {
      DEBUG_PRINT("Retrying MPU setup (attempt ");
      DEBUG_PRINT(attempt + 1);
      DEBUG_PRINT(" of ");
      DEBUG_PRINT(I2C_RETRY_COUNT);
      DEBUG_PRINTLN(")");
      
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
          DEBUG_PRINT("Retrying accelerometer range setting (attempt ");
          DEBUG_PRINT(i + 1); DEBUG_PRINTLN(")");
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
          DEBUG_PRINTLN("Accelerometer range set successfully");
        }
      }
      
      // Set the gyro range with retry
      bool gyroRangeSet = false;
      for (int i = 0; i < I2C_RETRY_COUNT && !gyroRangeSet; i++) {
        if (i > 0) {
          DEBUG_PRINT("Retrying gyro range setting (attempt ");
          DEBUG_PRINT(i + 1); DEBUG_PRINTLN(")");
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
          DEBUG_PRINTLN("Gyro range set successfully");
        }
      }
      
      // Set the filter bandwidth with retry
      bool filterBandwidthSet = false;
      for (int i = 0; i < I2C_RETRY_COUNT && !filterBandwidthSet; i++) {
        if (i > 0) {
          DEBUG_PRINT("Retrying filter bandwidth setting (attempt ");
          DEBUG_PRINT(i + 1); DEBUG_PRINTLN(")");
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
          DEBUG_PRINTLN("Filter bandwidth set successfully");
        }
      }
      
      // Only consider configuration successful if all three settings were applied
      configSuccess = accelRangeSet && gyroRangeSet && filterBandwidthSet;
      
      if (!configSuccess) {
        DEBUG_PRINTLN("WARNING: Some MPU6050 settings could not be verified");
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
    DEBUG_PRINTLN("MPU initialization attempt failed");
  }
  
  // If we've tried all attempts and still failed
  DEBUG_PRINTLN("ERROR: Failed to find or initialize MPU6050 after multiple attempts");
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
        DEBUG_PRINTLN("I2C communication recovered");
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
      DEBUG_PRINT("WARNING: Multiple I2C read failures (");
      DEBUG_PRINT(consecutiveErrors);
      DEBUG_PRINTLN(") - attempting recovery");
      
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
  DEBUG_PRINTLN("Attempting I2C bus recovery...");
  
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
    // Reset gyro sudden stop detection variables after recovery
    previousGyroRaw = 0;
    firstReading = true;
    DEBUG_PRINTLN("I2C bus recovery successful");
  } else {
    DEBUG_PRINTLN("WARNING: I2C bus recovery failed, will retry later");
  }
  
  return success;
}

void AccelerometerHandler::update() {
  // If not initialized, attempt to initialize
  if (!mpuInitialized) {
    // Don't attempt initialization too frequently
    static unsigned long lastInitAttempt = 0;
    if (millis() - lastInitAttempt > 5000) {
      DEBUG_PRINTLN("Accelerometer not initialized, attempting to restart");
      begin(config);
      lastInitAttempt = millis();
    }
    return;
  }
  
  // Get new sensor events with enhanced error handling
  sensors_event_t a, g, temp;
  
  if (!readMPUData(&a, &g, &temp)) {
    // Failed to read data - using PERF_DEBUG for high-frequency output
    PERF_DEBUG_PRINTLN("Failed to read from MPU6050");
    return;
  }
  
  // DUAL-SENSOR IMPACT DETECTION IMPLEMENTATION WITH GYRO SUDDEN STOP DETECTION
  
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
  
  // Calculate gyro sudden stop detection (only detects decreases in rotation speed)
  uint16_t gyroSuddenStop = 0;
  if (!firstReading) {
    // Calculate change: positive value means rotation slowed down (what we want to detect)
    int16_t gyroChange = (int16_t)previousGyroRaw - (int16_t)gyroRaw;
    gyroSuddenStop = (gyroChange > 0) ? gyroChange : 0; // Only count decreases (sudden stops)
  } else {
    firstReading = false; // Mark that we've had our first reading
  }
  
  // Store current reading for next delta calculation
  previousGyroRaw = gyroRaw;
  
  // HYBRID ACCELEROMETER DETECTION LOGIC:
  // Detects impacts through EITHER total magnitude OR individual axis exceeding thresholds
  bool totalMagnitudeExceeded = (accelRaw > IMPACT_ACCEL_THRESHOLD);
  float maxAxis = max(max(abs(a.acceleration.x), abs(a.acceleration.y)), abs(a.acceleration.z));
  uint16_t maxAxisRaw = (uint16_t)(maxAxis * 100);
  bool individualAxisExceeded = (maxAxisRaw > IMPACT_INDIVIDUAL_AXIS_THRESHOLD);
  bool accelThresholdExceeded = totalMagnitudeExceeded || individualAxisExceeded;
  
  // Continue with existing gyroscope detection (unchanged)
  bool gyroSuddenStopDetected = (gyroSuddenStop > IMPACT_GYRO_DELTA_THRESHOLD);
  bool impactDetectedByEither = accelThresholdExceeded || gyroSuddenStopDetected;
  
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
    
    // Set impact detected flag - FIXED: Only set if not already set
    // This prevents losing an impact between update() and impactDetected() calls
    if (!impactDetectedFlag) {
      impactDetectedFlag = true;
      
      // ENHANCED SERIAL OUTPUT WITH HYBRID ACCELEROMETER INFO
      PERF_DEBUG_PRINTLN("!!! HYBRID ACCELEROMETER IMPACT DETECTED !!!");
      PERF_DEBUG_PRINT("  Type: "); PERF_DEBUG_PRINTLN(impactTypeString);
      PERF_DEBUG_PRINT("  Total Magnitude: "); PERF_DEBUG_PRINT(accelRaw);
      PERF_DEBUG_PRINT(" (Threshold: "); PERF_DEBUG_PRINT(IMPACT_ACCEL_THRESHOLD);
      PERF_DEBUG_PRINT(", Exceeded: "); PERF_DEBUG_PRINT(totalMagnitudeExceeded ? "YES" : "NO");
      PERF_DEBUG_PRINTLN(")");
      PERF_DEBUG_PRINT("  Max Individual Axis: "); PERF_DEBUG_PRINT(maxAxisRaw);
      PERF_DEBUG_PRINT(" (Threshold: "); PERF_DEBUG_PRINT(IMPACT_INDIVIDUAL_AXIS_THRESHOLD);
      PERF_DEBUG_PRINT(", Exceeded: "); PERF_DEBUG_PRINT(individualAxisExceeded ? "YES" : "NO");
      PERF_DEBUG_PRINTLN(")");
      PERF_DEBUG_PRINT("  Gyro: "); PERF_DEBUG_PRINT(gyroRaw);
      PERF_DEBUG_PRINT(" (Previous: "); PERF_DEBUG_PRINT(previousGyroRaw); PERF_DEBUG_PRINTLN(")");
      PERF_DEBUG_PRINT("  Gyro SUDDEN STOP: "); PERF_DEBUG_PRINT(gyroSuddenStop);
      PERF_DEBUG_PRINT(" (Threshold: "); PERF_DEBUG_PRINT(IMPACT_GYRO_DELTA_THRESHOLD);
      PERF_DEBUG_PRINT(", Detected: "); PERF_DEBUG_PRINT(gyroSuddenStopDetected ? "YES" : "NO");
      PERF_DEBUG_PRINTLN(")");
      PERF_DEBUG_PRINT("  Classification: GyroMag "); PERF_DEBUG_PRINT(gyroRaw);
      PERF_DEBUG_PRINT(gyroRaw >= ROTATION_CLASSIFICATION_THRESHOLD ? " >= " : " < ");
      PERF_DEBUG_PRINT(ROTATION_CLASSIFICATION_THRESHOLD); PERF_DEBUG_PRINT(" = "); PERF_DEBUG_PRINTLN(impactTypeString);
      PERF_DEBUG_PRINT("  Impact Counts - Stabs: "); PERF_DEBUG_PRINT(config->totalStabImpacts);
      PERF_DEBUG_PRINT(", Rotations: "); PERF_DEBUG_PRINTLN(config->totalRotationImpacts);
    }
    
    lastImpactTime = millis();
    
  }
  // Note: We no longer clear the flag here - it's only cleared when checked by impactDetected()
  
  // Optional debug output (enabled for debugging) - using PERF_DEBUG for performance-critical output
  /*
  if (!impactDetectedFlag) {
    PERF_DEBUG_PRINT("No impact - TotalMag: "); PERF_DEBUG_PRINT(accelRaw);
    PERF_DEBUG_PRINT(", MaxAxis: "); PERF_DEBUG_PRINT(maxAxisRaw);
    PERF_DEBUG_PRINT(", GyroMag: "); PERF_DEBUG_PRINT(gyroRaw);
    PERF_DEBUG_PRINT(", GyroSuddenStop: "); PERF_DEBUG_PRINT(gyroSuddenStop);
    PERF_DEBUG_PRINT(", Cooldown: "); PERF_DEBUG_PRINTLN(millis() - lastImpactTime <= impactCooldown ? "ACTIVE" : "INACTIVE");
  }
  */

  // Make sure we don't hog the CPU
  yield();
}

bool AccelerometerHandler::impactDetected() {
  // FIXED: Atomic read and clear operation to prevent race conditions
  bool result = impactDetectedFlag;
  if (result) {
    impactDetectedFlag = false;  // Clear the flag after reading
    PERF_DEBUG_PRINTLN("Impact flag checked and cleared");
  }
  return result;
}

// OLD CALIBRATION METHODS REMOVED FOR DUAL-SENSOR SYSTEM
/*
void AccelerometerHandler::calibrate() {
  // CALIBRATION SYSTEM REMOVED - Dual-sensor system uses fixed thresholds
  // based on extensive data analysis. No calibration needed.
  DEBUG_PRINTLN("NOTICE: Calibration system removed in dual-sensor implementation.");
  DEBUG_PRINTLN("Using optimized fixed thresholds based on data analysis:");
  DEBUG_PRINT("  Accelerometer threshold: "); DEBUG_PRINTLN(IMPACT_ACCEL_THRESHOLD);
  DEBUG_PRINT("  Gyroscope sudden stop threshold: "); DEBUG_PRINTLN(IMPACT_GYRO_DELTA_THRESHOLD);
  DEBUG_PRINT("  Rotation classification: "); DEBUG_PRINTLN(ROTATION_CLASSIFICATION_THRESHOLD);
}

void AccelerometerHandler::waitForButtonPress() {
  // Helper method removed with calibration system
}
*/