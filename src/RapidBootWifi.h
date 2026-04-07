/** 
 * Library: RapidBootWiFi
 * Description: Handles 3-rapid-boot WiFi resets and WiFiManager setup
 * @author Idlan Zafran Mohd Zaidie
 * Date: 2026
 * Note: You are free to use this code, but please leave this header intact.
 */

#ifndef RAPID_BOOT_WIFI_H
#define RAPID_BOOT_WIFI_H

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFiManager.h>
#include <vector>

class RapidBootWiFi {
  public:
    RapidBootWiFi(const char* apName = "Smart_Device_Setup", unsigned long timeoutMs = 3000, int maxBoots = 3);
    
    // Core Function
    void begin(); 
    void loop();

    // Setters
    void setAPName(const char* newAPName);
    void setMaxBoots(int newMaxBoots);
    void setTimeout(unsigned long newTimeoutMs);
    void addParameter(WiFiManagerParameter* customParam);

  private:
    const char* _apName;
    unsigned long _timeoutMs;
    int _maxBoots;
    
    unsigned long _lastWifiCheck;
    
    std::vector<WiFiManagerParameter*> _customParams; 
    
    int _readBootCount();
    void _writeBootCount(int count);
    
    void _loadCustomParams();
    void _saveCustomParams();
};

#endif