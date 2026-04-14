/** * Library: RapidBootWiFi
 * Description: Handles 3-rapid-boot WiFi resets and WiFiManager setup
 * @author Idlan Zafran Mohd Zaidie
 * Date: 2026
 * Note: You are free to use this code, but please leave this header intact.
 * @license MIT
 */

#ifndef RAPID_BOOT_WIFI_H
#define RAPID_BOOT_WIFI_H

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFiManager.h>
#include <vector>

class RapidBootWiFi {
  public:
    RapidBootWiFi(const char* apName = "Smart_Device_Setup", unsigned long timeoutMs = 3000, int wifiBoots = 3, int factoryBoots = 5);
    ~RapidBootWiFi();
    
    void begin(); 
    void loop();
    
    void openPortal();
    
    void setAPName(const char* newAPName);
    void setTimeout(unsigned long newTimeoutMs);
    
    // NEW: Function to change both thresholds
    void setBootThresholds(int wifiBoots, int factoryBoots); 
    
    void addParameter(WiFiManagerParameter* customParam); 
    void addParameter(const char* id, const char* placeholder, const char* defaultValue, int length); 

    const char* getParameterValue(const char* id);

    int getCurrentBootCount();
    int incrementAndGetBootCount(); 
    bool wasWiFiReset();
    
  private:
    const char* _apName;
    unsigned long _timeoutMs;
    
    // NEW: Two distinct counters
    int _wifiBoots;
    int _factoryBoots;
        
    unsigned long _lastWifiCheck;
    
    // Stores pointers to the parameters
    std::vector<WiFiManagerParameter*> _customParams; 
    // Tracks which parameters the library created so we can safely delete them
    std::vector<bool> _libraryOwnsParam; 
    
    int _readBootCount();
    int _activeBootCount;
    void _writeBootCount(int count);
    
    void _loadCustomParams();
    void _saveCustomParams();
};

// Pre-create the object for the user, just like Serial or WiFi
extern RapidBootWiFi myWiFi;

#endif
