#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <variant> // For handling different data types in rows
#include <iomanip> // For setw and formatting in printTable
#include <algorithm> // For std::transform and ::tolower

// Add these constants at the top of the file after includes
namespace Colors {
    const std::string RESET   = "\033[0m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string BOLD    = "\033[1m";
}

// Define supported data types
enum DataType {
    INT,
    TEXT,
    REAL
};

std::ostream& operator<<(std::ostream& os, const DataType& dt) {
    switch (dt) {
        case INT: os << "INT"; break;
        case TEXT: os << "TEXT"; break;
        case REAL: os << "REAL"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

// Type alias for values in a row (using variant to hold different types)
using Value = std::variant<int, std::string, double>;

std::ostream& operator<<(std::ostream& os, const Value& val) {
    std::visit([&](const auto& v){ os << v; }, val);
    return os;
}

struct ColumnDefinition {
    std::string name;
    DataType dataType;

    ColumnDefinition(std::string n, DataType dt) : name(n), dataType(dt) {}
};


class Table {
public:
    std::string name;
    std::vector<ColumnDefinition> columns;
    std::vector<std::vector<Value>> rows;
    std::map<std::string, std::map<Value, std::vector<int>>> indexes; // Column name -> (Value -> Row Indices)

    Table(std::string tableName) : name(tableName) {}

    void addColumn(const ColumnDefinition& colDef) {
        columns.push_back(colDef);
    }

    void insertRow(const std::vector<Value>& rowValues) {
        if (rowValues.size() != columns.size()) {
            throw std::runtime_error("Number of values doesn't match column count.");
        }
        rows.push_back(rowValues);
        int rowIndex = rows.size() - 1; // Index of the newly inserted row

        // Update indexes if any
        for (const auto& indexPair : indexes) {
            const std::string& colName = indexPair.first;
            int colIndex = getColumnIndex(colName);
            if (colIndex != -1) {
                indexes[colName][rowValues[colIndex]].push_back(rowIndex);
            }
        }
    }

    void createIndex(const std::string& columnName) {
        int colIndex = getColumnIndex(columnName);
        if (colIndex == -1) {
            throw std::runtime_error("Column not found for index creation.");
        }
        if (indexes.count(columnName)) {
            return; // Index already exists
        }

        indexes[columnName] = {}; // Initialize index map

        for (int i = 0; i < rows.size(); ++i) {
            indexes[columnName][rows[i][colIndex]].push_back(i);
        }
        std::cout << "Index created on column '" << columnName << "' for table '" << name << "'" << std::endl;
    }

    std::vector<std::vector<Value>> selectRows(const std::vector<std::string>& selectedColumns, const std::string& whereClause = "") {
        std::vector<std::vector<Value>> resultRows;
        std::vector<int> selectedColIndices;
        for (const auto& colName : selectedColumns) {
            int index = getColumnIndex(colName);
            if (index == -1) {
                throw std::runtime_error("Column '" + colName + "' not found in table '" + name + "'.");
            }
            selectedColIndices.push_back(index);
        }

        // Simple WHERE clause handling (very basic for demonstration)
        std::string whereColName;
        std::string whereOp;
        std::string whereValueStr;
        Value whereValue;

        if (!whereClause.empty()) {
            std::istringstream whereStream(whereClause);
            whereStream >> whereColName >> whereOp >> whereValueStr;
            int whereColIndex = getColumnIndex(whereColName);
            if (whereColIndex == -1) {
                throw std::runtime_error("Column '" + whereColName + "' in WHERE clause not found.");
            }
            DataType whereColType = columns[whereColIndex].dataType;
            if (whereColType == INT) {
                whereValue = std::stoi(whereValueStr);
            } else if (whereColType == REAL) {
                whereValue = std::stod(whereValueStr);
            } else { // TEXT
                whereValue = whereValueStr;
            }
            if (whereOp == "==") whereOp = "="; // Correct operator if user uses "=="
        }


        for (int i = 0; i < rows.size(); ++i) {
            bool conditionMet = true;
            if (!whereClause.empty()) {
                int whereColIndex = getColumnIndex(whereColName);
                Value rowValue = rows[i][whereColIndex];
                if (whereOp == "=") {
                    conditionMet = (rowValue == whereValue);
                } else if (whereOp == "!=") {
                    conditionMet = (rowValue != whereValue);
                } else if (whereOp == "<" && std::holds_alternative<int>(rowValue) && std::holds_alternative<int>(whereValue)) {
                    conditionMet = (std::get<int>(rowValue) < std::get<int>(whereValue));
                } else if (whereOp == ">" && std::holds_alternative<int>(rowValue) && std::holds_alternative<int>(whereValue)) {
                    conditionMet = (std::get<int>(rowValue) > std::get<int>(whereValue));
                } // ... add more operators as needed ...
                else {
                    conditionMet = false; // Unsupported operator or type comparison
                }
            }

            if (conditionMet) {
                std::vector<Value> selectedValues;
                for (int colIndex : selectedColIndices) {
                    selectedValues.push_back(rows[i][colIndex]);
                }
                resultRows.push_back(selectedValues);
            }
        }
        return resultRows;
    }

    void updateRows(const std::string& setClause, const std::string& whereClause = "") {
        std::istringstream setStream(setClause);
        std::string updatePair;
        while(std::getline(setStream, updatePair, ',')){ // Split by comma for multiple SETs
            std::istringstream pairStream(updatePair);
            std::string setColName;
            std::string setValueStr;
            std::getline(pairStream, setColName, '=');
            std::getline(pairStream, setValueStr);

            setColName = trim(setColName);
            setValueStr = trim(setValueStr);

            int setColIndex = getColumnIndex(setColName);
            if (setColIndex == -1) {
                throw std::runtime_error("Column '" + setColName + "' in SET clause not found.");
            }
            DataType setColType = columns[setColIndex].dataType;
            Value setValue;
            if (setColType == INT) {
                setValue = std::stoi(setValueStr);
            } else if (setColType == REAL) {
                setValue = std::stod(setValueStr);
            } else { // TEXT
                setValue = setValueStr;
            }


            std::string whereColName;
            std::string whereOp;
            std::string whereValueStr;
            Value whereValue;

            bool hasWhereClause = !whereClause.empty();
            if (hasWhereClause) {
                std::istringstream whereStream(whereClause);
                whereStream >> whereColName >> whereOp >> whereValueStr;
                 int whereColIndex = getColumnIndex(whereColName);
                if (whereColIndex == -1) {
                    throw std::runtime_error("Column '" + whereColName + "' in WHERE clause not found.");
                }
                DataType whereColType = columns[whereColIndex].dataType;
                if (whereColType == INT) {
                    whereValue = std::stoi(whereValueStr);
                } else if (whereColType == REAL) {
                    whereValue = std::stod(whereValueStr);
                } else { // TEXT
                    whereValue = whereValueStr;
                }
                if (whereOp == "==") whereOp = "="; // Correct operator if user uses "=="
            }


            for (int i = 0; i < rows.size(); ++i) {
                bool conditionMet = !hasWhereClause; // If no WHERE clause, update all
                if (hasWhereClause) {
                    int whereColIndex = getColumnIndex(whereColName);
                    Value rowValue = rows[i][whereColIndex];
                     if (whereOp == "=") {
                        conditionMet = (rowValue == whereValue);
                    } else if (whereOp == "!=") {
                        conditionMet = (rowValue != whereValue);
                    } else if (whereOp == "<" && std::holds_alternative<int>(rowValue) && std::holds_alternative<int>(whereValue)) {
                        conditionMet = (std::get<int>(rowValue) < std::get<int>(whereValue));
                    } else if (whereOp == ">" && std::holds_alternative<int>(rowValue) && std::holds_alternative<int>(whereValue)) {
                        conditionMet = (std::get<int>(rowValue) > std::get<int>(whereValue));
                    }
                }

                if (conditionMet) {
                    rows[i][setColIndex] = setValue;
                    // Update indexes if necessary (more complex, might require rebuilding index or efficient update)
                    if (indexes.count(setColName)) {
                        // For simplicity, let's rebuild index after updates. For efficiency, you'd need a smarter approach.
                        indexes[setColName].clear();
                        for (int j = 0; j < rows.size(); ++j) {
                            indexes[setColName][rows[j][setColIndex]].push_back(j);
                        }
                    }
                }
            }
        }
    }

    void deleteRows(const std::string& whereClause = "") {
        std::string whereColName;
        std::string whereOp;
        std::string whereValueStr;
        Value whereValue;

        bool hasWhereClause = !whereClause.empty();
        if (hasWhereClause) {
            std::istringstream whereStream(whereClause);
            whereStream >> whereColName >> whereOp >> whereValueStr;
            int whereColIndex = getColumnIndex(whereColName);
            if (whereColIndex == -1) {
                throw std::runtime_error("Column '" + whereColName + "' in WHERE clause not found.");
            }
             DataType whereColType = columns[whereColIndex].dataType;
            if (whereColType == INT) {
                whereValue = std::stoi(whereValueStr);
            } else if (whereColType == REAL) {
                whereValue = std::stod(whereValueStr);
            } else { // TEXT
                whereValue = whereValueStr;
            }
            if (whereOp == "==") whereOp = "="; // Correct operator if user uses "=="
        }

        std::vector<std::vector<Value>> newRows;
        for (int i = 0; i < rows.size(); ++i) {
            bool conditionMet = !hasWhereClause; // If no WHERE, delete all (be careful!)
            if (hasWhereClause) {
                int whereColIndex = getColumnIndex(whereColName);
                Value rowValue = rows[i][whereColIndex];
                if (whereOp == "=") {
                    conditionMet = (rowValue == whereValue);
                } else if (whereOp == "!=") {
                    conditionMet = (rowValue != whereValue);
                } else if (whereOp == "<" && std::holds_alternative<int>(rowValue) && std::holds_alternative<int>(whereValue)) {
                    conditionMet = (std::get<int>(rowValue) < std::get<int>(whereValue));
                } else if (whereOp == ">" && std::holds_alternative<int>(rowValue) && std::holds_alternative<int>(whereValue)) {
                    conditionMet = (std::get<int>(rowValue) > std::get<int>(whereValue));
                }
            }

            if (!conditionMet) {
                newRows.push_back(rows[i]); // Keep rows that do NOT meet the condition
            } else {
                // Row meets condition, so it should be deleted. Do not add to newRows.
            }
        }
        rows = newRows; // Replace with the filtered rows

        // Indexes might need to be rebuilt after delete for affected tables.
        for (auto const& [colName, indexMap] : indexes) {
            createIndex(colName); // Rebuild all indexes for simplicity after deletion. In real DB, more efficient update needed.
        }
    }


    // Update the printTable method in the Table class
    void printTable() const {
        // Calculate column widths
        std::vector<size_t> columnWidths;
        for (const auto& col : columns) {
            std::string headerText = col.name + "(" + dataTypeToString(col.dataType) + ")";
            columnWidths.push_back(headerText.length());
        }

        // Update widths based on data
        for (const auto& row : rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                std::stringstream ss;
                ss << row[i];
                columnWidths[i] = std::max(columnWidths[i], ss.str().length());
            }
        }

        // Print table name
        std::cout << Colors::BOLD << Colors::CYAN << "+== Table: " << name << Colors::RESET << std::endl;

        // Print header
        std::cout << Colors::BOLD << Colors::BLUE << "| ";
        for (size_t i = 0; i < columns.size(); ++i) {
            std::string headerText = columns[i].name + "(" + dataTypeToString(columns[i].dataType) + ")";
            std::cout << std::setw(columnWidths[i]) << std::left << headerText << " | ";
        }
        std::cout << Colors::RESET << "\n";

        // Print separator
        std::cout << Colors::YELLOW << "+";
        for (size_t i = 0; i < columns.size(); ++i) {
            std::cout << std::string(columnWidths[i] + 2, '-') << (i < columns.size() - 1 ? "+" : "+");
        }
        std::cout << Colors::RESET << "\n";

        // Print rows
        for (const auto& row : rows) {
            std::cout << Colors::CYAN << "| ";
            for (size_t i = 0; i < row.size(); ++i) {
                std::stringstream ss;
                ss << row[i];
                std::cout << std::setw(columnWidths[i]) << std::left << ss.str() << " | ";
            }
            std::cout << Colors::RESET << "\n";
        }

        // Print bottom border
        std::cout << Colors::YELLOW << "+";
        for (size_t i = 0; i < columns.size(); ++i) {
            std::cout << std::string(columnWidths[i] + 2, '-') << (i < columns.size() - 1 ? "+" : "+");
        }
        std::cout << Colors::RESET << "\n";

        // Print row count
        std::cout << Colors::GREEN << "► " << rows.size() << " row(s) in set" << Colors::RESET << "\n\n";
    }

private:
     std::string dataTypeToString(DataType dt) const { // Helper function for DataType to string conversion
        switch (dt) {
            case INT: return "INT";
            case TEXT: return "TEXT";
            case REAL: return "REAL";
            default: return "UNKNOWN";
        }
    }

public: // Made public as requested, though typically kept protected/private in real design
    int getColumnIndex(const std::string& columnName) const {
        for (int i = 0; i < columns.size(); ++i) {
            if (columns[i].name == columnName) {
                return i;
            }
        }
        return -1;
    }
    std::string trim(const std::string& str) { // Helper function to trim whitespace
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
};


class Database {
public:
    std::string name;
    std::map<std::string, Table> tables;

    Database(std::string dbName) : name(dbName) {}

    void createTable(const std::string& tableName, const std::vector<ColumnDefinition>& colDefs) {
        if (tables.count(toLower(tableName))) {
            throw std::runtime_error("Table '" + tableName + "' already exists.");
        }
        // Use emplace and get the iterator to the newly created Table
        auto [it, inserted] = tables.emplace(toLower(tableName), Table(tableName)); // Store table name in lowercase
        if (!inserted) {
            throw std::runtime_error("Failed to create table.");
        }
        Table& newTable = it->second; // Reference to the new Table
        for (const auto& colDef : colDefs) {
            newTable.addColumn(colDef); // Add columns using the reference
        }
        std::cout << "Table '" << tableName << "' created." << std::endl;
    }

    Table& getTable(const std::string& tableName) {
        if (!tables.count(toLower(tableName))) { // Lookup table name in lowercase
            throw std::runtime_error("Table '" + tableName + "' not found.");
        }
        return tables.at(toLower(tableName)); // Return table using lowercase name
    }

    void executeQuery(const std::string& sqlQuery) {
        std::istringstream queryStream(sqlQuery);
        std::string command;
        queryStream >> command;

        if (command == "CREATE") {
            std::string type;
            queryStream >> type;
            if (type == "TABLE") {
                parseCreateTable(queryStream);
            } else if (type == "INDEX") {
                parseCreateIndex(queryStream);
            } else {
                std::cout << "Unsupported CREATE type: " << type << std::endl;
            }
        } else if (command == "INSERT") {
            parseInsert(queryStream);
        } else if (command == "SELECT") {
            parseSelect(queryStream);
        } else if (command == "UPDATE") {
            parseUpdate(queryStream);
        } else if (command == "DELETE") {
            parseDelete(queryStream);
        } else if (command == "PRINT") {
            std::string type;
            queryStream >> type;
            if (type == "TABLE") {
                std::string tableName;
                queryStream >> tableName;
                try {
                    getTable(tableName).printTable();
                } catch (const std::runtime_error& error) {
                    std::cerr << "Error: " << error.what() << std::endl;
                }
            } else {
                std::cout << "Unsupported PRINT type: " << type << std::endl;
            }
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    void saveDatabase(const std::string& filename) const {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            throw std::runtime_error("Failed to open file for saving database.");
        }

        // Write database metadata
        outFile << "DATABASE_NAME " << name << std::endl;
        outFile << "TABLE_COUNT " << tables.size() << std::endl;

        // Save each table
        for (const auto& [tableName, table] : tables) {
            // Write table metadata
            outFile << "TABLE_NAME " << table.name << std::endl;
            outFile << "COLUMN_COUNT " << table.columns.size() << std::endl;

            // Write column definitions
            for (const auto& col : table.columns) {
                outFile << "COLUMN " << col.name << " " << static_cast<int>(col.dataType) << std::endl;
            }

            // Write row count
            outFile << "ROW_COUNT " << table.rows.size() << std::endl;

            // Write row data
            for (const auto& row : table.rows) {
                outFile << "ROW ";
                for (const auto& value : row) {
                    if (std::holds_alternative<int>(value)) {
                        outFile << "I " << std::get<int>(value) << " ";
                    } else if (std::holds_alternative<std::string>(value)) {
                        outFile << "T " << std::get<std::string>(value) << " ";
                    } else if (std::holds_alternative<double>(value)) {
                        outFile << "R " << std::get<double>(value) << " ";
                    }
                }
                outFile << std::endl;
            }
        }

        outFile.close();
        std::cout << "Database '" << name << "' saved to '" << filename << "'" << std::endl;
    }

    void loadDatabase(const std::string& filename) {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            throw std::runtime_error("Database file not found: " + filename);
        }

        std::string line, key;

        // Read database metadata
        inFile >> key >> name;

        int tableCount;
        inFile >> key >> tableCount;

        // Clear existing tables
        tables.clear();

        // Read each table
        for (int t = 0; t < tableCount; t++) {
            std::string tableName;
            inFile >> key >> tableName;

            int columnCount;
            inFile >> key >> columnCount;

            std::vector<ColumnDefinition> colDefs;

            // Read column definitions
            for (int c = 0; c < columnCount; c++) {
                std::string colName;
                int dataTypeInt;
                inFile >> key >> colName >> dataTypeInt;
                colDefs.emplace_back(colName, static_cast<DataType>(dataTypeInt));
            }

            // Create table
            createTable(tableName, colDefs);
            Table& table = getTable(tableName);

            // Read rows
            int rowCount;
            inFile >> key >> rowCount;

            for (int r = 0; r < rowCount; r++) {
                std::vector<Value> rowValues;
                inFile >> key; // "ROW"

                for (int c = 0; c < columnCount; c++) {
                    char valueType;
                    inFile >> valueType;

                    if (valueType == 'I') {
                        int val;
                        inFile >> val;
                        rowValues.push_back(val);
                    } else if (valueType == 'T') {
                        std::string val;
                        inFile >> val;
                        rowValues.push_back(val);
                    } else if (valueType == 'R') {
                        double val;
                        inFile >> val;
                        rowValues.push_back(val);
                    }
                }
                table.insertRow(rowValues);
            }
        }

        inFile.close();
        std::cout << "Database '" << name << "' loaded from '" << filename << "'" << std::endl;
    }

public:
    std::string trim(const std::string& str) { // Helper function to trim whitespace
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    std::string toLower(std::string str) { // Helper function to convert string to lowercase
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }


private:
        void parseCreateTable(std::istringstream& queryStream) {
        std::string tableName = ""; // Initialize tableName
        queryStream >> tableName;
        std::string columnsPart;
        std::getline(queryStream, columnsPart); // Get the rest of the line with column definitions

        columnsPart = trim(columnsPart); // Trim leading/trailing spaces from the entire column part
        if (columnsPart.front() != '(' || columnsPart.back() != ')') {
            throw std::runtime_error("Invalid column definition format. Expected parentheses.");
        }
        columnsPart = trim(columnsPart); // Trim leading/trailing spaces from the entire column part
        if (columnsPart.front() != '(' || columnsPart.back() != ')') {
            throw std::runtime_error("Invalid column definition format. Expected parentheses.");
        }
        columnsPart = columnsPart.substr(1, columnsPart.length() - 2); // Remove outer parentheses

        std::vector<ColumnDefinition> colDefs;
        std::istringstream colStream(columnsPart);
        std::string colDefStr;

        while (std::getline(colStream, colDefStr, ',')) {
            std::istringstream singleColStream(colDefStr);
            std::string colName, dataTypeStrRaw;
            singleColStream >> colName >> dataTypeStrRaw;
            colName = trim(colName);
            std::string dataTypeStr = trim(dataTypeStrRaw); // Trim spaces around data type

            // Remove any trailing parenthesis from dataTypeStr, just in case
            if (!dataTypeStr.empty() && dataTypeStr.back() == ')') {
                dataTypeStr.pop_back();
            }
            dataTypeStr = trim(dataTypeStr); // Trim again after removing ')' if any

            DataType dataType;
            if (dataTypeStr == "INT") dataType = INT;
            else if (dataTypeStr == "TEXT") dataType = TEXT;
            else if (dataTypeStr == "REAL") dataType = REAL;
            else throw std::runtime_error("Unknown data type: " + dataTypeStr);

            colDefs.emplace_back(colName, dataType);
        }
        createTable(tableName, colDefs); // Table name already extracted in parse call
    }

    void parseInsert(std::istringstream& queryStream) {
        std::string intoKeyword, tableName, columnsPart, valuesKeyword, valuesPart;
        queryStream >> intoKeyword >> tableName;
        if (toLower(intoKeyword) != "into") throw std::runtime_error("Expected INTO after INSERT");

        // Get columns part
        std::getline(queryStream, columnsPart, '(');
        std::getline(queryStream, columnsPart, ')');
        columnsPart = trim(columnsPart);

        // Get values part
        queryStream >> valuesKeyword;
        if (toLower(valuesKeyword) != "values") throw std::runtime_error("Expected VALUES after column list");

        std::getline(queryStream, valuesPart, '(');
        std::getline(queryStream, valuesPart, ')');
        valuesPart = trim(valuesPart);

        Table& table = getTable(tableName);

        // Parse column names
        std::vector<std::string> columnNames;
        std::istringstream colStream(columnsPart);
        std::string colName;
        while (std::getline(colStream, colName, ',')) {
            columnNames.push_back(trim(colName));
        }

        // Parse values
        std::vector<Value> rowValues;
        std::istringstream valueStream(valuesPart);
        std::string valueStr;
        int colIndex = 0;

        while (std::getline(valueStream, valueStr, ',')) {
            valueStr = trim(valueStr);
            std::string colName = columnNames[colIndex];
            DataType colType = table.columns[table.getColumnIndex(colName)].dataType;

            try {
                if (colType == TEXT) {
                    // Remove quotes from text values
                    if (valueStr.front() == '\'' && valueStr.back() == '\'') {
                        rowValues.push_back(valueStr.substr(1, valueStr.length() - 2));
                    } else {
                        rowValues.push_back(valueStr);
                    }
                } else if (colType == INT) {
                    // Handle integer values
                    rowValues.push_back(std::stoi(valueStr));
                } else if (colType == REAL) {
                    // Handle real/double values
                    rowValues.push_back(std::stod(valueStr));
                }
            } catch (const std::exception& e) {
                throw std::runtime_error("Error converting value '" + valueStr +
                    "' for column '" + colName + "': " + e.what());
            }
            colIndex++;
        }

        table.insertRow(rowValues);
        std::cout << "Row inserted into table '" << tableName << "'" << std::endl;
    }

    void parseSelect(std::istringstream& queryStream) {
       std::vector<std::string> selectColumns;
        std::string columnName;
        while (queryStream >> columnName && toLower(columnName) != "from") { // Changed condition to toLower and "!=" "from"
            if (columnName == "*") {
                selectColumns.push_back("*");
                if (queryStream.peek() == ',') queryStream.ignore();
                break;
            } else {
                selectColumns.push_back(trim(columnName));
                if (queryStream.peek() == ',') queryStream.ignore();
            }
        }

        std::string fromKeyword;
        if (!(queryStream >> fromKeyword)) {
            throw std::runtime_error("Expected FROM keyword after column list in SELECT query");
        }
        if (toLower(fromKeyword) != "from") { // Added case-insensitive check for "FROM" again, just in case
             throw std::runtime_error("Expected FROM keyword after column list in SELECT query");
        }


        std::string tableName;
        if (!(queryStream >> tableName)) {
            throw std::runtime_error("Expected table name after FROM in SELECT query");
        }
        tableName = trim(tableName);


        std::string whereClause = "";
        std::string whereKeyword;
        if (queryStream >> whereKeyword) { // Check if there is a next word
            if (toLower(whereKeyword) == "where") {
                std::string tempWhereClause;
                std::getline(queryStream, tempWhereClause);
                whereClause = trim(tempWhereClause);
            } else {
                queryStream.seekg(-whereKeyword.length(), std::ios::cur); // Rewind if WHERE not found
            }
        }

        Table& table = getTable(tableName);
        std::vector<std::vector<Value>> results;
        if (selectColumns.size() == 1 && selectColumns[0] == "*") {
            std::vector<std::string> allColumnNames;
            for (const auto& colDef : table.columns) {
                allColumnNames.push_back(colDef.name);
            }
            results = table.selectRows(allColumnNames, whereClause);
        } else {
            results = table.selectRows(selectColumns, whereClause);
        }

        // Print results
        if (!results.empty()) {
            std::vector<std::string> headersToPrint = (selectColumns.size() == 1 && selectColumns[0] == "*") ?
                                                        std::vector<std::string>() : selectColumns;
            if (headersToPrint.empty()) {
                std::cout << "-------------------------" << std::endl; // Separator before tabular output
                table.printTable(); // Call printTable for tabular format when SELECT *
            } else {
                // Table header for selected columns
                std::cout << "+";
                for (const auto& colName : headersToPrint) {
                    std::cout << std::string(colName.length() + 2, '-') << "+";
                }
                std::cout << std::endl << "|";
                for (const auto& colName : headersToPrint) {
                    std::cout << " " << colName << " |";
                }
                 std::cout << std::endl << "+";
                for (const auto& colName : headersToPrint) {
                    std::cout << std::string(colName.length() + 2, '-') << "+";
                }
                std::cout << std::endl;


                for (const auto& row : results) {
                    std::cout << "|";
                    for (const auto& val : row) {
                        std::stringstream ss;
                        ss << val;
                        std::cout << " " << std::setw(headersToPrint[(&val - &row[0])].length()) << std::left << ss.str() << " |";
                    }
                    std::cout << std::endl;
                }
                std::cout << "+";
                 for (const auto& colName : headersToPrint) {
                    std::cout << std::string(colName.length() + 2, '-') << "+";
                }
                std::cout << std::endl;
                std::cout << "► " << results.size() << " row(s) in set" << std::endl;
            }

        } else {
            std::cout << "No rows selected." << std::endl;
        }
    }

    void parseUpdate(std::istringstream& queryStream) {
        std::string tableName, setKeyword, whereKeyword, whereClause = "";
        queryStream >> tableName >> setKeyword;
        if (toLower(setKeyword) != "set") throw std::runtime_error("Expected SET after table name in UPDATE");

        std::string setClauseRaw;
        std::getline(queryStream, setClauseRaw, 'W'); // Read until 'W' of WHERE or end of line, assuming WHERE is next
        std::string setClause = trim(setClauseRaw);

        std::istringstream remainingStream(setClauseRaw.substr(setClauseRaw.find_first_of('W')));
        if (remainingStream >> whereKeyword) {
            if (toLower(whereKeyword) == "where") {
                std::string tempWhereClause;
                std::getline(queryStream, tempWhereClause);
                whereClause = trim(tempWhereClause);
            } else {
                queryStream.seekg(-whereKeyword.length(), std::ios::cur); // Rewind if WHERE not found (though should not happen now)
            }
        }


        Table& table = getTable(tableName); // Table name already extracted
        table.updateRows(setClause, whereClause); // Pass the entire SET clause
        std::cout << "Rows updated in table '" << tableName << "'" << std::endl;
    }

    void parseDelete(std::istringstream& queryStream) {
        std::string fromKeyword, tableName, whereKeyword, whereClause = "";
        queryStream >> fromKeyword >> tableName;
        if (toLower(fromKeyword) != "from") throw std::runtime_error("Expected FROM after DELETE");

        std::string nextToken;
        if (queryStream >> nextToken) {
            if (toLower(nextToken) == "where") {
                std::string tempWhereClause;
                std::getline(queryStream, tempWhereClause);
                whereClause = trim(tempWhereClause);
            } else {
                queryStream.seekg(-nextToken.length(), std::ios::cur); // Rewind if WHERE not found
            }
        }

        Table& table = getTable(tableName); // Table name already extracted
        table.deleteRows(whereClause); // Perform deletion without capturing count
        std::cout << "Rows deleted from table '" << tableName << "'" << std::endl;
    }

    void parseCreateIndex(std::istringstream& queryStream) {
        std::string indexKeyword, indexName, onKeyword, tableName, columnsPart;
        queryStream >> indexKeyword >> indexName >> onKeyword >> tableName;
        if (toLower(indexKeyword) != "create" || toLower(indexName) != "index" || toLower(onKeyword) != "on") { // Corrected keyword order and added toLower
            throw std::runtime_error("Invalid CREATE INDEX syntax.");
        }
        std::string actualIndexName;
        queryStream >> actualIndexName; // Index name is after CREATE INDEX keywords
        queryStream >> onKeyword >> tableName; // ON and then table name
        if (toLower(onKeyword) != "on") { // Check ON keyword again
            throw std::runtime_error("Invalid CREATE INDEX syntax. Expected ON keyword.");
        }

        std::getline(queryStream, columnsPart);
        columnsPart = trim(columnsPart);
        if (columnsPart.front() != '(' || columnsPart.back() != ')') {
            throw std::runtime_error("Expected column list in parentheses for CREATE INDEX.");
        }
        std::string columnName = trim(columnsPart.substr(1, columnsPart.length() - 2));

        Table& table = getTable(tableName); // Table name already extracted
        table.createIndex(columnName);
    }

};


int main() {
    Database db("HexaDB_Instance");
    std::string dbFilename = "hexadb.data";

    // Try to load database from file on startup
    try {
        db.loadDatabase(dbFilename);
    } catch (const std::runtime_error& error) {
        std::cerr << "Warning: Could not load database from file. Starting with a new database. Error: " << error.what() << std::endl;
    }


    std::cout << "Welcome to HexaDB Terminal" << std::endl;
    std::cout << "Type 'help' for commands, 'exit' to quit." << std::endl;

    std::string command;
    while (true) {
        std::cout << "HexaDB> ";
        std::getline(std::cin, command);
        std::string trimmedCommand = db.trim(command); // Trim command at the beginning

        if (trimmedCommand == "exit") {
            break;
        } else if (trimmedCommand == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  CREATE TABLE table_name (column1_name data_type, column2_name data_type, ...)" << std::endl;
            std::cout << "  INSERT INTO table_name (column1, column2, ...) VALUES (value1, value2, ...)" << std::endl;
            std::cout << "  SELECT column1, column2, ... FROM table_name [WHERE condition] or SELECT * FROM table_name [WHERE condition]" << std::endl; // Updated help for SELECT *
            std::cout << "  UPDATE table_name SET column1 = value1, column2 = value2, ... [WHERE condition]" << std::endl;
            std::cout << "  DELETE FROM table_name [WHERE condition]" << std::endl;
            std::cout << "  CREATE INDEX index_name ON table_name (column_name)" << std::endl;
            std::cout << "  PRINT TABLE table_name" << std::endl; // Keep PRINT TABLE uppercase
            std::cout << "  SAVE DB filename" << std::endl;
            std::cout << "  LOAD DB filename" << std::endl;
            std::cout << "  exit" << std::endl;
        } else if (trimmedCommand.substr(0, 11) == "PRINT TABLE ") { // Check trimmed command
            std::cout << "Debug: PRINT TABLE command detected. trimmedCommand: '" << trimmedCommand << "'" << std::endl; // Debug print
            std::string tableName = trimmedCommand.substr(12);
            tableName = db.trim(tableName);
            try {
                db.getTable(tableName).printTable();
            } catch (const std::runtime_error& error) {
                std::cerr << "Error: " << error.what() << std::endl;
            }
        } else if (trimmedCommand.substr(0, 7) == "SAVE DB") { // Check trimmed command
            std::string filename = trimmedCommand.substr(8);
            filename = db.trim(filename);
            try {
                db.saveDatabase(filename.empty() ? dbFilename : filename); // Default filename if none provided
            } catch (const std::runtime_error& error) {
                std::cerr << "Error saving database: " << error.what() << std::endl;
            }
        } else if (trimmedCommand.substr(0, 7) == "LOAD DB") { // Check trimmed command
            std::string filename = trimmedCommand.substr(8);
            filename = db.trim(filename);
            try {
                db.loadDatabase(filename.empty() ? dbFilename : filename); // Default filename if none provided
            } catch (const std::runtime_error& error) {
                std::cerr << "Error: " << error.what() << std::endl;
            }
        }
         else if (!trimmedCommand.empty()) { // Check trimmed command for empty
            try {
                db.executeQuery(trimmedCommand); // Execute trimmedCommand
            } catch (const std::runtime_error& error) {
                std::cerr << "Error: " << error.what() << std::endl;
            }
        } else {
             std::cout << "Debug: Unknown command. trimmedCommand: '" << trimmedCommand << "'" << std::endl; // Debug unknown command
        }
    }

    std::cout << "Exiting HexaDB." << std::endl;
    return 0;
}