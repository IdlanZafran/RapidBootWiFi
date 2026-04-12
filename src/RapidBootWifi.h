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
    RapidBootWiFi(const char* apName = "Smart_Device_Setup", unsigned long timeoutMs = 3000, int maxBoots = 3);
    
    // Destructor to prevent memory leaks
    ~RapidBootWiFi();
    
    // Core Functions
    void begin(); 
    void loop();

    // Setters
    void setAPName(const char* newAPName);
    void setMaxBoots(int newMaxBoots);
    void setTimeout(unsigned long newTimeoutMs);
    
    // Parameter Injection
    void addParameter(WiFiManagerParameter* customParam); // Original way (User manages memory)
    void addParameter(const char* id, const char* placeholder, const char* defaultValue, int length); // New easy way (Library manages memory)

    // Get parameter value by ID
    const char* getParameterValue(const char* id);
    
  private:
    const char* _apName;
    unsigned long _timeoutMs;
    int _maxBoots;
    
    unsigned long _lastWifiCheck;
    
    // Stores pointers to the parameters
    std::vector<WiFiManagerParameter*> _customParams; 
    // Tracks which parameters the library created so we can safely delete them
    std::vector<bool> _libraryOwnsParam; 
    
    int _readBootCount();
    void _writeBootCount(int count);
    
    void _loadCustomParams();
    void _saveCustomParams();
};

// Pre-create the object for the user, just like Serial or WiFi
extern RapidBootWiFi myWiFi;

#endif