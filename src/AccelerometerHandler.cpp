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
  
  // Use the configured threshold instead of hard-coded value
  uint16_t currentThreshold = config->impactThreshold;
  
  // CHANGED: Comment out or reduce frequency of debug printing
  // Only print every 5 seconds instead of every second to reduce potential I2C conflicts
  /*
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {
    Serial.print("Accel: "); Serial.print(accelRaw);
    Serial.print(" (X:"); Serial.print(a.acceleration.x);
    Serial.print(" Y:"); Serial.print(a.acceleration.y);
    Serial.print(" Z:"); Serial.print(a.acceleration.z);
    Serial.print(") Threshold: "); Serial.println(currentThreshold);
    lastPrint = millis();
  }
  */
  
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
}

bool AccelerometerHandler::impactDetected() {
  // Return and clear the impact flag without Serial printing
  bool result = impactDetectedFlag;
  // Removed the Serial.println that was here causing the issue
  impactDetectedFlag = false;
  return result;
}

void AccelerometerHandler::calibrate() {
  if (!mpuInitialized) {
    Serial.println("ERROR: Cannot calibrate - Accelerometer not initialized");
    return;
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
  delay(1000);
  
  // Collect baseline readings for 3 seconds
  unsigned long baselineStart = millis();
  while (millis() - baselineStart < 3000) {
    sensors_event_t a, g, temp;
    if (mpu.getEvent(&a, &g, &temp)) {
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
    delay(10); // Sample at ~100Hz
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
      if (mpu.getEvent(&a, &g, &temp)) {
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
      delay(5); // Sample at ~200Hz for better peak detection
    }
    
    impactSamples[i] = maxImpact;
    Serial.print("Impact #"); Serial.print(i + 1);
    Serial.print(" maximum reading: "); Serial.println(impactSamples[i]);
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
  }
  
  // Wait for button press
  while (digitalRead(BTN_PIN) == HIGH) {
    delay(10);
  }
  
  // Debounce
  delay(50);
  
  // Wait for button release
  while (digitalRead(BTN_PIN) == LOW) {
    delay(10);
  }
  
  // Debounce
  delay(50);
}