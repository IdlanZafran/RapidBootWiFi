#include "RapidBootWiFi.h"

// Instantiate the global object so the user doesn't have to
RapidBootWiFi myWiFi;

// --- CONSTRUCTOR ---
RapidBootWiFi::RapidBootWiFi(const char* apName, unsigned long timeoutMs, int wifiBoots, int factoryBoots) {
    _apName = apName;
    _timeoutMs = timeoutMs;
    _wifiBoots = wifiBoots;
    _factoryBoots = factoryBoots;
    _lastWifiCheck = 0;
}

// --- DESTRUCTOR ---
RapidBootWiFi::~RapidBootWiFi() {
    // Clean up only the memory that the library allocated itself
    for (size_t i = 0; i < _customParams.size(); i++) {
        if (_libraryOwnsParam[i]) {
            
            // Temporarily disable the virtual destructor warning for this third-party class
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
            delete _customParams[i]; 
            #pragma GCC diagnostic pop
            
        }
    }
    _customParams.clear();
    _libraryOwnsParam.clear();
}

// --- SETTERS ---
void RapidBootWiFi::setBootThresholds(int wifiBoots, int factoryBoots) {
    if (wifiBoots > 0 && factoryBoots > wifiBoots) {
        _wifiBoots = wifiBoots;
        _factoryBoots = factoryBoots;
    }
}

void RapidBootWiFi::setAPName(const char* newAPName) {
    if (newAPName != nullptr && strlen(newAPName) > 0) {
        _apName = newAPName;
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
        if (!LittleFS.begin(true)) {
            Serial.println("LittleFS Mount Failed");
        }
    #elif defined(ESP8266)
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

    // 2. Check for thresholds
    if (bootCount >= _factoryBoots) {
        Serial.println(">>> 5 RAPID BOOTS: FACTORY RESETTING <<<");
        WiFiManager wifiManager;
        wifiManager.resetSettings(); // Clear WiFi
        
        LittleFS.format(); // Wipe saved parameters and ThingsSentral history
        Serial.println("LittleFS wiped clean.");
        
        bootCount = 0; // Reset counter because the ultimate action is done
    } 
    else if (bootCount == _wifiBoots) {
        Serial.println(">>> 3 RAPID BOOTS: Resetting WiFi ONLY <<<");
        WiFiManager wifiManager;
        wifiManager.resetSettings(); // Clear WiFi
        
        // IMPORTANT: We do NOT reset bootCount to 0 here. 
        // We let it save as '3'. If the user unplugs it again within 
        // the timeout window, it will climb to 4, and then 5.
    }

    // 3. Save the current boot count
    _writeBootCount(bootCount);

    // ==========================================
    // "WAITING ROOM" LOGIC
    // ==========================================
    Serial.printf("Waiting %lu ms for rapid-boot window to close...\n", _timeoutMs);
    
    unsigned long startTime = millis();
    while (millis() - startTime < _timeoutMs) {
        delay(10); // Keep watchdog happy
    }

    // If the ESP survived the waiting room without losing power, 
    // the user has stopped turning it on and off. Reset back to zero.
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
            
            // FIX: Ensure we don't overflow and ALWAYS null-terminate
            int maxLen = _customParams[i]->getValueLength();
            char* dest = (char*)_customParams[i]->getValue();
            memset(dest, 0, maxLen); // Clear the buffer first
            strncpy(dest, savedValue.c_str(), maxLen - 1);
            dest[maxLen - 1] = '\0'; // Force termination
            
            Serial.printf("Loaded parameter [%s]: %s\n", _customParams[i]->getID(), dest);
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

// --- ON-DEMAND CONFIG PORTAL ---
void RapidBootWiFi::openPortal() {
    Serial.println(">>> FORCING CONFIG PORTAL (Keeping WiFi Intact) <<<");
    WiFiManager wifiManager;

    // 1. Load the current parameters so they show up in the text boxes
    _loadCustomParams();
    for (size_t i = 0; i < _customParams.size(); i++) {
        wifiManager.addParameter(_customParams[i]);
    }

    // 2. Add a visual separator and the Factory Reset Input
    // Note: These are local variables, so they safely clean themselves up from memory!
    WiFiManagerParameter dangerDivider("<br><hr><h3 style='color:red;'>Danger Zone</h3>");
    WiFiManagerParameter wipeInput("wipe", "Type <b>YES</b> to Factory Reset IDs", "", 5);
    
    wifiManager.addParameter(&dangerDivider);
    wifiManager.addParameter(&wipeInput);

    // 3. Start the portal and wait for user to hit Save or Exit
    if (!wifiManager.startConfigPortal(_apName)) {
        Serial.println("Portal exited without saving.");
    }

    // 4. Check if the user requested a Factory Reset
    String wipeCommand = String(wipeInput.getValue());
    wipeCommand.toUpperCase(); // Make it case-insensitive

    if (wipeCommand == "YES") {
        Serial.println(">>> WIPE COMMAND RECEIVED! Deleting saved parameters...");
        
        // Loop through and dynamically delete only the parameter files
        for (size_t i = 0; i < _customParams.size(); i++) {
            String filename = String("/param_") + _customParams[i]->getID() + ".txt";
            LittleFS.remove(filename);
            Serial.printf("Deleted: %s\n", filename.c_str());
        }
        
        Serial.println("Parameters wiped successfully. Restarting to apply defaults...");
        delay(1000);
        ESP.restart();
    }

    // 5. If "YES" wasn't typed, save the parameters normally
    Serial.println("Saving updated parameters...");
    _saveCustomParams();
    
    Serial.println("Restarting to apply changes...");
    delay(1000);
    ESP.restart();
}

int RapidBootWiFi::getCurrentBootCount() {
    return _readBootCount();
}

bool RapidBootWiFi::wasWiFiReset() {
    // Check if boot count was exactly at the wifi threshold
    // This indicates a WiFi reset just occurred
    int currentCount = _readBootCount();
    return (currentCount == _wifiBoots);
}