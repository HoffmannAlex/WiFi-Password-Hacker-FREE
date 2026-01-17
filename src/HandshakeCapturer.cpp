#include "../include/HandshakeCapturer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <pdh.h>
#include <pdhmsg.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <spawn.h>
#include <cstdlib>
#endif

namespace fs = std::filesystem;

HandshakeCapturer::HandshakeCapturer() 
    : isCapturing_(false)
#ifdef _WIN32
    , processHandle_(nullptr)
#else
    , processId_(0)
#endif
{
}

HandshakeCapturer::~HandshakeCapturer() {
    stopCapture();
}

bool HandshakeCapturer::startCapture(const std::string& interface, 
                                   const std::string& targetBssid, 
                                   int channel) {
    std::lock_guard<std::mutex> lock(captureMutex_);
    
    // Stop any existing capture
    if (isCapturing_) {
        stopCapture();
    }

    try {
        // Create captures directory if it doesn't exist
        if (!fs::exists("captures")) {
            fs::create_directory("captures");
        }

        // Generate capture file name with timestamp
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count();
        
        captureFile_ = "captures/handshake_" + std::to_string(timestamp);
        
        // Build airodump-ng command
        std::string cmd = "airodump-ng";
        std::vector<std::string> args = {
            "--bssid", targetBssid,
            "--channel", std::to_string(channel),
            "--write", captureFile_,
            "--output-format", "pcap",
            interface
        };
        
        std::cout << "🎣 Starting capture: " << cmd;
        for (const auto& arg : args) {
            std::cout << " " << arg;
        }
        std::cout << std::endl;

#ifdef _WIN32
        // Windows implementation using CreateProcess
        std::string fullCmd = cmd;
        for (const auto& arg : args) {
            fullCmd += " " + arg;
        }
        
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        // Start the child process
        if (!CreateProcess(
            NULL,                   // No module name (use command line)
            const_cast<LPSTR>(fullCmd.c_str()), // Command line
            NULL,                   // Process handle not inheritable
            NULL,                   // Thread handle not inheritable
            FALSE,                  // Set handle inheritance to FALSE
            CREATE_NO_WINDOW,       // No console window
            NULL,                   // Use parent's environment block
            NULL,                   // Use parent's starting directory 
            &si,                    // Pointer to STARTUPINFO structure
            &pi                     // Pointer to PROCESS_INFORMATION structure
        )) {
            std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
            return false;
        }
        
        processHandle_ = pi.hProcess;
#else
        // Linux/Unix implementation using fork and exec
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process
            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(cmd.c_str()));
            
            for (const auto& arg : args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);
            
            // Execute airodump-ng
            execvp(cmd.c_str(), argv.data());
            
            // If we get here, execvp failed
            std::cerr << "Failed to execute airodump-ng" << std::endl;
            _exit(1);
        } else if (pid > 0) {
            // Parent process
            processId_ = pid;
        } else {
            // Fork failed
            std::cerr << "Fork failed" << std::endl;
            return false;
        }
#endif
        
        isCapturing_ = true;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Capture start error: " << e.what() << std::endl;
        return false;
    }
}

bool HandshakeCapturer::checkHandshake() {
    if (captureFile_.empty()) {
        return false;
    }
    
    std::string capFile = captureFile_ + "-01.cap";
    
    try {
        // First check if file exists and has reasonable size
        if (!fileExists(capFile)) {
            return false;
        }
        
        long fileSize = getFileSize(capFile);
        if (fileSize < 1000) {  // Reasonable minimum size for a capture with handshake
            return false;
        }
        
        // Use aircrack-ng to verify handshake
        std::string cmd = "aircrack-ng -J " + capFile;
        std::string output = executeCommand(cmd);
        
        if (output.find("WPA handshake") != std::string::npos) {
            return true;
        }
        
        // If aircrack-ng check fails, try alternative analysis
        return analyzeCapFile(capFile);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Handshake check error: " << e.what() << std::endl;
        return false;
    }
}

bool HandshakeCapturer::analyzeCapFile(const std::string& capFile) const {
    try {
        // Use tshark to analyze EAPOL packets
        std::string cmd = "tshark -r " + capFile + " -Y eapol -T fields -e wlan_mgt.fixed.cookie";
        std::string output = executeCommand(cmd);
        
        // Count non-empty lines (each line represents an EAPOL packet)
        std::istringstream iss(output);
        std::string line;
        int eapolCount = 0;
        
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                eapolCount++;
            }
        }
        
        // A complete handshake typically has 4 EAPOL packets
        return eapolCount >= 4;
        
    } catch (...) {
        return false;
    }
}

void HandshakeCapturer::stopCapture() {
    std::lock_guard<std::mutex> lock(captureMutex_);
    
    if (!isCapturing_) {
        return;
    }
    
    try {
#ifdef _WIN32
        if (processHandle_ != nullptr) {
            // Terminate the process
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
    } catch (const std::exception& e) {
        std::cerr << "Error stopping capture: " << e.what() << std::endl;
    }
    
    isCapturing_ = false;
}

std::string HandshakeCapturer::getCaptureFile() const {
    std::lock_guard<std::mutex> lock(captureMutex_);
    if (!captureFile_.empty()) {
        return captureFile_ + "-01.cap";
    }
    return "";
}

std::string HandshakeCapturer::executeCommand(const std::string& command) const {
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

bool HandshakeCapturer::fileExists(const std::string& path) const {
    struct stat buffer;   
    return (stat(path.c_str(), &buffer) == 0);
}

long HandshakeCapturer::getFileSize(const std::string& path) const {
    struct stat stat_buf;
    int rc = stat(path.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}
