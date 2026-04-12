# RapidBootWiFi ⚡

**By Idlan Zafran Mohd Zaidie**

[![Version](https://img.shields.io/badge/version-1.0.4-blue.svg)](https://github.com/IdlanZafran/ThingsSentral)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


A robust, zero-boilerplate, drop-in library for ESP32 and ESP8266 devices. It automatically handles WiFi provisioning via a Captive Portal and provides a physical **Factory Reset** mechanism by power-cycling the device **3 times rapidly**.

---

# Features

- ⚡ **Zero Boilerplate**  
  Works just like the native `Serial` or `WiFi` libraries. The `myWiFi` object is pre-created for you in the background.

- 📡 **Built-in WiFiManager**  
  Automatically spins up an Access Point if no known WiFi is found.

- 🔄 **Rapid-Boot Detection**  
  Uses `LittleFS` to count boots. If the device is turned on/off quickly 3 times within the timeout window, it wipes the saved WiFi credentials.

- 💾 **Auto-Save Custom Parameters**  
  Easily inject custom HTML parameters (like MQTT Server URLs, User IDs, or API Keys) into the Captive Portal. The library automatically saves, loads, and manages these in flash memory.

- 🔁 **Auto-Reconnect**  
  Non-blocking background keep-alive ensures your device silently reconnects if the router drops.

- ⚙️ **Dynamic Settings**  
  Change the AP name, timeout duration, or required boot count on the fly.

---

# Installation

## Arduino IDE

1. Open Arduino IDE  
2. Go to **Sketch → Include Library → Manage Libraries…**  
3. Search **RapidBootWiFi by Idlan Zafran Mohd Zaidie**  
4. Click **Install**

---

## PlatformIO

Add this to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/IdlanZafran/RapidBootWiFi.git
```

---

# Quick Start (Basic Usage)

Because `RapidBootWiFi` acts like a native Arduino library, you don't even need to create the object.

```cpp
#include <Arduino.h>
#include <RapidBootWiFi.h>

void setup() {
  Serial.begin(115200);
  
  // Optional: Override the default settings
  myWiFi.setAPName("Smart_Plug_Setup");
  myWiFi.setTimeout(3000);   // Wait 3 seconds to check for rapid boots
  myWiFi.setMaxBoots(3);     // Reset WiFi after 3 rapid boots
  
  // Handles LittleFS, Boot Counting, and WiFi connection
  myWiFi.begin(); 
  
  Serial.println("We are connected!");
}

void loop() {
  // Quietly checks WiFi status in background
  myWiFi.loop(); 
  
  // Your main project code goes here
}
```

---

# Advanced Usage (Custom Web Portal Parameters)

Need user input like API keys, MQTT server IP, or User IDs?  
`RapidBootWiFi` handles everything automatically.

```cpp
#include <Arduino.h>
#include <RapidBootWiFi.h>

void setup() {
  Serial.begin(115200);

  // Add parameters to captive portal
  myWiFi.addParameter("mqtt_ip", "MQTT Broker IP", "192.168.1.100", 40);
  myWiFi.addParameter("api_key", "Secret API Key", "", 50);

  // Start WiFi sequence
  myWiFi.begin(); 

  // Retrieve saved values
  Serial.println("\n--- Saved Configuration ---");
  Serial.printf("MQTT IP: %s\n", myWiFi.getParameterValue("mqtt_ip"));
  Serial.printf("API Key: %s\n", myWiFi.getParameterValue("api_key"));
}

void loop() {
  myWiFi.loop(); 
}
```

---

# API Reference

---

## Setup Methods

### `myWiFi.begin()`

- Initializes filesystem  
- Checks boot count  
- Evaluates rapid-boot timeout  
- Starts WiFi connection  

---

### `myWiFi.loop()`

- Non-blocking keep-alive function  
- Place inside Arduino `loop()`  
- Automatically reconnects WiFi  

---

# Configuration Methods  
*(Call before `begin()`)*

### `myWiFi.setAPName(const char* name)`

Sets Captive Portal AP name  

**Default:**  
```
Smart_Device_Setup
```

---

### `myWiFi.setTimeout(unsigned long ms)`

Sets rapid-boot detection window  

**Default:**  
```
3000 ms
```

---

### `myWiFi.setMaxBoots(int count)`

Sets number of rapid boots required for factory reset  

**Default:**  
```
3
```

---

# Parameter Management

### `myWiFi.addParameter()`

```cpp
myWiFi.addParameter(
  const char* id,
  const char* placeholder,
  const char* defaultValue,
  int length
);
```

Adds text input to Captive Portal

---

### `myWiFi.getParameterValue()`

```cpp
const char* value = myWiFi.getParameterValue("id");
```

Returns saved value

Returns empty string if ID not found

---

# Example

```cpp
myWiFi.addParameter("mqtt", "MQTT Server", "192.168.0.100", 40);

const char* mqtt = myWiFi.getParameterValue("mqtt");
```

---
