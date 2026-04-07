/*
 * Example: Basic Usage of RapidBootWiFi (Updated Architecture)
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * This sketch demonstrates the drop-in RapidBootWiFi library. 
 * When the device boots, it waits for 3 seconds. If it is turned off 
 * and on 3 times within those initial 3-second windows, it wipes 
 * the saved WiFi credentials.
 * * It also demonstrates how to inject custom parameters (like a Server URL) 
 * into the WiFiManager captive portal.
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>

// 1. Initialize the library
// Format: (AP_Name, Timeout_Milliseconds, Required_Boots_to_Reset)
RapidBootWiFi myWiFi("Smart Plug", 3000, 3);

// 2. Setup your custom parameters for the web portal
char serverURL[100] = "api.syokcircuit.com";
char deviceID[16] = "SyokCircuit_001";

WiFiManagerParameter custom_server("server", "Server URL", serverURL, 100);
WiFiManagerParameter custom_device("device", "Device ID", deviceID, 10);

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Starting SyokCircuit Node ---");

  // 3. Inject the custom parameters using the memory address pointer (&)
  myWiFi.addParameter(&custom_server);
  myWiFi.addParameter(&custom_device);

  // 4. Start the library
  // NOTE: This function will pause the code for exactly 3 seconds 
  // to evaluate the rapid-boot window before attempting to connect to WiFi.
  Serial.println("Initializing RapidBootWiFi...");
  myWiFi.begin();

  // 5. Read the values after successful connection
  // If the user changed these in the captive portal, the new values will print here
  Serial.println("\n--- Current Configuration ---");
  Serial.printf("Server URL: %s\n", custom_server.getValue());
  Serial.printf("Device ID : %s\n", custom_device.getValue());
  Serial.println("---------------------------\n");
  
  Serial.println("Setup complete! Entering main loop.");
}

void loop() {
// 1. Keep the WiFi Alive!
  // This will quietly check the connection in the background.
  // If it drops, it will automatically try to reconnect every 10 seconds.
  myWiFi.loop();
  
  // 2. Your normal project logic goes here
  // Because myWiFi.loop() is "non-blocking", this code will continue 
  // to run perfectly even while the ESP is trying to reconnect to the router!
  
  Serial.println("Reading sensors...");
  delay(1000);
}