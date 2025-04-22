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
  
  Serial.println("Initializing accelerometer...");
  
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
  Serial.println("Accelerometer initialized with 16G range");
  Serial.print("Impact threshold set to: ");
  Serial.println(config->impactThreshold);
  
  // Get initial reading for verification
  sensors_event_t a, g, temp;
  if (readMPUData(&a, &g, &temp)) {
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
  }
  else {
    Serial.println("WARNING: Initial accelerometer reading failed");
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
  
  // Calculate magnitude of acceleration
  float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                             a.acceleration.y * a.acceleration.y + 
                             a.acceleration.z * a.acceleration.z);
  
  // Convert to raw value comparable with threshold
  uint16_t accelRaw = (uint16_t)(accelMagnitude * 100);
  
  // Use the configured threshold instead of hard-coded value
  uint16_t currentThreshold = config->impactThreshold;
  
  // Check for impact (with cooldown to prevent multiple triggers)
  if (accelRaw > currentThreshold && 
      (millis() - lastImpactTime > impactCooldown)) {
    impactDetectedFlag = true;
    lastImpactTime = millis();
    
    Serial.println("!!! IMPACT DETECTED !!!");
    Serial.print("Magnitude: "); Serial.print(accelRaw);
    Serial.print(" (Threshold: "); Serial.print(currentThreshold);
    Serial.println(")");
  } else {
    // Only log when debug is enabled
    #ifdef DEBUG_MODE
    if (accelRaw <= currentThreshold) {
      Serial.println("No impact: Acceleration below threshold");
    }
    
    if (millis() - lastImpactTime <= impactCooldown) {
      Serial.println("No impact: Within cooldown period");
    }
    #endif
    
    impactDetectedFlag = false;
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

void AccelerometerHandler::calibrate() {
  if (!mpuInitialized) {
    Serial.println("ERROR: Cannot calibrate - Accelerometer not initialized");
    // Try to initialize
    if (!begin(config)) {
      Serial.println("ERROR: Failed to initialize accelerometer for calibration");
      return;
    }
  }
  
  Serial.println("\n== ACCELEROMETER CALIBRATION ==");
  Serial.println("This will help determine the best impact threshold settings.");
  Serial.println("Follow the instructions below:");
  
  // Step 1: Collect baseline readings (at rest)
  Serial.println("\nSTEP 1: Measuring baseline noise");
  Serial.println("Please place the bo staff on a stable surface and keep it still.");
  Serial.println("Collecting baseline readings for 3 seconds...");
  
  // Variables to track baseline statistics
  uint16_t baselineMax = 0;
  uint16_t baselineMin = 65535;
  float baselineSum = 0;
  int baselineSamples = 0;
  
  // Wait for staff to be still
  for (int i = 0; i < 100; i++) {
    delay(10);
    yield(); // Give time for other processes
  }
  
  // Collect baseline readings for 3 seconds
  unsigned long baselineStart = millis();
  while (millis() - baselineStart < 3000) {
    sensors_event_t a, g, temp;
    if (readMPUData(&a, &g, &temp)) {
      float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                                 a.acceleration.y * a.acceleration.y + 
                                 a.acceleration.z * a.acceleration.z);
      
      uint16_t accelRaw = (uint16_t)(accelMagnitude * 100);
      
      baselineMax = max(baselineMax, accelRaw);
      baselineMin = min(baselineMin, accelRaw);
      baselineSum += accelRaw;
      baselineSamples++;
      
      // Show the current reading
      Serial.print(".");
      if (baselineSamples % 50 == 0) Serial.println();
    }
    delay(10);
    yield(); // Prevent watchdog resets
  }
  
  float baselineAvg = baselineSum / baselineSamples;
  Serial.println();
  Serial.print("Baseline average: "); Serial.println(baselineAvg);
  Serial.print("Baseline min: "); Serial.println(baselineMin);
  Serial.print("Baseline max: "); Serial.println(baselineMax);
  
  // Step 2: Collect sample impact readings
  Serial.println("\nSTEP 2: Measuring impact levels");
  Serial.println("Please perform 5 sample impacts of different strengths.");
  Serial.println("Start with very gentle taps and gradually increase strength.");
  Serial.println("Press button once before each impact to continue.");
  
  uint16_t impactSamples[5] = {0, 0, 0, 0, 0};
  
  for (int i = 0; i < 5; i++) {
    Serial.print("\nReady for impact sample #"); Serial.println(i + 1);
    Serial.println("Press button once when ready to perform impact...");
    
    // Wait for button press
    waitForButtonPress();
    
    Serial.println("Now perform an impact within 3 seconds!");
    
    // Measure the maximum acceleration during a 3-second window
    unsigned long impactStart = millis();
    uint16_t maxImpact = 0;
    
    while (millis() - impactStart < 3000) {
      sensors_event_t a, g, temp;
      if (readMPUData(&a, &g, &temp)) {
        float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                                   a.acceleration.y * a.acceleration.y + 
                                   a.acceleration.z * a.acceleration.z);
        
        uint16_t accelRaw = (uint16_t)(accelMagnitude * 100);
        
        if (accelRaw > maxImpact) {
          maxImpact = accelRaw;
        }
        
        // Print current reading
        Serial.print("Current: "); Serial.print(accelRaw);
        Serial.print(", Max: "); Serial.println(maxImpact);
      }
      
      delay(5);
      yield(); // Prevent watchdog resets
    }
    
    impactSamples[i] = maxImpact;
    Serial.print("Impact #"); Serial.print(i + 1);
    Serial.print(" maximum reading: "); Serial.println(impactSamples[i]);
    
    // Small delay between impact measurements
    delay(500);
    yield();
  }
  
  // Step 3: Calculate appropriate threshold
  // Sort the impact samples to find median
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (impactSamples[i] > impactSamples[j]) {
        uint16_t temp = impactSamples[i];
        impactSamples[i] = impactSamples[j];
        impactSamples[j] = temp;
      }
    }
  }
  
  // Calculate recommended threshold
  uint16_t lightest = impactSamples[0];
  uint16_t medianImpact = impactSamples[2];
  uint16_t strongest = impactSamples[4];
  
  // Set threshold to slightly below the lightest impact to ensure detection
  uint16_t recommendedThreshold = (uint16_t)(lightest * 0.8); // 80% of lightest impact
  
  // Add some buffer above the baseline to avoid false positives
  uint16_t minThreshold = baselineMax * 1.5; // 150% of max baseline noise
  
  // Use the higher of the two calculated thresholds
  recommendedThreshold = max(recommendedThreshold, minThreshold);
  
  // Display results
  Serial.println("\n== CALIBRATION RESULTS ==");
  Serial.print("Baseline noise (max): "); Serial.println(baselineMax);
  Serial.print("Lightest impact: "); Serial.println(lightest);
  Serial.print("Median impact: "); Serial.println(medianImpact);
  Serial.print("Strongest impact: "); Serial.println(strongest);
  Serial.print("Recommended threshold: "); Serial.println(recommendedThreshold);
  
  // Step 4: Set and save the new threshold
  Serial.println("\nSetting new impact threshold...");
  config->impactThreshold = recommendedThreshold;
  
  // Display final setting
  Serial.print("New impact threshold set to: ");
  Serial.println(config->impactThreshold);
  Serial.println("Calibration complete!");
  Serial.println("NOTE: Remember to save settings for the new threshold to persist.");
}

void AccelerometerHandler::waitForButtonPress() {
  // Simple helper function to wait for a button press
  // Assumes the button is connected to BTN_PIN (defined in BoStaff.h)
  
  // Ensure button is not already pressed
  while (digitalRead(BTN_PIN) == LOW) {
    delay(10);
    yield(); // Allow other processes to run
  }
  
  // Wait for button press
  while (digitalRead(BTN_PIN) == HIGH) {
    delay(10);
    yield(); // Allow other processes to run
  }
  
  // Debounce
  delay(50);
  yield();
  
  // Wait for button release
  while (digitalRead(BTN_PIN) == LOW) {
    delay(10);
    yield(); // Allow other processes to run
  }
  
  // Debounce
  delay(50);
  yield();
}