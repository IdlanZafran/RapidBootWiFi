/**
 * Example: RapidBootWiFi with LED Feedback
 * Author: Idlan Zafran Mohd Zaidie
 * Date: 2026
 * * Flow: 
 * 1. myWiFi.begin() -> Increments count in LittleFS immediately.
 * 2. LED Logic -> Uses the incremented count to show visual feedback.
 * 3. myWiFi.connect() -> Connects to WiFi and starts the 3-second reset window.
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>

// Pin definitions for LED feedback
const int LED_BOOT_1 = 14; 
const int LED_BOOT_2 = 12; 
const int LED_BOOT_3 = 13;
const int BUTTON_PIN = 0; // BOOT/Flash button on most ESPs

void updateBootLeds(int count) {
  digitalWrite(LED_BOOT_1, (count >= 1) ? HIGH : LOW);
  digitalWrite(LED_BOOT_2, (count >= 2) ? HIGH : LOW);
  digitalWrite(LED_BOOT_3, (count >= 3) ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  delay(100); 
  Serial.println("\n--- Starting RapidBootWiFi Device ---");

  // 1. Hardware Setup
  pinMode(LED_BOOT_1, OUTPUT);
  pinMode(LED_BOOT_2, OUTPUT);
  pinMode(LED_BOOT_3, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  // 2. STAGE 1: Increment and Detect Reset
  // This increments the file and checks thresholds internally.
  myWiFi.begin(); 

  // 3. STAGE 2: Display Feedback
  // Now we "peek" at the count that was just incremented.
  int currentCount = myWiFi.getCurrentBootCount();
  Serial.printf("Boot Session: %d\n", currentCount);
  updateBootLeds(currentCount);

  // 4. Configuration
  myWiFi.setAPName("Smart_Device_Setup");
  myWiFi.setTimeout(3000); // 3 seconds to "stay on" before count resets to 0
  myWiFi.setBootThresholds(3, 5);
  myWiFi.addParameter("uID", "User ID", "00953", 15);

  // 5. STAGE 3: Run Connection
  // Check if button is held for manual portal, otherwise start connection
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println(">>> Manual Setup Triggered! Opening Portal...");
    myWiFi.openPortal(); 
  } else {
    // This connects to WiFi and performs the 3-second waiting room pause.
    myWiFi.connect(); 
  }

  // 6. POST-BOOT CHECK
  // Check if a reset occurred during begin()
  if (myWiFi.wasWiFiReset()) {
    Serial.println(">>> ALERT: WiFi credentials were reset this boot!");
    // Example: Flash LEDs 5 times to confirm reset success
    for(int i=0; i<5; i++) {
        updateBootLeds(3); delay(100);
        updateBootLeds(0); delay(100);
    }
  }

  // 7. Cleanup UI
  // Turn off LEDs to save power or transition to main app state
  updateBootLeds(0);
  Serial.println("System Connected and Ready.");
}

void loop() {
  // Handles background WiFi maintenance
  myWiFi.loop(); 
}