#pragma once

#include "WiFiNetwork.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>

/**
 * @brief Advanced WiFi network scanner using airodump-ng
 */
class WiFiScanner {
public:
    /**
     * @brief Constructor
     * @param scanTime Time in seconds to scan for networks (default: 10 seconds)
     */
    explicit WiFiScanner(int scanTime = 10);

    /**
     * @brief Destructor
     */
    ~WiFiScanner();

    /**
     * @brief Scan for WiFi networks using airodump-ng
     * @param interface Network interface to use (default: wlan0mon)
     * @return Vector of discovered WiFi networks
     */
    std::vector<WiFiNetwork> scanNetworks(const std::string& interface = "wlan0mon");

    /**
     * @brief Get detailed information for a specific network
     * @param bssid BSSID of the network to get details for
     * @return WiFiNetwork object if found, nullptr otherwise
     */
    std::shared_ptr<WiFiNetwork> getNetworkDetails(const std::string& bssid) const;

    /**
     * @brief Stop any ongoing scan
     */
    void stopScan();

private:
    int scanTime_;
    std::vector<WiFiNetwork> networks_;
    mutable std::mutex networksMutex_;
    bool stopRequested_;
    std::string scanResultsFile_;

    /**
     * @brief Parse airodump-ng CSV results
     * @param csvFile Path to the CSV file to parse
     * @return Vector of parsed WiFi networks
     */
    std::vector<WiFiNetwork> parseAirodumpResults(const std::string& csvFile) const;

    /**
     * @brief Execute a shell command and get its output
     * @param cmd Command to execute
     * @return Command output as string
     */
    std::string execCommand(const std::string& cmd) const;
};
