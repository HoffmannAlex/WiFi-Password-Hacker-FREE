#pragma once

#include <string>
#include <vector>
#include <map>
#include <random>
#include <unordered_set>
#include <memory>

/**
 * @brief AI-powered password generation with context awareness
 */
class AIPasswordGenerator {
public:
    /**
     * @brief Constructor
     */
    AIPasswordGenerator();

    /**
     * @brief Generate a context-aware password based on SSID and attempt number
     * @param ssid The SSID of the target network
     * @param attempt Current attempt number (used to determine generation phase)
     * @return Generated password
     */
    std::string generateContextPassword(const std::string& ssid, int attempt);

    /**
     * @brief Clear the cache of previously generated passwords
     */
    void clearCache();

private:
    struct CommonPatterns {
        std::vector<std::string> commonWords;
        std::vector<std::string> commonSuffixes;
        std::vector<std::string> commonPrefixes;
        std::vector<std::string> keyboardPatterns;
        std::vector<char> specialChars;
        std::map<char, char> leetReplacements;
    } commonPatterns_;

    std::unordered_set<std::string> generatedPasswords_;
    std::mt19937 rng_;
    std::uniform_int_distribution<int> dist_;

    // Generation phases
    std::string phase1Generation(const std::string& ssid);
    std::string phase2Generation(const std::string& ssid);
    std::string phase3Generation(const std::string& ssid);
    std::string phase4Generation(const std::string& ssid);

    // Helper methods
    std::string cleanSsid(const std::string& ssid) const;
    std::string toLeetSpeak(const std::string& text) const;
    std::string generateRandom(int length) const;
    
    /**
     * @brief Load common password patterns
     */
    void loadCommonPatterns();
    
    /**
     * @brief Get a random element from a vector
     */
    template<typename T>
    const T& getRandomElement(const std::vector<T>& vec) const {
        std::uniform_int_distribution<size_t> dist(0, vec.size() - 1);
        return vec[dist(rng_)];
    }
};
