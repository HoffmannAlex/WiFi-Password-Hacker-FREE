#pragma once

#include <string>
#include <vector>

/**
 * @brief Data class for WiFi network information
 */
class WiFiNetwork {
public:
    std::string ssid;
    std::string bssid;
    int signal;
    int channel;
    std::string encryption;
    std::string security;
    int clients;

    /**
     * @brief Default constructor
     */
    WiFiNetwork();

    /**
     * @brief Parameterized constructor
     */
    WiFiNetwork(const std::string& ssid, const std::string& bssid, 
               int signal, int channel, 
               const std::string& encryption, 
               const std::string& security, 
               int clients = 0);

    /**
     * @brief Determine security level from encryption string
     */
    static std::string determineSecurity(const std::string& encryption);
};
