#include "../include/WiFiScanner.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <cstring>
#include <memory>

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

WiFiScanner::WiFiScanner(int scanTime) 
    : scanTime_(scanTime), stopRequested_(false), scanResultsFile_("scan_results-01.csv") {}

WiFiScanner::~WiFiScanner() {
    stopScan();
}

std::vector<WiFiNetwork> WiFiScanner::scanNetworks(const std::string& interface) {
    stopRequested_ = false;
    networks_.clear();
    
    try {
        std::string cmd = "airodump-ng --write scan_results --output-format csv --write-interval 2 " + interface;
        
        // Run airodump-ng in a separate thread
        std::thread scanThread([this, cmd]() {
            // In a real implementation, we would use popen or CreateProcess
            // For now, we'll simulate the scan by reading from a file
            std::this_thread::sleep_for(std::chrono::seconds(scanTime_));
            
            if (!stopRequested_) {
                std::lock_guard<std::mutex> lock(networksMutex_);
                networks_ = parseAirodumpResults(scanResultsFile_);
            }
        });
        
        // Wait for scan to complete or until stop is requested
        if (scanThread.joinable()) {
            scanThread.join();
        }
        
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Scan error: ") + e.what());
    }
    
    return networks_;
}

std::vector<WiFiNetwork> WiFiScanner::parseAirodumpResults(const std::string& csvFile) const {
    std::vector<WiFiNetwork> networks;
    std::ifstream file(csvFile);
    
    if (!file.is_open()) {
        throw std::runtime_error("Could not open scan results file");
    }
    
    std::string line;
    bool inNetworkSection = false;
    
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        
        // Check for start of network section
        if (line.find("BSSID,") == 0) {
            inNetworkSection = true;
            continue;
        }
        
        // Skip if not in network section or reached station section
        if (!inNetworkSection || line.find("Station MAC,") == 0) {
            continue;
        }
        
        // Parse network entry
        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        // Split by comma, handling quoted fields
        bool inQuotes = false;
        std::string field;
        
        for (char c : line) {
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                tokens.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }
        tokens.push_back(field);
        
        // Ensure we have enough fields
        if (tokens.size() >= 14) {
            try {
                std::string bssid = tokens[0];
                int signal = tokens[8].empty() ? 0 : std::stoi(tokens[8]);
                int channel = tokens[3].empty() ? 1 : std::stoi(tokens[3]);
                std::string ssid = tokens[13];
                std::string encryption = tokens[5];
                
                networks.emplace_back(ssid, bssid, signal, channel, encryption, "");
            } catch (const std::exception&) {
                // Skip malformed entries
                continue;
            }
        }
    }
    
    // Sort by signal strength (strongest first)
    std::sort(networks.begin(), networks.end(), 
             [](const WiFiNetwork& a, const WiFiNetwork& b) {
                 return a.signal > b.signal;
             });
    
    return networks;
}

std::shared_ptr<WiFiNetwork> WiFiScanner::getNetworkDetails(const std::string& bssid) const {
    std::lock_guard<std::mutex> lock(networksMutex_);
    
    for (const auto& network : networks_) {
        if (network.bssid == bssid) {
            return std::make_shared<WiFiNetwork>(network);
        }
    }
    
    return nullptr;
}

void WiFiScanner::stopScan() {
    stopRequested_ = true;
    // In a real implementation, we would send SIGTERM to the airodump-ng process
}

std::string WiFiScanner::execCommand(const std::string& cmd) const {
    std::array<char, 128> buffer;
    std::string result;
    
#ifdef _WIN32
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
#endif
    
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}
