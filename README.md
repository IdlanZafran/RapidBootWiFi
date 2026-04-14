
# RapidBootWiFi ⚡

**By Idlan Zafran Mohd Zaidie**

[![Version](https://img.shields.io/badge/version-1.2.2-blue.svg)](https://github.com/IdlanZafran/RapidBootWiFi)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A robust, zero-boilerplate library for ESP32 and ESP8266. It handles WiFi provisioning via a Captive Portal and provides a physical **Dual-Stage Factory Reset** mechanism via rapid power-cycling.

-----

# Features

  - ⚡ **Manual Stage Control** Split logic allows you to trigger LED feedback the millisecond the device boots, *before* WiFi connection delays.
  - 📡 **Built-in WiFiManager** Automatically spins up an Access Point if no known WiFi is found.
  - 🔄 **Dual-Stage Rapid-Boot Detection** - **3 Boots:** Wipe WiFi credentials.
      - **5 Boots:** Full Factory Reset (formats LittleFS).
  - 🔘 **Magic Button Support** Trigger the portal via a physical button without losing saved credentials.
  - 💾 **Auto-Save Parameters** Handles custom HTML parameters (API Keys, MQTT IPs) and saves them to flash automatically.
  - 🔁 **Auto-Reconnect** Non-blocking background keep-alive.

-----

# Quick Start (LED Feedback Mode)

This is the recommended way to use the library to ensure users see visual confirmation of their "rapid boots."

```cpp
#include <Arduino.h>
#include <RapidBootWiFi.h>

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize & Increment (Stage 1)
  // This happens instantly. No WiFi delays yet.
  myWiFi.begin(); 

  // 2. Visual Feedback
  // Peek at the count to light up LEDs
  int count = myWiFi.getCurrentBootCount();
  if (count >= 1) digitalWrite(14, HIGH); // LED 1
  if (count >= 2) digitalWrite(12, HIGH); // LED 2
  if (count >= 3) digitalWrite(13, HIGH); // LED 3 (WiFi Reset Triggered!)

  // 3. Connect (Stage 2)
  // Now handle the 3-second 'waiting room' and WiFi connection
  myWiFi.connect(); 
  
  // Clear LEDs after successful boot
  digitalWrite(14, LOW); digitalWrite(12, LOW); digitalWrite(13, LOW);
}

void loop() {
  myWiFi.loop(); 
}
```

-----

# API Reference

### `myWiFi.begin()`

Initializes LittleFS and **increments the boot counter immediately**. Call this at the very top of `setup()` to get instant access to the boot count.

### `myWiFi.connect()`

The "Blocking" stage.

1.  Checks if the boot count hit the reset thresholds.
2.  Attempts to connect to WiFi (or starts the Captive Portal).
3.  **Waits for the Timeout Period** (default 3s). If the device stays powered on for this duration, the boot counter resets to 0.

### `myWiFi.getCurrentBootCount()`

Returns the current session's boot count (1, 2, 3, etc.). Use this to drive your LED or Buzzer UI.

### `myWiFi.wasWiFiReset()`

Returns `true` if the current boot resulted in a WiFi or Factory reset. Useful for running post-reset initialization code.

### `myWiFi.openPortal()`

Force-launches the Captive Portal. Useful for "Setup Buttons." It includes a "Danger Zone" field; typing **YES** into it will wipe all saved parameters.

### `myWiFi.loop()`

Keep-alive function. Place in `loop()` to handle auto-reconnections.

-----

# Configuration

*Call these before `myWiFi.begin()`*

| Method | Description | Default |
| :--- | :--- | :--- |
| `setAPName(str)` | Name of the Setup Hotspot | `Smart_Device_Setup` |
| `setTimeout(ms)` | The "Waiting Room" window duration | `3000` |
| `setBootThresholds(w, f)` | Thresholds for WiFi and Factory reset | `3, 5` |

-----

# Parameter Management

```cpp
// Add a parameter (saved to LittleFS automatically)
myWiFi.addParameter("api_key", "Cloud API Key", "DEFAULT_KEY", 32);

// Retrieve the value anywhere in your code
const char* key = myWiFi.getParameterValue("api_key");
```

-----

# Installation

### Arduino IDE

**Library Manager:** Search for `RapidBootWiFi` by Idlan Zafran Mohd Zaidie.

### PlatformIO

```ini
lib_deps =
    https://github.com/IdlanZafran/RapidBootWiFi.git
```