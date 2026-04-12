#include "RapidBootWiFi.h"

// Instantiate the global object so the user doesn't have to
RapidBootWiFi myWiFi;

// --- CONSTRUCTOR ---
RapidBootWiFi::RapidBootWiFi(const char* apName, unsigned long timeoutMs, int maxBoots) {
    _apName = apName;
    _timeoutMs = timeoutMs;
    _maxBoots = maxBoots;
    _lastWifiCheck = 0;
}

// --- DESTRUCTOR ---
RapidBootWiFi::~RapidBootWiFi() {
    // Clean up only the memory that the library allocated itself
    for (size_t i = 0; i < _customParams.size(); i++) {
        if (_libraryOwnsParam[i]) {
            delete _customParams[i]; 
        }
    }
    _customParams.clear();
    _libraryOwnsParam.clear();
}

// --- SETTERS ---
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

// --- PARAMETER INJECTION ---

// Original method: User passes a pointer
void RapidBootWiFi::addParameter(WiFiManagerParameter* customParam) {
    _customParams.push_back(customParam);
    _libraryOwnsParam.push_back(false); // User owns this, we won't delete it
}

// New method: Library handles the WiFiManager stuff behind the scenes
void RapidBootWiFi::addParameter(const char* id, const char* placeholder, const char* defaultValue, int length) {
    WiFiManagerParameter* newParam = new WiFiManagerParameter(id, placeholder, defaultValue, length);
    _customParams.push_back(newParam);
    _libraryOwnsParam.push_back(true); // Library created this, we must delete it later
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
    while (millis() - startTime < _timeoutMs) {
        delay(10); // Keep watchdog happy
    }

    Serial.println("Rapid-boot window closed. Resetting counter to 0.");
    _writeBootCount(0);
    // ==========================================

    // 4. Setup WiFiManager
    Serial.println("Starting WiFi...");
    WiFiManager wifiManager;

    // Load the saved parameters from LittleFS
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
    
    // Save the parameters if the user changed them
    _saveCustomParams();
}

// --- KEEPALIVE LOOP ---
void RapidBootWiFi::loop() {
    if (WiFi.status() != WL_CONNECTED) {
        if (millis() - _lastWifiCheck >= 10000) {
            Serial.println("⚠️ WiFi connection lost! Attempting to reconnect...");
            WiFi.reconnect(); 
            _lastWifiCheck = millis();
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

void RapidBootWiFi::_loadCustomParams() {
    for (size_t i = 0; i < _customParams.size(); i++) {
        String filename = String("/param_") + _customParams[i]->getID() + ".txt";
        File readFile = LittleFS.open(filename, "r");
        
        if (readFile) {
            String savedValue = readFile.readString();
            readFile.close();
            strncpy((char*)_customParams[i]->getValue(), savedValue.c_str(), _customParams[i]->getValueLength());
            Serial.printf("Loaded parameter [%s]: %s\n", _customParams[i]->getID(), savedValue.c_str());
        }
    }
}

void RapidBootWiFi::_saveCustomParams() {
    for (size_t i = 0; i < _customParams.size(); i++) {
        String filename = String("/param_") + _customParams[i]->getID() + ".txt";
        String currentValueInFile = "";
        
        File readFile = LittleFS.open(filename, "r");
        if (readFile) {
            currentValueInFile = readFile.readString();
            readFile.close();
        }
        
        String newValueFromPortal = String(_customParams[i]->getValue());
        
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

// --- GET PARAMETER VALUE ---
const char* RapidBootWiFi::getParameterValue(const char* id) {
    for (size_t i = 0; i < _customParams.size(); i++) {
        // If the ID matches, return the value!
        if (strcmp(_customParams[i]->getID(), id) == 0) {
            return _customParams[i]->getValue();
        }
    }
    return ""; // Return empty string if parameter not found
}