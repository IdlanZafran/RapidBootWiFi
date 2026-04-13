
# RapidBootWiFi ⚡

**By Idlan Zafran Mohd Zaidie**

[![Version](https://img.shields.io/badge/version-1.1.1-blue.svg)](https://github.com/IdlanZafran/RapidBootWiFi)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A robust, zero-boilerplate, drop-in library for ESP32 and ESP8266 devices. It automatically handles WiFi provisioning via a Captive Portal and provides a physical **Dual-Stage Factory Reset** mechanism by power-cycling the device rapidly. 

---

# Features

- ⚡ **Zero Boilerplate** Works just like the native `Serial` or `WiFi` libraries. The `myWiFi` object is pre-created for you in the background.

- 📡 **Built-in WiFiManager** Automatically spins up an Access Point if no known WiFi is found.

- 🔄 **Dual-Stage Rapid-Boot Detection** Uses `LittleFS` to count boots. By default: Power-cycle **3 times** to wipe WiFi credentials, or **5 times** for a complete **Factory Reset** (formats the entire filesystem).

- 🔘 **On-Demand Config Portal** Easily bind a hardware button (like the ESP BOOT button) to trigger the config portal on startup *without* losing your saved WiFi password.

- 💾 **Auto-Save Custom Parameters & Danger Zone** Easily inject custom HTML parameters (MQTT Server URLs, User IDs, API Keys) into the portal. The library saves them safely to flash memory. Includes a built-in "Danger Zone" field in the portal to securely wipe parameters back to code defaults.

- 🔁 **Auto-Reconnect** Non-blocking background keep-alive ensures your device silently reconnects if the router drops.

---

# Installation

## Arduino IDE

1. Open Arduino IDE  
2. Go to **Sketch → Include Library → Manage Libraries…** 3. Search **RapidBootWiFi by Idlan Zafran Mohd Zaidie** 4. Click **Install**

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
  myWiFi.setTimeout(3000);           // Wait 3 seconds to check for rapid boots
  myWiFi.setBootThresholds(3, 5);    // 3 boots = WiFi Reset | 5 boots = Full Factory Reset
  
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

# Advanced Usage (Parameters & On-Demand Portal)

Need user input like API keys or User IDs? Want to let users change settings later without dropping their WiFi? Use the hardware button integration!

```cpp
#include <Arduino.h>
#include <RapidBootWiFi.h>

void setup() {
  Serial.begin(115200);
  
  // Pin 0 is the built-in BOOT/FLASH button on most ESP boards
  pinMode(0, INPUT_PULLUP);

  // Add parameters to captive portal
  myWiFi.addParameter("mqtt_ip", "MQTT Broker IP", "192.168.1.100", 40);
  myWiFi.addParameter("user_id", "User ID", "00953", 16);

  // Magic Button Check: Hold BOOT button while powering on to enter Setup Mode
  if (digitalRead(0) == LOW) {
      Serial.println("Setup button held! Opening Portal...");
      myWiFi.openPortal(); // Pauses code until user saves/exits web page
  } else {
      myWiFi.begin(); // Standard rapid-boot logic and connection
  }

  // Retrieve saved values
  Serial.println("\n--- Saved Configuration ---");
  Serial.printf("MQTT IP: %s\n", myWiFi.getParameterValue("mqtt_ip"));
  Serial.printf("User ID: %s\n", myWiFi.getParameterValue("user_id"));
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

### `myWiFi.openPortal()`
- Immediately launches the Config Portal AP  
- Keeps current WiFi credentials safe  
- Includes a built-in parameter wipe ("Danger Zone")  
- Restarts the ESP automatically upon saving  

### `myWiFi.loop()`
- Non-blocking keep-alive function  
- Place inside Arduino `loop()`  
- Automatically reconnects WiFi  

---

## Configuration Methods  
*(Call before `begin()`)*

### `myWiFi.setAPName(const char* name)`
Sets Captive Portal AP name  
**Default:** `Smart_Device_Setup`

### `myWiFi.setTimeout(unsigned long ms)`
Sets rapid-boot detection window  
**Default:** `3000` ms

### `myWiFi.setBootThresholds(int wifiBoots, int factoryBoots)`
Sets the sequence required to reset the device.  
**Default:** `3` (WiFi Wipe), `5` (Complete LittleFS Format)

---

## Parameter Management

### `myWiFi.addParameter()`

```cpp
myWiFi.addParameter(
  const char* id,
  const char* placeholder,
  const char* defaultValue,
  int length
);
```
Adds text input to Captive Portal. (Library automatically manages the memory pointers behind the scenes).

### `myWiFi.getParameterValue()`

```cpp
const char* value = myWiFi.getParameterValue("id");
```
Returns saved value. Returns empty string if ID not found.

---

**Example:**

```cpp
myWiFi.addParameter("mqtt", "MQTT Server", "192.168.0.100", 40);
const char* mqtt = myWiFi.getParameterValue("mqtt");
```