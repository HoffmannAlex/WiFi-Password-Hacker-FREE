#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ctime>
#include "WiFiNetwork.h"

/**
 * @brief Security assessment report for a WiFi network
 */
struct SecurityReport {
    std::string ssid;
    std::string bssid;
    std::string securityLevel;
    std::string encryption;
    int signalStrength;
    int channel;
    std::vector<std::string> recommendations;
    int riskScore;
    std::vector<std::string> vulnerabilities;
    std::time_t timestamp;
};

/**
 * @brief Analyzes WiFi network security posture
 */
class SecurityAnalyzer {
public:
    /**
     * @brief Constructor
     */
    SecurityAnalyzer();

    /**
     * @brief Perform comprehensive security analysis of a target network
     * @param network The WiFi network to analyze
     * @return Security report containing analysis results
     */
    SecurityReport analyzeNetwork(const WiFiNetwork& network);

    /**
     * @brief Generate and display a human-readable security report
     * @param report The security report to display
     */
    void generateReport(const SecurityReport& report) const;

    /**
     * @brief Save the security report to a JSON file
     * @param report The security report to save
     * @return Path to the saved report file, or empty string on failure
     */
    std::string saveReport(const SecurityReport& report) const;

private:
    /**
     * @brief Check if the SSID appears to be a default router SSID
     */
    bool isDefaultSsid(const std::string& ssid) const;

    /**
     * @brief Check if the SSID contains potentially sensitive information
     */
    bool containsSensitiveInfo(const std::string& ssid) const;

    /**
     * @brief Ensure the reports directory exists
     */
    void ensureReportsDirectory() const;

    // Default SSID patterns to detect
    const std::vector<std::string> defaultSsidPatterns_ = {
        "linksys", "netgear", "dlink", "tp-link", "asus", 
        "belkin", "cisco", "arris", "default", "wireless",
        "router", "modem"
    };

    // Sensitive information patterns to detect in SSIDs
    const std::vector<std::string> sensitiveInfoPatterns_ = {
        "admin", "password", "security", "wifi", "internet",
        "company", "corp", "inc", "office", "home"
    };
};
