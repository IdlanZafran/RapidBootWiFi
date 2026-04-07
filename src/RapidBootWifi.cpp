#include "RapidBootWiFi.h"

// --- CONSTRUCTOR ---
RapidBootWiFi::RapidBootWiFi(const char* apName, unsigned long timeoutMs, int maxBoots) {
    _apName = apName;
    _timeoutMs = timeoutMs;
    _maxBoots = maxBoots;
    _lastWifiCheck = 0;
}

// --- SETTERS & PARAMETER INJECTION ---
void RapidBootWiFi::setAPName(const char* newAPName) {
    if (newAPName != nullptr && strlen(newAPName) > 0) {
        _apName = newAPName;
    }
}

void RapidBootWiFi::setMaxBoots(int newMaxBoots) {
    if (newMaxBoots > 0) {
        _maxBoots = newMaxBoots;
    }
}

void RapidBootWiFi::setTimeout(unsigned long newTimeoutMs) {
    _timeoutMs = newTimeoutMs;
}

void RapidBootWiFi::addParameter(WiFiManagerParameter* customParam) {
    _customParams.push_back(customParam);
}

// --- CORE FUNCTIONS ---
void RapidBootWiFi::begin() {
    #ifdef ESP32
        // ESP32 Way: 'true' means format if it fails
        if (!LittleFS.begin(true)) {
            Serial.println("LittleFS Mount Failed");
        }
    #elif defined(ESP8266)
        // ESP8266 Way: Takes zero arguments. We must format manually if it fails.
        if (!LittleFS.begin()) {
            Serial.println("LittleFS Mount Failed. Formatting now...");
            LittleFS.format(); 
            if (!LittleFS.begin()) {
                Serial.println("LittleFS format and mount completely failed.");
            }
        }
    #endif

    // 1. Read and increment boot count
    int bootCount = _readBootCount();
    bootCount++;
    Serial.printf("Boot Count: %d\n", bootCount);

    // 2. Check for rapid boots
    if (bootCount >= _maxBoots) {
        Serial.println("Rapid boots detected! Resetting WiFi...");
        WiFiManager wifiManager;
        wifiManager.resetSettings(); 
        bootCount = 0;               
    }

    // 3. Save the incremented boot count
    _writeBootCount(bootCount);

    // ==========================================
    // "WAITING ROOM" LOGIC
    // ==========================================
    Serial.printf("Waiting %lu ms for rapid-boot window to close...\n", _timeoutMs);
    
    unsigned long startTime = millis();
    // Hold the ESP32 here until the timeout duration has passed
    while (millis() - startTime < _timeoutMs) {
        delay(10); // Small delay to keep the watchdog timer happy
    }

    // If the device is still powered on after the timeout, reset the count to 0
    Serial.println("Rapid-boot window closed. Resetting counter to 0.");
    _writeBootCount(0);
    // ==========================================


// ... (Your Waiting Room logic stays up here) ...

    // 4. Setup WiFiManager
    Serial.println("Starting WiFi...");
    WiFiManager wifiManager;

    // ---> ADD THIS: Load the saved parameters from LittleFS <---
    _loadCustomParams();

    // Inject parameters into the portal
    for (size_t i = 0; i < _customParams.size(); i++) {
        wifiManager.addParameter(_customParams[i]);
    }

    // Start the portal or connect
    if (!wifiManager.autoConnect(_apName)) {
        Serial.println("WiFi Failed! Restarting...");
        delay(3000);
        ESP.restart();
    }
    
    Serial.println("WiFi Connected!");
    
    // ---> ADD THIS: Save the parameters if the user changed them <---
    _saveCustomParams();

}

// --- KEEPALIVE LOOP ---
void RapidBootWiFi::loop() {
    // Translation: "Is the WiFi disconnected right now?"
    if (WiFi.status() != WL_CONNECTED) {
        
        // Translation: "Has it been at least 10 seconds since we last tried to reconnect?"
        // We do this so we don't freeze the ESP by trying to connect a thousand times a second.
        if (millis() - _lastWifiCheck >= 10000) {
            Serial.println("⚠️ WiFi connection lost! Attempting to reconnect...");
            
            // WiFi.reconnect() is a built-in ESP command. 
            // It tells the chip: "Use the credentials you already have saved in flash and try again!"
            WiFi.reconnect(); 
            
            _lastWifiCheck = millis(); // Reset the 10-second timer
        }
    }
}

// --- PRIVATE HELPER FUNCTIONS ---
int RapidBootWiFi::_readBootCount() {
    int count = 0;
    File readFile = LittleFS.open("/boot_count.txt", "r");
    if (readFile) {
        count = readFile.readString().toInt();
        readFile.close();
    }
    return count;
}

void RapidBootWiFi::_writeBootCount(int count) {
    File writeFile = LittleFS.open("/boot_count.txt", "w");
    if (writeFile) {
        writeFile.print(String(count));
        writeFile.close();
    }
}

// ==========================================
// AUTO-SAVE & AUTO-LOAD PARAMETERS LOGIC
// ==========================================

void RapidBootWiFi::_loadCustomParams() {
    // Translation: "Go through every parameter the user gave us one by one."
    for (size_t i = 0; i < _customParams.size(); i++) {
        
        // Translation: "Create a unique filename based on the parameter ID."
        // Example: If ID is "server", filename becomes "/param_server.txt"
        String filename = String("/param_") + _customParams[i]->getID() + ".txt";
        
        File readFile = LittleFS.open(filename, "r");
        
        // Translation: "If the file exists, read it and overwrite the default value."
        if (readFile) {
            String savedValue = readFile.readString();
            readFile.close();
            
            // This copies the saved LittleFS text directly into the parameter's memory box!
            strncpy((char*)_customParams[i]->getValue(), savedValue.c_str(), _customParams[i]->getValueLength());
            
            Serial.printf("Loaded parameter [%s]: %s\n", _customParams[i]->getID(), savedValue.c_str());
        }
    }
}

void RapidBootWiFi::_saveCustomParams() {
    // Translation: "Go through every parameter again to see if we need to save."
    for (size_t i = 0; i < _customParams.size(); i++) {
        
        String filename = String("/param_") + _customParams[i]->getID() + ".txt";
        String currentValueInFile = "";
        
        // 1. Read what is currently inside the file
        File readFile = LittleFS.open(filename, "r");
        if (readFile) {
            currentValueInFile = readFile.readString();
            readFile.close();
        }
        
        // 2. Get the value currently held in the WiFiManager parameter
        String newValueFromPortal = String(_customParams[i]->getValue());
        
        // Translation: "Did the user change the value? If they are different, write the new one to memory!"
        if (currentValueInFile != newValueFromPortal) {
            File writeFile = LittleFS.open(filename, "w");
            if (writeFile) {
                writeFile.print(newValueFromPortal);
                writeFile.close();
                Serial.printf("Saved new parameter [%s]: %s\n", _customParams[i]->getID(), newValueFromPortal.c_str());
            }
        }
    }
}