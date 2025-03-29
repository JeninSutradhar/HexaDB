#ifndef HEXADB_H
#define HEXADB_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <variant>
#include <iomanip>
#include <algorithm>

// Color constants for formatted output (declarations only)
namespace Colors {
    extern const std::string RESET;
    extern const std::string RED;
    extern const std::string GREEN;
    extern const std::string YELLOW;
    extern const std::string BLUE;
    extern const std::string MAGENTA;
    extern const std::string CYAN;
    extern const std::string BOLD;
}

// Supported data types
enum DataType {
    INT,
    TEXT,
    REAL
};

std::ostream& operator<<(std::ostream& os, const DataType& dt);

// Type alias for values in a row
using Value = std::variant<int, std::string, double>;

std::ostream& operator<<(std::ostream& os, const Value& val);

struct ColumnDefinition {
    std::string name;
    DataType dataType;

    ColumnDefinition(std::string n, DataType dt);
};

class Table {
public:
    std::string name;
    std::vector<ColumnDefinition> columns;
    std::vector<std::vector<Value>> rows;
    std::map<std::string, std::map<Value, std::vector<int>>> indexes;

    Table(std::string tableName);
    void addColumn(const ColumnDefinition& colDef);
    void insertRow(const std::vector<Value>& rowValues);
    void createIndex(const std::string& columnName);
    std::vector<std::vector<Value>> selectRows(const std::vector<std::string>& selectedColumns, const std::string& whereClause = "");
    void updateRows(const std::string& setClause, const std::string& whereClause = "");
    void deleteRows(const std::string& whereClause = "");
    void printTable() const;

    int getColumnIndex(const std::string& columnName) const;
    std::string trim(const std::string& str) const;

private:
    std::string dataTypeToString(DataType dt) const;
    std::string toLower(std::string str) const;
};

class Database {
public:
    std::string name;
    std::map<std::string, Table> tables;

    Database(std::string dbName);
    void createTable(const std::string& tableName, const std::vector<ColumnDefinition>& colDefs);
    Table& getTable(const std::string& tableName);
    void executeQuery(const std::string& sqlQuery);
    void saveDatabase(const std::string& filename) const;
    void loadDatabase(const std::string& filename);
    std::string trim(const std::string& str) const;
    std::string toLower(std::string str) const;

private:
    void parseCreateTable(std::istringstream& queryStream);
    void parseInsert(std::istringstream& queryStream);
    void parseSelect(std::istringstream& queryStream);
    void parseUpdate(std::istringstream& queryStream);
    void parseDelete(std::istringstream& queryStream);
    void parseCreateIndex(std::istringstream& queryStream);
};

#endif // HEXADB_H