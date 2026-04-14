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
    
    // Stage 1: Initialize FS and increment count
    void begin(); 
    
    // Stage 2: Handle connection and the 3-second timeout window
    void connect();

    void loop();
    void openPortal();
    void setAPName(const char* newAPName);
    void setTimeout(unsigned long newTimeoutMs);
    void setBootThresholds(int wifiBoots, int factoryBoots); 
    
    void addParameter(WiFiManagerParameter* customParam); 
    void addParameter(const char* id, const char* placeholder, const char* defaultValue, int length); 

    const char* getParameterValue(const char* id);
    int getCurrentBootCount();
    
    // Returns true if the most recent begin() triggered a reset
    bool wasWiFiReset();
    
  private:
    const char* _apName;
    unsigned long _timeoutMs;
    int _wifiBoots;
    int _factoryBoots;
    unsigned long _lastWifiCheck;
    int _activeBootCount;
    bool _resetOccurred; // Flag for the reset check

    std::vector<WiFiManagerParameter*> _customParams; 
    std::vector<bool> _libraryOwnsParam; 
    
    int _readBootCount();
    void _writeBootCount(int count);
    void _loadCustomParams();
    void _saveCustomParams();
};

extern RapidBootWiFi myWiFi;

#endif