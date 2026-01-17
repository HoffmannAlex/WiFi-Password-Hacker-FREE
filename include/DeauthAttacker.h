#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>

/**
 * @brief Sends deauthentication packets to force client reconnection
 */
class DeauthAttacker {
public:
    /**
     * @brief Constructor
     */
    DeauthAttacker();

    /**
     * @brief Destructor
     */
    ~DeauthAttacker();

    /**
     * @brief Start deauthentication attack in a separate thread
     * @param interface Network interface to use
     * @param targetBssid BSSID of the target access point
     * @param clientBssid Optional BSSID of specific client to target (broadcast if empty)
     * @param duration Duration of the attack in seconds
     */
    void startDeauthAttack(const std::string& interface,
                          const std::string& targetBssid,
                          const std::string& clientBssid = "",
                          int duration = 30);

    /**
     * @brief Stop the deauthentication attack
     */
    void stopAttack();

    /**
     * @brief Check if an attack is currently in progress
     * @return true if attacking, false otherwise
     */
    bool isAttacking() const;

private:
    std::atomic<bool> isAttacking_;
    std::unique_ptr<std::thread> attackThread_;
    mutable std::mutex mutex_;
    
    // Process handle for the aireplay-ng process
#ifdef _WIN32
    void* processHandle_;
#else
    int processId_;
#endif

    /**
     * @brief Worker function that runs in a separate thread to send deauth packets
     */
    void deauthWorker(const std::string& interface,
                     const std::string& targetBssid,
                     const std::string& clientBssid,
                     int duration);

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
