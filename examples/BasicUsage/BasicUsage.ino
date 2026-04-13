/*
 * Example: Basic Usage of RapidBootWiFi (Updated Architecture)
 * Author: Idlan Zafran Mohd Zaidie
 * Description:
 * This sketch demonstrates the drop-in RapidBootWiFi library. 
 * - 3 Rapid Boots: Wipes saved WiFi credentials.
 * - 5 Rapid Boots: Factory Resets (Wipes WiFi + Custom Parameters/LittleFS).
 * - Hold BOOT Button on startup: Opens the Config Portal without dropping WiFi.
 * - Demonstrates reading boot count for conditional actions
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting device...");

  // Configure library
  myWiFi.setAPName("Smart Device");
  myWiFi.setTimeout(3000);
  myWiFi.setBootThresholds(3, 5);

  // Add custom parameters
  myWiFi.addParameter("server", "Server URL", "thingssentral.my", 100);
  myWiFi.addParameter("device", "Device ID", "000953", 16);

  // Start WiFi (handles rapid-boot logic)
  myWiFi.begin();

  // CHECK: Was WiFi just reset by rapid-boot?
  if (myWiFi.wasWiFiReset()) {
    Serial.println(">>> WiFi was reset! Running post-reset code...");
    
    // Your code here (send notification, reset variables, etc.)
    
  } else {
    Serial.println("Normal boot - no reset occurred.");
  }

  // Get parameter values
  Serial.printf("Server: %s\n", myWiFi.getParameterValue("server"));
  Serial.printf("Device: %s\n", myWiFi.getParameterValue("device"));
}

void loop() {
  myWiFi.loop();  // Keep WiFi alive
  
  // Your main code here
  delay(1000);
}
