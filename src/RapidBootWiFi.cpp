#include "RapidBootWiFi.h"

RapidBootWiFi myWiFi;

RapidBootWiFi::RapidBootWiFi(const char* apName, unsigned long timeoutMs, int wifiBoots, int factoryBoots) {
    _apName = apName;
    _timeoutMs = timeoutMs;
    _wifiBoots = wifiBoots;
    _factoryBoots = factoryBoots;
    _lastWifiCheck = 0;
    _activeBootCount = 0;
    _resetOccurred = false;
}

RapidBootWiFi::~RapidBootWiFi() {
    for (size_t i = 0; i < _customParams.size(); i++) {
        if (_libraryOwnsParam[i]) {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
            delete _customParams[i]; 
            #pragma GCC diagnostic pop
        }
    }
}

void RapidBootWiFi::begin() {
    #ifdef ESP32
        if (!LittleFS.begin(true)) Serial.println("LittleFS Fail");
    #else
        if (!LittleFS.begin()) { LittleFS.format(); LittleFS.begin(); }
    #endif

    int count = _readBootCount();
    count++;
    _writeBootCount(count);
    _activeBootCount = count;
    _resetOccurred = false; // Reset flag for new boot session

    // Check thresholds immediately before UI display
    if (_activeBootCount >= _factoryBoots) {
        WiFiManager wm;
        wm.resetSettings();
        LittleFS.format();
        _activeBootCount = 0;
        _writeBootCount(0);
        _resetOccurred = true;
        Serial.println(">>> FACTORY RESET TRIGGERED <<<");
    } 
    else if (_activeBootCount == _wifiBoots) {
        WiFiManager wm;
        wm.resetSettings();
        _resetOccurred = true;
        Serial.println(">>> WIFI RESET TRIGGERED <<<");
    }
}

void RapidBootWiFi::connect() {
  // 1. ONLY launch the blocking WiFiManager if the user intentionally triggered
  // the 3-rapid-boot reset. Ignore the lack of credentials otherwise.
  if (_resetOccurred) {
    Serial.println(">>> Rapid Boot Detected! Opening WiFiManager Portal <<<");
    
    WiFiManager wifiManager;
    _loadCustomParams();
    for (auto p : _customParams) { wifiManager.addParameter(p); }

    // Give the admin 3 minutes to enter credentials
    wifiManager.setConfigPortalTimeout(180); 

    // AutoConnect will now act purely as an On-Demand Portal
    if (!wifiManager.autoConnect(_apName)) {
      Serial.println("Portal timeout. Rebooting to offline mode...");
      delay(1000);
      ESP.restart(); // Restart so it boots normally and the door works again
    }
    _saveCustomParams();
  } else {
    Serial.println("Normal boot. Proceeding directly to loop to ensure instant door operation...");
  }

  // 2. The "Waiting Room" Window
  // We still wait 3 seconds here so the ESP can catch intentional rapid power cycles
  unsigned long startTime = millis();
  while (millis() - startTime < _timeoutMs) { 
    yield(); 
  }

  // 3. We survived the boot window without a reset, clear the counter
  _writeBootCount(0);
}

int RapidBootWiFi::getCurrentBootCount() { return _activeBootCount; }

bool RapidBootWiFi::wasWiFiReset() { return _resetOccurred; }

// --- Private Helpers ---
int RapidBootWiFi::_readBootCount() {
    File f = LittleFS.open("/boot_count.txt", "r");
    if (!f) return 0;
    int c = f.readString().toInt();
    f.close();
    return c;
}

void RapidBootWiFi::_writeBootCount(int count) {
    File f = LittleFS.open("/boot_count.txt", "w");
    if (f) { f.print(String(count)); f.close(); }
}

void RapidBootWiFi::loop() {
    if (WiFi.status() != WL_CONNECTED && millis() - _lastWifiCheck >= 10000) {
        WiFi.reconnect();
        _lastWifiCheck = millis();
    }
}

const char* RapidBootWiFi::getParameterValue(const char* id) {
    for (auto p : _customParams) {
        if (strcmp(p->getID(), id) == 0) return p->getValue();
    }
    return "";
}

void RapidBootWiFi::_loadCustomParams() {
    for (auto p : _customParams) {
        String path = String("/param_") + p->getID() + ".txt";
        File f = LittleFS.open(path, "r");
        if (f) {
            String val = f.readString();
            f.close();
            strncpy((char*)p->getValue(), val.c_str(), p->getValueLength() - 1);
        }
    }
}

void RapidBootWiFi::_saveCustomParams() {
    for (auto p : _customParams) {
        String path = String("/param_") + p->getID() + ".txt";
        File f = LittleFS.open(path, "w");
        if (f) { f.print(p->getValue()); f.close(); }
    }
}

void RapidBootWiFi::addParameter(const char* id, const char* placeholder, const char* defaultValue, int length) {
    WiFiManagerParameter* newParam = new WiFiManagerParameter(id, placeholder, defaultValue, length);
    _customParams.push_back(newParam);
    _libraryOwnsParam.push_back(true);
}

void RapidBootWiFi::openPortal() {
    WiFiManager wifiManager;
    _loadCustomParams();
    for (auto p : _customParams) { wifiManager.addParameter(p); }
    if (!wifiManager.startConfigPortal(_apName)) {
        Serial.println("Portal Exit");
    }
    _saveCustomParams();
    ESP.restart();
}

void RapidBootWiFi::setAPName(const char* newAPName) {
    if (newAPName != nullptr && strlen(newAPName) > 0) {
        _apName = newAPName;
    }
}

void RapidBootWiFi::setTimeout(unsigned long newTimeoutMs) {
    _timeoutMs = newTimeoutMs;
}

void RapidBootWiFi::setBootThresholds(int wifiBoots, int factoryBoots) {
    if (wifiBoots > 0 && factoryBoots > wifiBoots) {
        _wifiBoots = wifiBoots;
        _factoryBoots = factoryBoots;
    }
}

// The overloaded version where the user passes their own parameter pointer
void RapidBootWiFi::addParameter(WiFiManagerParameter* customParam) {
    _customParams.push_back(customParam);
    _libraryOwnsParam.push_back(false); // User owns this, we won't delete it
}