#include "../include/WiFiNetwork.h"
#include <algorithm>

WiFiNetwork::WiFiNetwork() 
    : signal(0), channel(1), clients(0) {}

WiFiNetwork::WiFiNetwork(const std::string& ssid, const std::string& bssid, 
                       int signal, int channel, 
                       const std::string& encryption, 
                       const std::string& security, 
                       int clients)
    : ssid(ssid), bssid(bssid), signal(signal), 
      channel(channel), encryption(encryption), 
      security(security.empty() ? determineSecurity(encryption) : security), 
      clients(clients) {}

std::string WiFiNetwork::determineSecurity(const std::string& encryption) {
    std::string encLower = encryption;
    std::transform(encLower.begin(), encLower.end(), encLower.begin(), ::tolower);
    
    if (encLower.find("wpa3") != std::string::npos) {
        return "WPA3";
    } else if (encLower.find("wpa2") != std::string::npos) {
        return "WPA2";
    } else if (encLower.find("wpa") != std::string::npos) {
        return "WPA";
    } else if (encLower.find("wep") != std::string::npos) {
        return "WEP";
    } else if (encLower.find("open") != std::string::npos) {
        return "OPEN";
    }
    
    return "UNKNOWN";
}
