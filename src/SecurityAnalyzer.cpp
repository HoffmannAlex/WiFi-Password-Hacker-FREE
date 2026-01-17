#include "../include/SecurityAnalyzer.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace fs = std::filesystem;

SecurityAnalyzer::SecurityAnalyzer() = default;

SecurityReport SecurityAnalyzer::analyzeNetwork(const WiFiNetwork& network) {
    int riskScore = 0;
    std::vector<std::string> vulnerabilities;
    std::vector<std::string> recommendations;

    // Analyze encryption
    if (network.security == "WEP" || network.security == "OPEN") {
        riskScore += 80;
        vulnerabilities.push_back("Weak encryption");
        recommendations.push_back("Upgrade to WPA2/WPA3");
    } else if (network.security == "WPA") {
        riskScore += 60;
        vulnerabilities.push_back("Deprecated WPA encryption");
        recommendations.push_back("Upgrade to WPA2/WPA3");
    } else if (network.security == "WPA2") {
        riskScore += 30;
        vulnerabilities.push_back("Potential KRACK vulnerability");
        recommendations.push_back("Consider WPA3 upgrade");
    } else if (network.security == "WPA3") {
        riskScore += 10;
        recommendations.push_back("Good security practice");
    }

    // Analyze signal strength
    if (network.signal > 80) {
        riskScore += 5;
        recommendations.push_back("Consider reducing transmit power");
    } else if (network.signal < 20) {
        riskScore += 20;
        vulnerabilities.push_back("Weak signal may indicate distance issues");
    }

    // Analyze channel
    if (network.channel >= 1 && network.channel <= 11) {
        riskScore += 5;  // 2.4GHz crowded spectrum
    } else if (network.channel >= 36) {
        riskScore += 0;  // 5GHz less crowded
    }

    // SSID analysis
    if (isDefaultSsid(network.ssid)) {
        riskScore += 25;
        vulnerabilities.push_back("Default SSID detected");
        recommendations.push_back("Change default SSID");
    }

    if (containsSensitiveInfo(network.ssid)) {
        riskScore += 30;
        vulnerabilities.push_back("SSID contains sensitive information");
        recommendations.push_back("Change SSID to remove sensitive info");
    }

    // Determine risk level
    std::string riskLevel;
    if (riskScore >= 70) {
        riskLevel = "HIGH";
    } else if (riskScore >= 40) {
        riskLevel = "MEDIUM";
    } else {
        riskLevel = "LOW";
    }

    SecurityReport report{
        .ssid = network.ssid,
        .bssid = network.bssid,
        .securityLevel = riskLevel,
        .encryption = network.security,
        .signalStrength = network.signal,
        .channel = network.channel,
        .recommendations = std::move(recommendations),
        .riskScore = riskScore,
        .vulnerabilities = std::move(vulnerabilities),
        .timestamp = std::time(nullptr)
    };

    return report;
}

void SecurityAnalyzer::generateReport(const SecurityReport& report) const {
    std::cout << "\n📊 SECURITY ANALYSIS REPORT" << std::endl;
    std::cout << "   SSID: " << report.ssid << std::endl;
    std::cout << "   BSSID: " << report.bssid << std::endl;
    std::cout << "   Risk Level: " << report.securityLevel << std::endl;
    std::cout << "   Risk Score: " << report.riskScore << "/100" << std::endl;
    std::cout << "   Encryption: " << report.encryption << std::endl;
    std::cout << "   Signal: " << report.signalStrength << "%" << std::endl;

    if (!report.vulnerabilities.empty()) {
        std::cout << "\n   🔴 VULNERABILITIES:" << std::endl;
        for (const auto& vuln : report.vulnerabilities) {
            std::cout << "      • " << vuln << std::endl;
        }
    }

    if (!report.recommendations.empty()) {
        std::cout << "\n   ✅ RECOMMENDATIONS:" << std::endl;
        for (const auto& rec : report.recommendations) {
            std::cout << "      • " << rec << std::endl;
        }
    }
}

std::string SecurityAnalyzer::saveReport(const SecurityReport& report) const {
    ensureReportsDirectory();
    
    // Create a safe filename from SSID
    std::string safeSsid = report.ssid;
    std::replace_if(safeSsid.begin(), safeSsid.end(),
                   [](char c) { return !std::isalnum(c) && c != '-' && c != '_'; }, '_');
    
    // Format timestamp
    std::time_t t = report.timestamp;
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    std::string timestamp = oss.str();
    
    std::string filename = "reports/security_analysis_" + safeSsid + "_" + timestamp + ".json";
    
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "❌ Failed to open file for writing: " << filename << std::endl;
            return "";
        }
        
        // Convert report to JSON
        file << "{\n";
        file << "  \"ssid\": \"" << report.ssid << "\",\n";
        file << "  \"bssid\": \"" << report.bssid << "\",\n";
        file << "  \"security_level\": \"" << report.securityLevel << "\",\n";
        file << "  \"encryption\": \"" << report.encryption << "\",\n";
        file << "  \"signal_strength\": " << report.signalStrength << ",\n";
        file << "  \"channel\": " << report.channel << ",\n";
        file << "  \"risk_score\": " << report.riskScore << ",\n";
        
        // Vulnerabilities array
        file << "  \"vulnerabilities\": [";
        for (size_t i = 0; i < report.vulnerabilities.size(); ++i) {
            if (i > 0) file << ", ";
            file << "\"" << report.vulnerabilities[i] << "\"";
        }
        file << "],\n";
        
        // Recommendations array
        file << "  \"recommendations\": [";
        for (size_t i = 0; i < report.recommendations.size(); ++i) {
            if (i > 0) file << ", ";
            file << "\"" << report.recommendations[i] << "\"";
        }
        file << "],\n";
        
        // Timestamp
        file << "  \"timestamp\": " << report.timestamp << "\n";
        file << "}";
        
        file.close();
        std::cout << "   📄 Report saved: " << filename << std::endl;
        return filename;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error saving report: " << e.what() << std::endl;
        return "";
    }
}

bool SecurityAnalyzer::isDefaultSsid(const std::string& ssid) const {
    std::string lowerSsid = ssid;
    std::transform(lowerSsid.begin(), lowerSsid.end(), lowerSsid.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    
    for (const auto& pattern : defaultSsidPatterns_) {
        if (lowerSsid.find(pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool SecurityAnalyzer::containsSensitiveInfo(const std::string& ssid) const {
    std::string lowerSsid = ssid;
    std::transform(lowerSsid.begin(), lowerSsid.end(), lowerSsid.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    
    for (const auto& pattern : sensitiveInfoPatterns_) {
        if (lowerSsid.find(pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void SecurityAnalyzer::ensureReportsDirectory() const {
    try {
        if (!fs::exists("reports")) {
            fs::create_directory("reports");
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create reports directory: " << e.what() << std::endl;
    }
}
