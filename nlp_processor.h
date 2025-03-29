#ifndef NLP_PROCESSOR_H
#define NLP_PROCESSOR_H

#include <string>
#include <vector>
#include <map>
#include "hexadb.h" // Assuming this includes necessary Database and Table definitions

class NLPProcessor {
public:
    NLPProcessor(const std::string& apiKey, Database& db);
    void executeNLQuery(const std::string& nlQuery);

private:
    std::string apiKey;
    Database& database;
    std::vector<std::pair<std::string, std::string>> commandHistory; // Stores <natural_query, sql_query> pairs
    static const size_t MAX_HISTORY_SIZE = 10; // Keep last 10 commands for context

    std::string getSchema() const;
    std::string constructPrompt(const std::string& nlQuery) const;
    std::string callGeminiAPI(const std::string& prompt) const;
    void printFormattedResponse(const std::string& nlQuery, const std::string& sqlQuery, const std::string& reasoning);
    std::string getCommandHistory() const;
};

#endif // NLP_PROCESSOR_H