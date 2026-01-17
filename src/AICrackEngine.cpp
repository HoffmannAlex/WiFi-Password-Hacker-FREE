#include "../include/AICrackEngine.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstring>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <spawn.h>
#endif

AICrackEngine::AICrackEngine() 
    : isCracking_(false)
    , passwordGenerator_(std::make_unique<AIPasswordGenerator>())
#ifdef _WIN32
    , processHandle_(nullptr)
#else
    , processId_(0)
#endif
{
}

AICrackEngine::~AICrackEngine() {
    stopCracking();
}

std::string AICrackEngine::crackHandshake(const std::string& captureFile,
                                       const std::string& wordlistFile,
                                       const std::string& ssid) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if capture file exists
    std::ifstream fileCheck(captureFile);
    if (!fileCheck.good()) {
        std::cerr << "❌ Capture file not found: " << captureFile << std::endl;
        return "";
    }
    
    // Check if wordlist file exists
    fileCheck = std::ifstream(wordlistFile);
    if (!fileCheck.good()) {
        std::cerr << "❌ Wordlist file not found: " << wordlistFile << std::endl;
        return "";
    }
    
    std::cout << "🔓 Cracking " << ssid << " with AI wordlist..." << std::endl;
    std::cout << "   Capture: " << captureFile << std::endl;
    std::cout << "   Wordlist: " << wordlistFile << std::endl;
    
    // Extract BSSID from capture file
    std::string bssid = extractBssid(captureFile);
    if (bssid.empty()) {
        std::cerr << "❌ Could not extract BSSID from capture file" << std::endl;
        return "";
    }
    
    // Build the aircrack-ng command
    std::vector<std::string> args = {
        "aircrack-ng",
        "-w", wordlistFile,
        "-b", bssid,
        captureFile
    };
    
    // Convert args to C-style array for execvp
    std::vector<char*> argv;
    for (const auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);
    
    // Create pipes for reading output
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "❌ Failed to create pipe" << std::endl;
        return "";
    }
    
    // Fork the process
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "❌ Failed to fork process" << std::endl;
        close(pipefd[0]);
        close(pipefd[1]);
        return "";
    }
    
    if (pid == 0) {
        // Child process
        close(pipefd[0]);  // Close read end of pipe
        
        // Redirect stdout and stderr to the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Execute aircrack-ng
        execvp("aircrack-ng", argv.data());
        
        // If we get here, execvp failed
        std::cerr << "❌ Failed to execute aircrack-ng" << std::endl;
        _exit(1);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end of pipe
        
        // Read from the pipe
        char buffer[1024];
        std::string output;
        ssize_t count;
        
        isCracking_ = true;
        processId_ = pid;
        
        while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            if (!isCracking_) {
                break;
            }
            
            buffer[count] = '\0';
            std::string line(buffer);
            output += line;
            
            // Check for successful password
            size_t keyPos = line.find("KEY FOUND");
            if (keyPos != std::string::npos) {
                // Extract password from line like: "KEY FOUND! [ password123 ]"
                size_t start = line.find('[', keyPos);
                size_t end = line.find(']', start);
                
                if (start != std::string::npos && end != std::string::npos) {
                    std::string password = line.substr(start + 1, end - start - 1);
                    // Trim whitespace
                    password.erase(0, password.find_first_not_of(" \t\n\r\f\v"));
                    password.erase(password.find_last_not_of(" \t\n\r\f\v") + 1);
                    
                    if (!password.empty()) {
                        std::cout << "✅ KEY FOUND: " << password << std::endl;
                        stopCracking();
                        close(pipefd[0]);
                        return password;
                    }
                }
            }
            
            // Show progress
            std::cout << line << std::flush;
        }
        
        close(pipefd[0]);
        
        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);
        isCracking_ = false;
        
        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            if (exitStatus != 0) {
                std::cerr << "❌ aircrack-ng exited with status: " << exitStatus << std::endl;
            }
        }
    }
    
    return "";
}

void AICrackEngine::stopCracking() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isCracking_) {
        return;
    }
    
    isCracking_ = false;
    terminateProcess();
}

bool AICrackEngine::isCracking() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return isCracking_;
}

std::string AICrackEngine::extractBssid(const std::string& captureFile) const {
    std::string cmd = "aircrack-ng -J " + captureFile;
    std::string output = executeCommand(cmd);
    
    // Parse BSSID from output
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.find("BSSID") != std::string::npos) {
            std::istringstream lineStream(line);
            std::string token;
            
            // Skip the first token (should be "BSSID")
            if (lineStream >> token && token == "BSSID" && lineStream >> token) {
                // Check if it looks like a MAC address
                if (token.size() == 17 && 
                    token[2] == ':' && token[5] == ':' && 
                    token[8] == ':' && token[11] == ':' && 
                    token[14] == ':') {
                    return token;
                }
            }
        }
    }
    
    return "";
}

std::string AICrackEngine::executeCommand(const std::string& command) const {
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

void AICrackEngine::terminateProcess() {
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
