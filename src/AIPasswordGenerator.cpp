#include "../include/AIPasswordGenerator.h"
#include <algorithm>
#include <cctype>
#include <random>
#include <chrono>

AIPasswordGenerator::AIPasswordGenerator() 
    : rng_(std::chrono::system_clock::now().time_since_epoch().count())
    , dist_(0, RAND_MAX) {
    loadCommonPatterns();
}

void AIPasswordGenerator::loadCommonPatterns() {
    commonPatterns_.commonWords = {
        "password", "admin", "wifi", "internet", "home", "office",
        "default", "wireless", "network", "secure", "security"
    };
    
    commonPatterns_.commonSuffixes = {"123", "1234", "12345", "!", "@", "#", "2024", "2025"};
    commonPatterns_.commonPrefixes = {"!", "#", "$", "admin", "wifi", "wireless", "secure"};
    
    commonPatterns_.keyboardPatterns = {
        "qwerty", "asdfgh", "zxcvbn", "1q2w3e", "1qaz2wsx", "qazwsx",
        "qwertyuiop", "asdfghjkl", "zxcvbnm"
    };
    
    commonPatterns_.specialChars = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
    
    commonPatterns_.leetReplacements = {
        {'a', '4'}, {'e', '3'}, {'i', '1'}, {'o', '0'}, 
        {'s', '5'}, {'t', '7'}, {'A', '4'}, {'E', '3'}, 
        {'I', '1'}, {'O', '0'}, {'S', '5'}, {'T', '7'}
    };
}

std::string AIPasswordGenerator::generateContextPassword(const std::string& ssid, int attempt) {
    // Phase-based generation strategy
    if (attempt < 1000) {
        return phase1Generation(ssid);
    } else if (attempt < 5000) {
        return phase2Generation(ssid);
    } else if (attempt < 20000) {
        return phase3Generation(ssid);
    } else {
        return phase4Generation(ssid);
    }
}

void AIPasswordGenerator::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    generatedPasswords_.clear();
}

std::string AIPasswordGenerator::phase1Generation(const std::string& ssid) {
    std::vector<std::string> patterns;
    
    // Common passwords
    patterns = commonPatterns_.commonWords;
    
    // Simple SSID variations
    std::string ssidClean = cleanSsid(ssid);
    if (!ssidClean.empty()) {
        for (const auto& suffix : commonPatterns_.commonSuffixes) {
            patterns.push_back(ssidClean + suffix);
            
            std::string lower = ssidClean;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            patterns.push_back(lower + suffix);
            
            std::string upper = ssidClean;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            patterns.push_back(upper + suffix);
        }
    }
    
    if (patterns.empty()) {
        return generateRandom(8);
    }
    
    return getRandomElement(patterns);
}

std::string AIPasswordGenerator::phase2Generation(const std::string& ssid) {
    std::vector<std::string> patterns = commonPatterns_.keyboardPatterns;
    
    // Leet speak variations
    std::string ssidClean = cleanSsid(ssid);
    if (!ssidClean.empty()) {
        std::string leetVersion = toLeetSpeak(ssidClean);
        patterns.push_back(leetVersion);
        patterns.push_back(leetVersion + "123");
        patterns.push_back(leetVersion + "!");
    }
    
    // Mixed case variations
    size_t initialSize = patterns.size();
    for (size_t i = 0; i < std::min(initialSize, static_cast<size_t>(10)); ++i) {
        std::string pattern = patterns[i];
        if (pattern.empty()) continue;
        
        // Capitalized version
        std::string capitalized = pattern;
        if (!capitalized.empty()) {
            capitalized[0] = static_cast<char>(::toupper(capitalized[0]));
            patterns.push_back(capitalized);
        }
        
        // Uppercase version
        std::string upper = pattern;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        patterns.push_back(upper);
        
        // Lowercase version
        std::string lower = pattern;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        patterns.push_back(lower);
    }
    
    if (patterns.empty()) {
        return generateRandom(10);
    }
    
    return getRandomElement(patterns);
}

std::string AIPasswordGenerator::phase3Generation(const std::string& ssid) {
    std::string base = cleanSsid(ssid);
    if (base.empty()) {
        base = "WiFi";
    }
    
    std::vector<std::function<std::string()>> strategies = {
        [this, &base]() {
            static std::uniform_int_distribution<int> digitDist(1000, 9999);
            return base + std::to_string(digitDist(rng_));
        },
        [this, &base]() {
            std::string result = base;
            if (!result.empty()) {
                result[0] = static_cast<char>(::toupper(result[0]));
            }
            if (!commonPatterns_.specialChars.empty()) {
                result += getRandomElement(commonPatterns_.specialChars);
            }
            return result;
        },
        [this, &base]() {
            std::string prefix = getRandomElement(commonPatterns_.commonPrefixes);
            return prefix + base + "123";
        },
        [this, &base]() {
            std::string leet = toLeetSpeak(base);
            std::string suffix = getRandomElement(commonPatterns_.commonSuffixes);
            return leet + suffix;
        },
        [this, &base]() {
            std::string special = commonPatterns_.specialChars.empty() ? "!" : 
                                std::string(1, getRandomElement(commonPatterns_.specialChars));
            
            static std::uniform_int_distribution<int> digitDist(100, 999);
            return base + special + std::to_string(digitDist(rng_));
        }
    };
    
    std::string password = getRandomElement(strategies)();
    if (password.length() < 8 || password.length() > 16) {
        return generateRandom(12);
    }
    
    return password;
}

std::string AIPasswordGenerator::phase4Generation(const std::string& ssid) {
    (void)ssid; // Unused parameter
    
    static const std::vector<std::string> structures = {
        "wwwwddddss",  // word word word word digit digit digit digit special
        "wwwddssww",   // word word word digit digit special special word word
        "ddwwwwssdd"   // digit digit word word word word special special digit digit
    };
    
    std::string structure = getRandomElement(structures);
    std::string password;
    
    for (char charType : structure) {
        if (charType == 'w') {  // word character
            static const std::string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            password += getRandomElement(std::vector<char>(letters.begin(), letters.end()));
        } else if (charType == 'd') {  // digit
            static const std::string digits = "0123456789";
            password += getRandomElement(std::vector<char>(digits.begin(), digits.end()));
        } else if (charType == 's' && !commonPatterns_.specialChars.empty()) {  // special
            password += getRandomElement(commonPatterns_.specialChars);
        }
    }
    
    return password;
}

std::string AIPasswordGenerator::cleanSsid(const std::string& ssid) const {
    std::string result;
    for (char c : ssid) {
        if (std::isalnum(c)) {
            result += c;
        }
    }
    return result.empty() ? "WiFi" : result;
}

std::string AIPasswordGenerator::toLeetSpeak(const std::string& text) const {
    std::string result;
    for (char c : text) {
        auto it = commonPatterns_.leetReplacements.find(c);
        if (it != commonPatterns_.leetReplacements.end()) {
            result += it->second;
        } else {
            result += c;
        }
    }
    return result;
}

std::string AIPasswordGenerator::generateRandom(int length) const {
    static const std::string chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "!@#$%^&*()";
    
    std::string result;
    std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);
    
    for (int i = 0; i < length; ++i) {
        result += chars[dist(rng_)];
    }
    
    return result;
}
