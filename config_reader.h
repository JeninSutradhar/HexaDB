#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>
#include <fstream>
#include <stdexcept>

class ConfigReader {
public:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    static std::string getApiKey(const std::string& configFile = "config.txt") {
        std::ifstream file(configFile);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open config file. Please ensure config.txt exists with GEMINI_API_KEY=your_key");
        }

        std::string line;
        const std::string KEY_PREFIX = "GEMINI_API_KEY=";
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
            
            if (line.substr(0, KEY_PREFIX.length()) == KEY_PREFIX) {
                std::string key = trim(line.substr(KEY_PREFIX.length()));
                if (key.empty()) {
                    throw std::runtime_error("GEMINI_API_KEY is empty in config file");
                }
                if (key == "your_api_key_here") {
                    throw std::runtime_error("Please replace the default API key in config.txt with your actual Gemini API key");
                }
                return key;
            }
        }
        throw std::runtime_error("GEMINI_API_KEY not found in config file");
    }
};

#endif // CONFIG_READER_H
