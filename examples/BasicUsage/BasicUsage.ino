/**
 * Example: Basic Usage of RapidBootWiFi (Updated Architecture)
 * Author: Idlan Zafran Mohd Zaidie
 * Description:
 * Demonstrates manual increment for LED feedback + wasWiFiReset() check.
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>

// Pin definitions for LED feedback
const int LED_BOOT_1 = 14; 
const int LED_BOOT_2 = 12; 
const int LED_BOOT_3 = 13;

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
  pinMode(0, INPUT_PULLUP); // BOOT button check

  // 2. IMMEDIATE ACTION: Increment and show LEDs
  // This happens before the 3-second 'waiting room'
  int currentCount = myWiFi.incrementAndGetBootCount();
  Serial.printf("Current Session Boot Count: %d\n", currentCount);
  updateBootLeds(currentCount);

  // 3. Configure Library
  myWiFi.setAPName("Smart_Device_Setup");
  myWiFi.setTimeout(3000); 
  myWiFi.setBootThresholds(3, 5);

  // 4. Add Custom Parameters
  myWiFi.addParameter("server", "Server URL", "thingssentral.my", 100);

  // 5. Start Process
  if (digitalRead(0) == LOW) {
    Serial.println(">>> Setup button held! Opening Portal...");
    myWiFi.openPortal(); 
  } else {
    // begin() uses the count we just incremented to decide on resets
    myWiFi.begin(); 
  }

  // 6. CHECK: Was WiFi just reset by the 3-boot threshold?
  // We check this AFTER begin() because that's where the reset happens.
  if (myWiFi.wasWiFiReset()) {
    Serial.println(">>> ALERT: WiFi was reset due to 3 rapid boots!");
    // You could play a specific tone here or flash LEDs to confirm reset
  }

  // 7. Cleanup UI
  digitalWrite(LED_BOOT_1, LOW);
  digitalWrite(LED_BOOT_2, LOW);
  digitalWrite(LED_BOOT_3, LOW);

  Serial.println("System Ready.");
}

void loop() {
  myWiFi.loop(); 
}