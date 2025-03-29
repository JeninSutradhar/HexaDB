// j.sutradhar@symbola.io
#include "hexadb.h"

// Color constants definitions
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

std::ostream& operator<<(std::ostream& os, const DataType& dt) {
    switch (dt) {
        case INT: os << "INT"; break;
        case TEXT: os << "TEXT"; break;
        case REAL: os << "REAL"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Value& val) {
    std::visit([&](const auto& v){ os << v; }, val);
    return os;
}

ColumnDefinition::ColumnDefinition(std::string n, DataType dt) : name(n), dataType(dt) {}

Table::Table(std::string tableName) : name(tableName) {}

void Table::addColumn(const ColumnDefinition& colDef) {
    columns.push_back(colDef);
}

void Table::insertRow(const std::vector<Value>& rowValues) {
    if (rowValues.size() != columns.size()) {
        throw std::runtime_error("Number of values (" + std::to_string(rowValues.size()) + 
                                 ") doesn't match column count (" + std::to_string(columns.size()) + ").");
    }
    rows.push_back(rowValues);
    int rowIndex = rows.size() - 1;

    for (const auto& indexPair : indexes) {
        const std::string& colName = indexPair.first;
        int colIndex = getColumnIndex(colName);
        if (colIndex != -1) {
            indexes[colName][rowValues[colIndex]].push_back(rowIndex);
        }
    }
}

void Table::createIndex(const std::string& columnName) {
    int colIndex = getColumnIndex(columnName);
    if (colIndex == -1) {
        throw std::runtime_error("Column '" + columnName + "' not found for index creation in table '" + name + "'.");
    }
    if (indexes.count(columnName)) {
        return; // Index already exists
    }

    indexes[columnName] = {};
    for (int i = 0; i < rows.size(); ++i) {
        indexes[columnName][rows[i][colIndex]].push_back(i);
    }
    std::cout << "Index created on column '" << columnName << "' for table '" << name << "'" << std::endl;
}

std::vector<std::vector<Value>> Table::selectRows(const std::vector<std::string>& selectedColumns, const std::string& whereClause) {
    std::vector<std::vector<Value>> resultRows;
    std::vector<int> selectedColIndices;
    for (const auto& colName : selectedColumns) {
        int index = getColumnIndex(colName);
        if (index == -1) {
            throw std::runtime_error("Column '" + colName + "' not found in table '" + name + "'.");
        }
        selectedColIndices.push_back(index);
    }

    std::string whereColName, whereOp, whereValueStr;
    Value whereValue;
    bool hasWhereClause = !whereClause.empty();

    if (hasWhereClause) {
        std::istringstream whereStream(whereClause);
        whereStream >> whereColName >> whereOp >> whereValueStr;
        int whereColIndex = getColumnIndex(whereColName);
        if (whereColIndex == -1) {
            throw std::runtime_error("Column '" + whereColName + "' in WHERE clause not found in table '" + name + "'.");
        }
        DataType whereColType = columns[whereColIndex].dataType;
        try {
            if (whereColType == INT) {
                whereValue = std::stoi(whereValueStr);
            } else if (whereColType == REAL) {
                whereValue = std::stod(whereValueStr);
            } else { // TEXT
                whereValue = whereValueStr;
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid value '" + whereValueStr + "' in WHERE clause: " + e.what());
        }
        if (whereOp == "==") whereOp = "=";
    }

    for (int i = 0; i < rows.size(); ++i) {
        bool conditionMet = !hasWhereClause;
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
            } else {
                throw std::runtime_error("Unsupported operator '" + whereOp + "' or incompatible types in WHERE clause.");
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

void Table::updateRows(const std::string& setClause, const std::string& whereClause) {
    std::istringstream setStream(setClause);
    std::string updatePair;
    while (std::getline(setStream, updatePair, ',')) {
        std::istringstream pairStream(trim(updatePair));
        std::string setColName, setValueStr;
        std::getline(pairStream, setColName, '=');
        std::getline(pairStream, setValueStr);

        setColName = trim(setColName);
        setValueStr = trim(setValueStr);
        int setColIndex = getColumnIndex(setColName);
        if (setColIndex == -1) {
            throw std::runtime_error("Column '" + setColName + "' in SET clause not found in table '" + name + "'.");
        }
        DataType setColType = columns[setColIndex].dataType;
        Value setValue;
        try {
            if (setColType == INT) {
                setValue = std::stoi(setValueStr);
            } else if (setColType == REAL) {
                setValue = std::stod(setValueStr);
            } else { // TEXT
                setValue = setValueStr;
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid value '" + setValueStr + "' in SET clause: " + e.what());
        }

        std::string whereColName, whereOp, whereValueStr;
        Value whereValue;
        bool hasWhereClause = !whereClause.empty();
        if (hasWhereClause) {
            std::istringstream whereStream(whereClause);
            whereStream >> whereColName >> whereOp >> whereValueStr;
            int whereColIndex = getColumnIndex(whereColName);
            if (whereColIndex == -1) {
                throw std::runtime_error("Column '" + whereColName + "' in WHERE clause not found in table '" + name + "'.");
            }
            DataType whereColType = columns[whereColIndex].dataType;
            try {
                if (whereColType == INT) {
                    whereValue = std::stoi(whereValueStr);
                } else if (whereColType == REAL) {
                    whereValue = std::stod(whereValueStr);
                } else { // TEXT
                    whereValue = whereValueStr;
                }
            } catch (const std::exception& e) {
                throw std::runtime_error("Invalid value '" + whereValueStr + "' in WHERE clause: " + e.what());
            }
            if (whereOp == "==") whereOp = "=";
        }

        for (int i = 0; i < rows.size(); ++i) {
            bool conditionMet = !hasWhereClause;
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
                } else {
                    throw std::runtime_error("Unsupported operator '" + whereOp + "' or incompatible types in WHERE clause.");
                }
            }

            if (conditionMet) {
                rows[i][setColIndex] = setValue;
                if (indexes.count(setColName)) {
                    indexes[setColName].clear();
                    for (int j = 0; j < rows.size(); ++j) {
                        indexes[setColName][rows[j][setColIndex]].push_back(j);
                    }
                }
            }
        }
    }
}

void Table::deleteRows(const std::string& whereClause) {
    std::string whereColName, whereOp, whereValueStr;
    Value whereValue;
    bool hasWhereClause = !whereClause.empty();

    if (hasWhereClause) {
        std::istringstream whereStream(whereClause);
        whereStream >> whereColName >> whereOp >> whereValueStr;
        int whereColIndex = getColumnIndex(whereColName);
        if (whereColIndex == -1) {
            throw std::runtime_error("Column '" + whereColName + "' in WHERE clause not found in table '" + name + "'.");
        }
        DataType whereColType = columns[whereColIndex].dataType;
        try {
            if (whereColType == INT) {
                whereValue = std::stoi(whereValueStr);
            } else if (whereColType == REAL) {
                whereValue = std::stod(whereValueStr);
            } else { // TEXT
                whereValue = whereValueStr;
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid value '" + whereValueStr + "' in WHERE clause: " + e.what());
        }
        if (whereOp == "==") whereOp = "=";
    }

    std::vector<std::vector<Value>> newRows;
    for (int i = 0; i < rows.size(); ++i) {
        bool conditionMet = !hasWhereClause;
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
            } else {
                throw std::runtime_error("Unsupported operator '" + whereOp + "' or incompatible types in WHERE clause.");
            }
        }

        if (!conditionMet) {
            newRows.push_back(rows[i]);
        }
    }
    rows = newRows;

    for (auto const& [colName, indexMap] : indexes) {
        createIndex(colName); // Rebuild indexes
    }
}

void Table::printTable() const {
    std::vector<size_t> columnWidths;
    for (const auto& col : columns) {
        std::string headerText = col.name + "(" + dataTypeToString(col.dataType) + ")";
        columnWidths.push_back(headerText.length());
    }

    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            std::stringstream ss;
            ss << row[i];
            columnWidths[i] = std::max(columnWidths[i], ss.str().length());
        }
    }

    // Print table header with name
    std::cout << Colors::BOLD << Colors::CYAN << "┌─ Table: " << name << Colors::RESET << std::endl;

    // Print top border
    std::cout << Colors::BLUE << "┌";
    for (size_t i = 0; i < columns.size(); ++i) {
        std::string line;
        for (size_t j = 0; j < columnWidths[i] + 2; j++) line += "─";
        std::cout << line << (i < columns.size() - 1 ? "┬" : "┐");
    }
    std::cout << Colors::RESET << "\n";

    // Print column headers
    std::cout << Colors::BOLD << Colors::BLUE << "│ ";
    for (size_t i = 0; i < columns.size(); ++i) {
        std::string headerText = columns[i].name + "(" + dataTypeToString(columns[i].dataType) + ")";
        std::cout << std::setw(columnWidths[i]) << std::left << headerText << (i < columns.size() - 1 ? " │ " : " │");
    }
    std::cout << Colors::RESET << "\n";

    // Print header separator
    std::cout << Colors::BLUE << "├";
    for (size_t i = 0; i < columns.size(); ++i) {
        std::string line;
        for (size_t j = 0; j < columnWidths[i] + 2; j++) line += "─";
        std::cout << line << (i < columns.size() - 1 ? "┼" : "┤");
    }
    std::cout << Colors::RESET << "\n";

    // Print rows
    for (const auto& row : rows) {
        std::cout << Colors::CYAN << "│ ";
        for (size_t i = 0; i < row.size(); ++i) {
            std::stringstream ss;
            ss << row[i];
            std::cout << std::setw(columnWidths[i]) << std::left << ss.str() << (i < row.size() - 1 ? " │ " : " │");
        }
        std::cout << Colors::RESET << "\n";
    }

    // Print bottom border
    std::cout << Colors::BLUE << "└";
    for (size_t i = 0; i < columns.size(); ++i) {
        std::string line;
        for (size_t j = 0; j < columnWidths[i] + 2; j++) line += "─";
        std::cout << line << (i < columns.size() - 1 ? "┴" : "┘");
    }
    std::cout << Colors::RESET << "\n";
    std::cout << Colors::GREEN << "► " << rows.size() << " row(s) in set" << Colors::RESET << "\n\n";
}

std::string Table::dataTypeToString(DataType dt) const {
    switch (dt) {
        case INT: return "INT";
        case TEXT: return "TEXT";
        case REAL: return "REAL";
        default: return "UNKNOWN";
    }
}

std::string Table::toLower(std::string str) const {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

int Table::getColumnIndex(const std::string& columnName) const {
    std::string lowerColName = toLower(columnName);
    for (int i = 0; i < columns.size(); ++i) {
        if (toLower(columns[i].name) == lowerColName) {
            return i;
        }
    }
    return -1;
}

std::string Table::trim(const std::string& str) const {
    const std::string whitespace = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, last - first + 1);
}

Database::Database(std::string dbName) : name(dbName) {}

void Database::createTable(const std::string& tableName, const std::vector<ColumnDefinition>& colDefs) {
    std::string lowerTableName = toLower(tableName);
    if (tables.count(lowerTableName)) {
        throw std::runtime_error("Table '" + tableName + "' already exists.");
    }
    auto [it, inserted] = tables.emplace(lowerTableName, Table(tableName));
    if (!inserted) {
        throw std::runtime_error("Failed to create table '" + tableName + "'.");
    }
    Table& newTable = it->second;
    for (const auto& colDef : colDefs) {
        newTable.addColumn(colDef);
    }
    std::cout << "Table '" << tableName << "' created." << std::endl;
}

Table& Database::getTable(const std::string& tableName) {
    std::string lowerTableName = toLower(tableName);
    if (!tables.count(lowerTableName)) {
        throw std::runtime_error("Table '" + tableName + "' not found.");
    }
    return tables.at(lowerTableName);
}

void Database::executeQuery(const std::string& sqlQuery) {
    std::istringstream queryStream(sqlQuery);
    std::string command;
    queryStream >> command;

    if (toLower(command) == "create") {
        std::string type;
        queryStream >> type;
        if (toLower(type) == "table") {
            parseCreateTable(queryStream);
        } else if (toLower(type) == "index") {
            parseCreateIndex(queryStream);
        } else {
            throw std::runtime_error("Unsupported CREATE type: " + type);
        }
    } else if (toLower(command) == "insert") {
        parseInsert(queryStream);
    } else if (toLower(command) == "select") {
        parseSelect(queryStream);
    } else if (toLower(command) == "update") {
        parseUpdate(queryStream);
    } else if (toLower(command) == "delete") {
        parseDelete(queryStream);
    } else if (toLower(command) == "print") {
        std::string type;
        queryStream >> type;
        if (toLower(type) == "table") {
            std::string tableName;
            queryStream >> tableName;
            getTable(tableName).printTable();
        } else {
            throw std::runtime_error("Unsupported PRINT type: " + type);
        }
    } else {
        throw std::runtime_error("Unknown command: " + command);
    }
}

void Database::saveDatabase(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file '" + filename + "' for saving database.");
    }

    outFile << "DATABASE_NAME " << name << std::endl;
    outFile << "TABLE_COUNT " << tables.size() << std::endl;

    for (const auto& [tableName, table] : tables) {
        outFile << "TABLE_NAME " << table.name << std::endl;
        outFile << "COLUMN_COUNT " << table.columns.size() << std::endl;

        for (const auto& col : table.columns) {
            outFile << "COLUMN " << col.name << " " << static_cast<int>(col.dataType) << std::endl;
        }

        outFile << "ROW_COUNT " << table.rows.size() << std::endl;

        for (const auto& row : table.rows) {
            outFile << "ROW";
            for (size_t i = 0; i < row.size(); ++i) {
                const auto& value = row[i];
                if (i > 0) outFile << " ";
                if (std::holds_alternative<int>(value)) {
                    outFile << "I " << std::get<int>(value);
                } else if (std::holds_alternative<std::string>(value)) {
                    outFile << "T \"" << std::get<std::string>(value) << "\"";
                } else if (std::holds_alternative<double>(value)) {
                    outFile << "R " << std::get<double>(value);
                }
            }
            outFile << std::endl;
        }
    }

    outFile.close();
    std::cout << "Database '" << name << "' saved to '" << filename << "'" << std::endl;
}

void Database::loadDatabase(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        throw std::runtime_error("Database file not found: " + filename);
    }

    std::string line, key;
    std::map<std::string, Table> newTables;

    auto splitRow = [](const std::string& line) {
        std::vector<std::string> tokens;
        std::istringstream stream(line);
        std::string token;
        while (stream >> std::ws) {
            if (stream.peek() == '"') {
                stream.ignore(1);
                std::string quotedToken;
                std::getline(stream, quotedToken, '"');
                tokens.push_back(quotedToken);
            } else {
                stream >> token;
                tokens.push_back(token);
            }
        }
        return tokens;
    };

    std::getline(inFile, line);
    std::istringstream dbStream(line);
    dbStream >> key >> name;

    std::getline(inFile, line);
    std::istringstream countStream(line);
    int tableCount;
    countStream >> key >> tableCount;

    for (int t = 0; t < tableCount; t++) {
        std::getline(inFile, line);
        std::istringstream tableStream(line);
        std::string tableName;
        tableStream >> key >> tableName;

        std::getline(inFile, line);
        std::istringstream colCountStream(line);
        int columnCount;
        colCountStream >> key >> columnCount;

        std::vector<ColumnDefinition> colDefs;
        for (int c = 0; c < columnCount; c++) {
            std::getline(inFile, line);
            std::istringstream colStream(line);
            std::string colName;
            int dataTypeInt;
            colStream >> key >> colName >> dataTypeInt;
            colDefs.emplace_back(colName, static_cast<DataType>(dataTypeInt));
        }

        auto [it, inserted] = newTables.emplace(toLower(tableName), Table(tableName));
        if (!inserted) {
            throw std::runtime_error("Failed to create table '" + tableName + "' during load.");
        }
        Table& table = it->second;
        for (const auto& colDef : colDefs) {
            table.addColumn(colDef);
        }
        std::cout << "Table '" << tableName << "' created." << std::endl;

        std::getline(inFile, line);
        std::istringstream rowCountStream(line);
        int rowCount;
        rowCountStream >> key >> rowCount;

        for (int r = 0; r < rowCount; r++) {
            std::getline(inFile, line);
            std::vector<std::string> tokens = splitRow(line);
            if (tokens.empty() || tokens[0] != "ROW") {
                throw std::runtime_error("Expected 'ROW' at the start of row data: " + line);
            }

            std::vector<Value> rowValues;
            int tokenIndex = 1;
            for (int c = 0; c < columnCount; c++) {
                if (tokenIndex >= tokens.size()) {
                    throw std::runtime_error("Insufficient tokens in row data: " + line);
                }
                std::string typeStr = tokens[tokenIndex++];
                if (typeStr == "I") {
                    if (tokenIndex >= tokens.size()) {
                        throw std::runtime_error("Missing value for type 'I' in row data: " + line);
                    }
                    int val = std::stoi(tokens[tokenIndex++]);
                    rowValues.push_back(val);
                } else if (typeStr == "T") {
                    if (tokenIndex >= tokens.size()) {
                        throw std::runtime_error("Missing value for type 'T' in row data: " + line);
                    }
                    std::string val = tokens[tokenIndex++];
                    rowValues.push_back(val);
                } else if (typeStr == "R") {
                    if (tokenIndex >= tokens.size()) {
                        throw std::runtime_error("Missing value for type 'R' in row data: " + line);
                    }
                    double val = std::stod(tokens[tokenIndex++]);
                    rowValues.push_back(val);
                } else {
                    throw std::runtime_error("Invalid value type '" + typeStr + "' in row data: " + line);
                }
            }
            table.insertRow(rowValues);
        }
    }

    inFile.close();
    tables = std::move(newTables);
    std::cout << "Database '" << name << "' loaded from '" << filename << "'" << std::endl;
}

std::string Database::trim(const std::string& str) const {
    const std::string whitespace = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, last - first + 1);
}

std::string Database::toLower(std::string str) const {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

void Database::parseCreateTable(std::istringstream& queryStream) {
    std::string tableName;
    queryStream >> tableName;
    std::string columnsPart;
    std::getline(queryStream, columnsPart);
    columnsPart = trim(columnsPart);

    if (columnsPart.front() != '(' || columnsPart.back() != ')') {
        throw std::runtime_error("Invalid column definition format in CREATE TABLE '" + tableName + 
                                 "'. Expected: (column_name type, ...)");
    }
    columnsPart = columnsPart.substr(1, columnsPart.length() - 2);

    std::vector<ColumnDefinition> colDefs;
    std::istringstream colStream(columnsPart);
    std::string colDefStr;

    while (std::getline(colStream, colDefStr, ',')) {
        std::istringstream singleColStream(trim(colDefStr));
        std::string colName, dataTypeStr;
        singleColStream >> colName >> dataTypeStr;
        colName = trim(colName);
        dataTypeStr = trim(dataTypeStr);

        DataType dataType;
        if (toLower(dataTypeStr) == "int") dataType = INT;
        else if (toLower(dataTypeStr) == "text") dataType = TEXT;
        else if (toLower(dataTypeStr) == "real") dataType = REAL;
        else throw std::runtime_error("Unknown data type '" + dataTypeStr + "' in table '" + tableName + "'");

        colDefs.emplace_back(colName, dataType);
    }
    createTable(tableName, colDefs);
}

void Database::parseInsert(std::istringstream& queryStream) {
    std::string intoKeyword, tableName, columnsPart, valuesKeyword, valuesPart;
    queryStream >> intoKeyword >> tableName;
    if (toLower(intoKeyword) != "into") throw std::runtime_error("Expected INTO after INSERT");

    std::getline(queryStream, columnsPart, '(');
    std::getline(queryStream, columnsPart, ')');
    columnsPart = trim(columnsPart);

    queryStream >> valuesKeyword;
    if (toLower(valuesKeyword) != "values") throw std::runtime_error("Expected VALUES after column list");

    std::getline(queryStream, valuesPart, '(');
    std::getline(queryStream, valuesPart, ')');
    valuesPart = trim(valuesPart);

    Table& table = getTable(tableName);
    std::vector<std::string> columnNames;
    std::istringstream colStream(columnsPart);
    std::string colName;
    while (std::getline(colStream, colName, ',')) {
        columnNames.push_back(trim(colName));
    }

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
                if (valueStr.front() == '\'' && valueStr.back() == '\'') {
                    rowValues.push_back(valueStr.substr(1, valueStr.length() - 2));
                } else {
                    rowValues.push_back(valueStr);
                }
            } else if (colType == INT) {
                rowValues.push_back(std::stoi(valueStr));
            } else if (colType == REAL) {
                rowValues.push_back(std::stod(valueStr));
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Error converting value '" + valueStr + "' for column '" + colName + "': " + e.what());
        }
        colIndex++;
    }

    table.insertRow(rowValues);
    std::cout << "Row inserted into table '" << tableName << "'" << std::endl;
}

void Database::parseSelect(std::istringstream& queryStream) {
    std::vector<std::string> selectColumns;
    std::string columnName;
    while (queryStream >> columnName && toLower(columnName) != "from") {
        if (columnName == "*") {
            selectColumns.push_back("*");
            if (queryStream.peek() == ',') queryStream.ignore();
            break;
        } else {
            selectColumns.push_back(trim(columnName));
            if (queryStream.peek() == ',') queryStream.ignore();
        }
    }

    std::string fromKeyword, tableName;
    if (!(queryStream >> fromKeyword) || toLower(fromKeyword) != "from") {
        throw std::runtime_error("Expected FROM keyword after column list in SELECT query");
    }
    if (!(queryStream >> tableName)) {
        throw std::runtime_error("Expected table name after FROM in SELECT query");
    }
    tableName = trim(tableName);

    std::string whereClause;
    std::string whereKeyword;
    if (queryStream >> whereKeyword && toLower(whereKeyword) == "where") {
        std::getline(queryStream, whereClause);
        whereClause = trim(whereClause);
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

    if (!results.empty()) {
        std::vector<std::string> headersToPrint = (selectColumns.size() == 1 && selectColumns[0] == "*") ?
                                                  std::vector<std::string>() : selectColumns;
        if (headersToPrint.empty()) {
            table.printTable();
        } else {
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

void Database::parseUpdate(std::istringstream& queryStream) {
    std::string tableName, setKeyword;
    queryStream >> tableName >> setKeyword;
    if (toLower(setKeyword) != "set") throw std::runtime_error("Expected SET after table name in UPDATE");

    std::string setClause;
    std::getline(queryStream, setClause);
    size_t wherePos = setClause.find("WHERE");
    std::string whereClause = "";
    if (wherePos != std::string::npos) {
        whereClause = trim(setClause.substr(wherePos + 5));
        setClause = trim(setClause.substr(0, wherePos));
    } else {
        setClause = trim(setClause);
    }

    Table& table = getTable(tableName);
    table.updateRows(setClause, whereClause);
    std::cout << "Rows updated in table '" << tableName << "'" << std::endl;
}

void Database::parseDelete(std::istringstream& queryStream) {
    std::string fromKeyword, tableName;
    queryStream >> fromKeyword >> tableName;
    if (toLower(fromKeyword) != "from") throw std::runtime_error("Expected FROM after DELETE");

    std::string whereClause;
    std::string whereKeyword;
    if (queryStream >> whereKeyword && toLower(whereKeyword) == "where") {
        std::getline(queryStream, whereClause);
        whereClause = trim(whereClause);
    }

    Table& table = getTable(tableName);
    table.deleteRows(whereClause);
    std::cout << "Rows deleted from table '" << tableName << "'" << std::endl;
}

void Database::parseCreateIndex(std::istringstream& queryStream) {
    std::string indexName, onKeyword, tableName, columnsPart;
    queryStream >> indexName >> onKeyword >> tableName;
    if (toLower(onKeyword) != "on") {
        throw std::runtime_error("Invalid CREATE INDEX syntax. Expected ON after index name.");
    }

    std::getline(queryStream, columnsPart);
    columnsPart = trim(columnsPart);
    if (columnsPart.front() != '(' || columnsPart.back() != ')') {
        throw std::runtime_error("Expected column list in parentheses for CREATE INDEX on '" + tableName + "'.");
    }
    std::string columnName = trim(columnsPart.substr(1, columnsPart.length() - 2));

    Table& table = getTable(tableName);
    table.createIndex(columnName);
}

// Include NLP processor and config reader after all HexaDB definitions
#include "nlp_processor.h"
#include "config_reader.h"

int main() {
    Database db("HexaDB_Instance");
    std::string dbFilename = "hexadb.data";
    std::string apiKey;
    try {
        apiKey = ConfigReader::getApiKey();
    } catch (const std::runtime_error& error) {
        std::cerr << Colors::RED << "Error: " << error.what() << Colors::RESET << std::endl;
        return 1;
    }
    NLPProcessor nlp(apiKey, db);

    try {
        db.loadDatabase(dbFilename);
    } catch (const std::runtime_error& error) {
        std::cerr << "Warning: Could not load database from file. Starting with a new database. Error: " << error.what() << std::endl;
    }

    std::cout << "Welcome to HexaDB Terminal" << std::endl;
    std::cout << "Type 'help' for commands, 'exit' to quit, or 'NLP <query>' for natural language queries." << std::endl;

    std::string command;
    while (true) {
        std::cout << "HexaDB> ";
        std::getline(std::cin, command);
        std::string trimmedCommand = db.trim(command);

        if (trimmedCommand == "exit") {
            break;
        } else if (trimmedCommand == "help") {
            std::cout << Colors::BOLD << Colors::CYAN << "\nHexaDB Command Reference" << Colors::RESET << std::endl;
            std::cout << Colors::CYAN << "═══════════════════════" << Colors::RESET << "\n" << std::endl;

            // Natural Language Section
            std::cout << Colors::BOLD << "Natural Language:" << Colors::RESET << std::endl;
            std::cout << Colors::GREEN << "  NLP" << Colors::RESET << " <query> - Execute natural language queries" << std::endl;
            std::cout << "  Example: 'NLP show all products where price > 100'\n" << std::endl;

            // Table Operations
            std::cout << Colors::BOLD << "Table Operations:" << Colors::RESET << std::endl;
            std::cout << Colors::BLUE << "  CREATE TABLE" << Colors::RESET << " table_name (column1_name data_type, ...)" << std::endl;
            std::cout << Colors::BLUE << "  INSERT INTO" << Colors::RESET << " table_name (columns) VALUES (values)" << std::endl;
            std::cout << Colors::BLUE << "  SELECT" << Colors::RESET << " columns FROM table_name [WHERE condition]" << std::endl;
            std::cout << Colors::BLUE << "  UPDATE" << Colors::RESET << " table_name SET column = value [WHERE condition]" << std::endl;
            std::cout << Colors::BLUE << "  DELETE FROM" << Colors::RESET << " table_name [WHERE condition]\n" << std::endl;

            // Database Management
            std::cout << Colors::BOLD << "Database Management:" << Colors::RESET << std::endl;
            std::cout << Colors::MAGENTA << "  CREATE INDEX" << Colors::RESET << " index_name ON table_name (column)" << std::endl;
            std::cout << Colors::MAGENTA << "  PRINT TABLE" << Colors::RESET << " table_name" << std::endl;
            std::cout << Colors::MAGENTA << "  SAVE DB" << Colors::RESET << " [filename] - Save database to file" << std::endl;
            std::cout << Colors::MAGENTA << "  LOAD DB" << Colors::RESET << " [filename] - Load database from file\n" << std::endl;

            // Data Types
            std::cout << Colors::BOLD << "Supported Data Types:" << Colors::RESET << std::endl;
            std::cout << "  INT   - Integer values" << std::endl;
            std::cout << "  TEXT  - Text strings" << std::endl;
            std::cout << "  REAL  - Decimal numbers\n" << std::endl;

            std::cout << Colors::BOLD << "Other Commands:" << Colors::RESET << std::endl;
            std::cout << "  exit  - Exit HexaDB\n" << std::endl;
        } else if (trimmedCommand.substr(0, 4) == "NLP ") {
            std::string nlQuery = db.trim(trimmedCommand.substr(4));
            nlp.executeNLQuery(nlQuery);
        } else if (trimmedCommand.substr(0, 11) == "PRINT TABLE") {
            std::string tableName = db.trim(trimmedCommand.substr(11));
            try {
                db.getTable(tableName).printTable();
            } catch (const std::runtime_error& error) {
                std::cerr << "Error: " << error.what() << std::endl;
            }
        } else if (trimmedCommand.substr(0, 7) == "SAVE DB") {
            std::string filename = db.trim(trimmedCommand.substr(7));
            try {
                db.saveDatabase(filename.empty() ? dbFilename : filename);
            } catch (const std::runtime_error& error) {
                std::cerr << "Error saving database: " << error.what() << std::endl;
            }
        } else if (trimmedCommand.substr(0, 7) == "LOAD DB") {
            std::string filename = db.trim(trimmedCommand.substr(7));
            try {
                db.loadDatabase(filename.empty() ? dbFilename : filename);
            } catch (const std::runtime_error& error) {
                std::cerr << "Error loading database: " << error.what() << std::endl;
                std::cerr << "Database state may be inconsistent. Consider restarting or loading a valid file." << std::endl;
            }
        } else if (!trimmedCommand.empty()) {
            try {
                db.executeQuery(trimmedCommand);
            } catch (const std::runtime_error& error) {
                std::cerr << "Error: " << error.what() << std::endl;
            }
        }
    }

    std::cout << "Exiting HexaDB." << std::endl;
    return 0;
}