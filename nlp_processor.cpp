#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include "json.hpp"
#include "nlp_processor.h"

using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

NLPProcessor::NLPProcessor(const std::string& apiKey, Database& db)
    : apiKey(apiKey), database(db) {}

std::string NLPProcessor::getSchema() const {
    std::ostringstream schema;
    for (const auto& [tableName, table] : database.tables) {
        schema << "Table: " << table.name << "\nColumns:\n";
        for (const auto& col : table.columns) {
            schema << "- " << col.name << " (" << col.dataType << ")\n";
        }
    }
    return schema.str().empty() ? "No tables exist in the database yet." : schema.str();
}

std::string NLPProcessor::getCommandHistory() const {
    std::ostringstream history;
    for (const auto& [nl, sql] : commandHistory) {
        history << "NL Query: " << nl << "\nSQL: " << sql << "\n\n";
    }
    return history.str();
}

// Helper function to wrap text to a specific width
std::string wrapText(const std::string& text, size_t width) {
    std::istringstream words(text);
    std::ostringstream wrapped;
    std::string word;

    size_t line_length = 0;
    while (words >> word) {
        if (line_length + word.length() > width) {
            wrapped << "\n    "; // 4 spaces indent for continuation lines
            line_length = 4;
        } else if (line_length > 0) {
            wrapped << " ";
            line_length += 1;
        }
        wrapped << word;
        line_length += word.length();
    }
    return wrapped.str();
}

void NLPProcessor::printFormattedResponse(const std::string& nlQuery, const std::string& sqlQuery, const std::string& reasoning) {
    const int boxWidth = 75;
    std::string horizontalLine;
    for (int i = 0; i < boxWidth - 2; i++) horizontalLine += "━";
    
    // Format the box with double-line box drawing
    std::cout << Colors::CYAN << "\n┏" << horizontalLine << "┓" << Colors::RESET << std::endl;
    
    // Natural Language Query
    std::cout << Colors::CYAN << "┃" << Colors::RESET << Colors::BOLD << " Natural Language Query:" << Colors::RESET << std::endl;
    std::cout << Colors::CYAN << "┃" << Colors::RESET << " " << nlQuery << std::endl;
    
    // SQL Query
    std::cout << Colors::CYAN << "┃" << Colors::RESET << std::endl;
    std::cout << Colors::CYAN << "┃" << Colors::RESET << Colors::BOLD << " Generated SQL:" << Colors::RESET << std::endl;
    std::cout << Colors::CYAN << "┃" << Colors::RESET << " " << sqlQuery << std::endl;
    
    // Reasoning (with word wrap)
    if (!reasoning.empty()) {
        std::cout << Colors::CYAN << "┃" << Colors::RESET << std::endl;
        std::cout << Colors::CYAN << "┃" << Colors::RESET << Colors::BOLD << " Reasoning:" << Colors::RESET << std::endl;
        std::string wrappedReasoning = wrapText(reasoning, boxWidth - 6); // -6 for margin and box chars
        std::istringstream reasoningLines(wrappedReasoning);
        std::string line;
        while (std::getline(reasoningLines, line)) {
            std::cout << Colors::CYAN << "┃" << Colors::RESET << " " << line << std::endl;
        }
    }
    
    std::cout << Colors::CYAN << "┗" << horizontalLine << "┛" << Colors::RESET << std::endl;
}

std::string NLPProcessor::constructPrompt(const std::string& nlQuery) const {
    std::ostringstream prompt;
    prompt << "You are an expert SQL programmer. Your task is to convert natural language queries into valid SQL queries.\n\n";
    prompt << "Database Schema:\n" << getSchema() << "\n\n";
    
    // Add command history for context
    std::string history = getCommandHistory();
    if (!history.empty()) {
        prompt << "Previous commands for context:\n" << history << "\n\n";
    }

    prompt << "Natural Language Query: \"" << nlQuery << "\"\n\n";
    prompt << "Guidelines:\n";
    prompt << "1. For CREATE TABLE queries, include at least one column definition (e.g., 'id INT')\n";
    prompt << "2. Supported column types are: INT, TEXT, REAL\n";
    prompt << "3. Return your response in this JSON format:\n";
    prompt << "{\n";
    prompt << "    \"sql\": \"<generated SQL query>\",\n";
    prompt << "    \"reasoning\": \"<brief explanation of how you interpreted the query>\"\n";
    prompt << "}\n";
    return prompt.str();
}





void NLPProcessor::executeNLQuery(const std::string& nlQuery) {
    try {
        std::string response = callGeminiAPI(constructPrompt(nlQuery));
        
        // Parse JSON response
        json responseJson = json::parse(response);
        std::string sqlQuery = responseJson["sql"].get<std::string>();
        std::string reasoning = responseJson["reasoning"].get<std::string>();
        
        // Print formatted response
        printFormattedResponse(nlQuery, sqlQuery, reasoning);
        
        // Execute the query
        database.executeQuery(sqlQuery);
        
        // Add to command history
        if (commandHistory.size() >= MAX_HISTORY_SIZE) {
            commandHistory.erase(commandHistory.begin());
        }
        commandHistory.emplace_back(nlQuery, sqlQuery);
        
        std::cout << Colors::BOLD << Colors::GREEN << " Query executed successfully!" << Colors::RESET << "\n\n";
    } catch (const json::exception& e) {
        std::cerr << Colors::RED << "Error parsing API response: " << e.what() << Colors::RESET << "\n";
    } catch (const std::runtime_error& e) {
        std::cerr << Colors::RED << "Error executing query: " << e.what() << Colors::RESET << "\n";
    }
}

std::string NLPProcessor::callGeminiAPI(const std::string& prompt) const {
    CURL* curl;
    CURLcode res;
    std::string responseString;
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + apiKey;

    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    json requestJson = {
        {"contents", {{{"parts", {{{"text", prompt}}}}}}}
    };
    std::string jsonPayload = requestJson.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    try {
        json responseJson = json::parse(responseString);
        if (responseJson.contains("candidates") && !responseJson["candidates"].empty()) {
            std::string text = responseJson["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();
            
            // Remove markdown code blocks if present
            size_t start = text.find("{\n");
            if (start == std::string::npos) {
                start = text.find("{");
            }
            if (start == std::string::npos) {
                throw std::runtime_error("Invalid response format: no JSON object found");
            }
            
            size_t end = text.rfind("}");
            if (end == std::string::npos) {
                throw std::runtime_error("Invalid response format: no closing brace found");
            }
            
            // Extract just the JSON part
            std::string jsonPart = text.substr(start, end - start + 1);
            return jsonPart;
        } else {
            throw std::runtime_error("No valid response from Gemini API");
        }
    } catch (const json::exception& e) {
        throw std::runtime_error("Failed to parse Gemini API response: " + std::string(e.what()));
    }
}

// Helper function to trim whitespace from both ends of a string
std::string trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) return ""; // String is all whitespace
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}