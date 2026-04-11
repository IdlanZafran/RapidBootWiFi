# RapidBootWiFi ⚡

By Idlan Zafran Mohd Zaidie

A robust, drop-in library for ESP32 and ESP8266 devices that automatically handles WiFi provisioning via a Captive Portal, and provides a physical "Factory Reset" mechanism by power-cycling the device 3 times rapidly.

## Features
* **Built-in WiFiManager:** Automatically spins up an Access Point if no known WiFi is found.
* **Rapid-Boot Detection:** Uses `LittleFS` to count boots. If the device is turned on/off quickly 3 times, it wipes the saved WiFi credentials.
* **Auto-Save Parameters:** Easily inject custom HTML parameters (like Server URLs or API Keys) into the Captive Portal. The library automatically saves and loads these from flash memory.
* **Auto-Reconnect:** Non-blocking background keep-alive ensures your device silently reconnects if the router drops.
* **Dynamic Settings:** Change the AP name, timeout duration, or required boot count on the fly.

## Installation

### For Arduino IDE
1. Open the Arduino IDE.
2. Go to **Sketch** -> **Include Library** -> **Manage Libraries...**
3. In the search bar, type **RapidBootWiFi** by Idlan Zafran Mohd Zaidie.
4. Click **Install**.

### For PlatformIO
Add the following to your `platformio.ini` file under your environment configuration:
```ini
lib_deps =
    [https://github.com/IdlanZafran/RapidBootWiFi.git](https://github.com/IdlanZafran/RapidBootWiFi.git)
```

## Quick Start

```cpp
#include <Arduino.h>
#include <RapidBootWiFi.h>

// Initialize with (AP_NAME, TIMEOUT_MS, REQUIRED_BOOTS)
RapidBootWiFi myWiFi("Smart Plug", 3000, 3);

void setup() {
  Serial.begin(115200);
  
  // Handles LittleFS, Boot Counting, the 3-second Waiting Room, and WiFi connection
  myWiFi.begin(); 
}

void loop() {
  // Quietly checks WiFi status in the background. 
  // If the router drops, it automatically attempts to reconnect every 10 seconds.
  myWiFi.loop(); 
  
  // Your main project code goes here!
}