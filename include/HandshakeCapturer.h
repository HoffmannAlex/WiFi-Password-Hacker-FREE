#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>

/**
 * @brief Capture WPA handshakes using airodump-ng
 */
class HandshakeCapturer {
public:
    /**
     * @brief Constructor
     */
    HandshakeCapturer();

    /**
     * @brief Destructor
     */
    ~HandshakeCapturer();

    /**
     * @brief Start capturing handshake for target network
     * @param interface Network interface to use
     * @param targetBssid BSSID of the target network
     * @param channel Channel of the target network
     * @return true if capture started successfully, false otherwise
     */
    bool startCapture(const std::string& interface, 
                     const std::string& targetBssid, 
                     int channel);

    /**
     * @brief Check if WPA handshake has been captured
     * @return true if handshake is captured, false otherwise
     */
    bool checkHandshake();

    /**
     * @brief Stop the capture process
     */
    void stopCapture();

    /**
     * @brief Get the capture file path
     * @return Path to the capture file if available, empty string otherwise
     */
    std::string getCaptureFile() const;

private:
    std::atomic<bool> isCapturing_;
    std::string captureFile_;
    mutable std::mutex captureMutex_;
    
    // Process ID of the airodump-ng process
#ifdef _WIN32
    void* processHandle_;
#else
    pid_t processId_;
#endif

    /**
     * @brief Analyze cap file for handshake indicators
     * @param capFile Path to the capture file
     * @return true if handshake is detected, false otherwise
     */
    bool analyzeCapFile(const std::string& capFile) const;

    /**
     * @brief Execute a command and capture its output
     * @param command Command to execute
     * @return Command output as string
     */
    std::string executeCommand(const std::string& command) const;

    /**
     * @brief Check if a file exists
     * @param path Path to the file
     * @return true if file exists, false otherwise
     */
    bool fileExists(const std::string& path) const;

    /**
     * @brief Get file size in bytes
     * @param path Path to the file
     * @return File size in bytes, or -1 if error
     */
    long getFileSize(const std::string& path) const;
};
