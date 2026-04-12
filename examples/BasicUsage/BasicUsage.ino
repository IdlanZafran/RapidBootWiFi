/*
 * Example: Basic Usage of RapidBootWiFi (Updated Architecture)
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * This sketch demonstrates the drop-in RapidBootWiFi library. 
 * When the device boots, it waits. If it is turned off and on 
 * 3 times within those initial windows, it wipes the saved WiFi credentials.
 * It also demonstrates how to inject custom parameters into the WiFiManager portal.
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>

// Default variables to populate the portal with
char serverURL[] = "thingssentral.my";
char userID[] = "000953";

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Starting SyokCircuit Node ---");

  // 1. Configure the library (Overrides the default settings)
  myWiFi.setAPName("Smart Plug");
  myWiFi.setTimeout(3000);
  myWiFi.setMaxBoots(3);

  // 2. Add custom parameters for the web portal
  // Syntax: addParameter(id, placeholder_text, default_value, length)
  myWiFi.addParameter("server", "Server URL", serverURL, 100);
  myWiFi.addParameter("device", "User ID", userID, 10);

  // 3. Start the library
  // NOTE: This function will pause the code for exactly 3 seconds 
  // to evaluate the rapid-boot window before attempting to connect to WiFi.
  Serial.println("Initializing RapidBootWiFi...");
  myWiFi.begin();

  // 4. Read the values after successful connection
  // Using the new helper function to fetch the values by their ID
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