/*
 * Example: Basic Usage of RapidBootWiFi (Updated Architecture)
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * This sketch demonstrates the drop-in RapidBootWiFi library. 
 * - 3 Rapid Boots: Wipes saved WiFi credentials.
 * - 5 Rapid Boots: Factory Resets (Wipes WiFi + Custom Parameters/LittleFS).
 * - Hold BOOT Button on startup: Opens the Config Portal without dropping WiFi.
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>

// Default variables to populate the portal with
char serverURL[100] = "thingssentral.my";
char userID[16] = "000953";

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Starting SyokCircuit Node ---");

  // Setup the built-in BOOT/FLASH button on ESP boards (usually GPIO 0)
  pinMode(0, INPUT_PULLUP);

  // 1. Configure the library
  myWiFi.setAPName("Smart Plug");
  myWiFi.setTimeout(3000); // 3-second rapid-boot window
  myWiFi.setBootThresholds(3, 5); // 3 boots = WiFi Reset | 5 boots = Factory Reset

  // 2. Add custom parameters for the web portal
  // Syntax: addParameter(id, placeholder_text, default_value, length)
  myWiFi.addParameter("server", "Server URL", serverURL, 100);
  myWiFi.addParameter("device", "User ID", userID, 16);

  // 3. Start the library or open the portal
  // If you hold the BOOT button while plugging it in, force the config portal
  if (digitalRead(0) == LOW) {
      Serial.println("Setup button held! Opening Configuration Portal...");
      myWiFi.openPortal(); // This pauses execution until the user saves/exits the portal
  } else {
      Serial.println("Initializing RapidBootWiFi...");
      myWiFi.begin(); // Standard rapid-boot logic and connection
  }

  // 4. Read the values after successful connection
  Serial.println("\n--- Current Configuration ---");
  Serial.printf("Server URL: %s\n", myWiFi.getParameterValue("server"));
  Serial.printf("Device ID : %s\n", myWiFi.getParameterValue("device"));
  Serial.println("---------------------------\n");
  
  Serial.println("Setup complete! Entering main loop.");
}

void loop() {
  // 1. Keep the WiFi Alive!
  // This will quietly check the connection in the background.
  myWiFi.loop();
  
  // 2. Your normal project logic goes here
  // delay(1000) is fine for testing, but in a real project, use millis() 
  // so you don't block the myWiFi.loop() from checking the connection!
  Serial.println("Reading sensors...");
  delay(1000); 
}