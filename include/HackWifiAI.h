#pragma once

#include "WiFiScanner.h"
#include "HandshakeCapturer.h"
#include "DeauthAttacker.h"
#include "AICrackEngine.h"
#include "SecurityAnalyzer.h"
#include <string>
#include <memory>

/**
 * @brief Main class for AI-powered WiFi security testing
 */
class HackWifiAI {
public:
    /**
     * @brief Constructor
     */
    HackWifiAI();

    /**
     * @brief Main execution flow of the application
     * @return 0 on success, non-zero on error
     */
    int run();

private:
    // Core components
    std::unique_ptr<WiFiScanner> scanner_;
    std::unique_ptr<HandshakeCapturer> capturer_;
    std::unique_ptr<DeauthAttacker> deauth_;
    std::unique_ptr<AICrackEngine> cracker_;
    std::unique_ptr<SecurityAnalyzer> analyzer_;
    
    // Target network information
    WiFiNetwork targetNetwork_;
    std::string captureFile_;
    std::string interface_;

    /**
     * @brief Display the application banner
     */
    void displayBanner() const;

    /**
     * @brief Display legal warnings and get confirmation
     * @return true if user confirms, false otherwise
     */
    bool showLegalWarning() const;

    /**
     * @brief Set up the required environment and permissions
     * @return true if setup is successful, false otherwise
     */
    bool setupEnvironment();

    /**
     * @brief Scan for available WiFi networks
     * @return Vector of discovered WiFi networks
     */
    std::vector<WiFiNetwork> scanNetworks();

    /**
     * @brief Select a target network from the scanned list
     * @param networks List of available networks
     * @return true if selection is successful, false otherwise
     */
    bool selectTarget(const std::vector<WiFiNetwork>& networks);

    /**
     * @brief Capture WPA handshake from the target network
     * @return true if handshake is captured, false otherwise
     */
    bool captureHandshake();

    /**
     * @brief Perform AI-powered password cracking
     * @return The cracked password if successful, empty string otherwise
     */
    std::string crackPassword();

    /**
     * @brief Run security analysis on the target network
     * @return Security report
     */
    SecurityReport runSecurityAnalysis();

    /**
     * @brief Display the final results
     * @param password The cracked password (or empty if not found)
     * @param report The security analysis report
     */
    void displayResults(const std::string& password, const SecurityReport& report) const;

    /**
     * @brief Clean up resources and disable monitor mode
     */
    void cleanup() const;

    /**
     * @brief Check if the application is running with root/administrator privileges
     * @return true if running with elevated privileges, false otherwise
     */
    bool isRunningAsRoot() const;

    /**
     * @brief Check if required tools are installed
     * @return true if all required tools are available, false otherwise
     */
    bool checkRequiredTools() const;
};
