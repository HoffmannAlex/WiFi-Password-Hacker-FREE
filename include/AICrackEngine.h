#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include "AIPasswordGenerator.h"

/**
 * @brief AI-powered WPA password cracking engine
 */
class AICrackEngine {
public:
    /**
     * @brief Constructor
     */
    AICrackEngine();

    /**
     * @brief Destructor
     */
    ~AICrackEngine();

    /**
     * @brief Crack handshake using aircrack-ng with AI wordlist
     * @param captureFile Path to the capture file containing the handshake
     * @param wordlistFile Path to the wordlist file
     * @param ssid SSID of the target network (for context-aware generation)
     * @return The cracked password if successful, empty string otherwise
     */
    std::string crackHandshake(const std::string& captureFile, 
                              const std::string& wordlistFile,
                              const std::string& ssid);

    /**
     * @brief Stop the cracking process
     */
    void stopCracking();

    /**
     * @brief Check if cracking is in progress
     * @return true if cracking is in progress, false otherwise
     */
    bool isCracking() const;

private:
    std::atomic<bool> isCracking_;
    std::unique_ptr<std::thread> crackThread_;
    mutable std::mutex mutex_;
    std::unique_ptr<AIPasswordGenerator> passwordGenerator_;
    
    // Process handle for the aircrack-ng process
#ifdef _WIN32
    void* processHandle_;
#else
    int processId_;
#endif

    /**
     * @brief Extract BSSID from capture file
     * @param captureFile Path to the capture file
     * @return Extracted BSSID or empty string if not found
     */
    std::string extractBssid(const std::string& captureFile) const;

    /**
     * @brief Execute a command and capture its output
     * @param command Command to execute
     * @return Command output as string
     */
    std::string executeCommand(const std::string& command) const;

    /**
     * @brief Terminate the running process
     */
    void terminateProcess();
};
