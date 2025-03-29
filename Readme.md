# HexaDB Technical Documentation

## Core Architecture

### Database Engine
HexaDB implements a columnar in-memory storage engine with the following components:

#### Storage Layer
- **Table Structure**: Hash-based table storage with O(1) lookup
- **Column Types**: 
  - `INT`: 64-bit signed integer
  - `TEXT`: Variable-length string with UTF-8 encoding
  - `REAL`: 64-bit IEEE 754 floating-point
- **Memory Management**: Dynamic allocation with smart pointers for automatic cleanup

#### Query Engine
- **Parser**: Recursive descent parser for SQL-like syntax
- **Executor**: Pipeline-based query execution
- **Optimizer**: Rule-based optimization for WHERE clause evaluation

#### Index Management
- **Implementation**: B+ tree with configurable page size
- **Operations**: O(log n) lookup, insertion, and deletion
- **Memory Overhead**: ~20 bytes per index entry

### Natural Language Processing

#### Query Processing
- **Model**: Google Gemini API integration
- **Prompt Engineering**: Context-aware SQL generation
- **Error Handling**: Graceful degradation with fallback strategies

#### Command History
- **Storage**: LRU cache with configurable size
- **Context Window**: Last 5 queries for context enhancement

## API Reference

### SQL Interface

#### Data Definition Language (DDL)
```sql
CREATE TABLE table_name (
    column_name data_type [, ...]
);

CREATE INDEX index_name ON table_name (column_name);
```

#### Data Manipulation Language (DML)
```sql
INSERT INTO table_name [(column_list)] VALUES (value_list);

SELECT [column_list | *] 
  FROM table_name 
  [WHERE condition];

UPDATE table_name 
  SET column = value [, ...] 
  [WHERE condition];

DELETE FROM table_name 
  [WHERE condition];
```

### Natural Language Interface

```sql
NLP <natural language query>
```

Supports:
- Schema inference
- Complex joins
- Aggregation
- Nested queries

## Implementation Details

### Table Class
```cpp
class Table {
    std::vector<ColumnDefinition> columns;
    std::vector<std::vector<Value>> data;
    std::unordered_map<std::string, std::unique_ptr<Index>> indexes;
};
```

### Query Execution
1. **Parsing**: Token generation and AST construction
2. **Validation**: Schema and type checking
3. **Optimization**: Index selection and predicate pushdown
4. **Execution**: Vectorized operations where possible

### Persistence Layer
- **Format**: Custom binary format with checksums
- **Version Control**: Schema version tracking
- **Compression**: LZ4 for TEXT columns

### Configuration

#### Required Files
- `config.txt`: API keys and system parameters
- `hexadb.data`: Default persistence file

#### Build Requirements
- C++17 or later
- libcurl for HTTP requests
- 64-bit architecture

```bash
g++ -std=c++17 hexadb.cpp nlp_processor.cpp -o hexadb -lcurl
```

## Performance Characteristics

### Memory Usage
- Base: ~1KB per table
- Rows: (24 + sum(column_sizes)) bytes
- Indexes: ~20 bytes per key

### Time Complexity
- Table Scan: O(n)
- Index Lookup: O(log n)
- Insert: O(log n) with index, O(1) without
- Update: O(log n) per index
- Delete: O(log n) per index

### Concurrency
- Single-threaded execution model
- Lock-free read operations
- Write serialization for consistency

## Error Handling

### Exception Hierarchy
```cpp
std::runtime_error
  ├── DatabaseError
  │     ├── TableNotFoundError
  │     └── ColumnNotFoundError
  ├── ParserError
  └── ExecutionError
```

### Recovery Mechanisms
- Transaction rollback on error
- Automatic checkpoint recovery
- Corrupt index rebuilding

## Internal APIs

### Database Management
```cpp
Database::createTable(name, columns)
Database::dropTable(name)
Database::executeQuery(sqlQuery)
```

### NLP Processing
```cpp
NLPProcessor::executeNLQuery(query)
NLPProcessor::updateContext(history)
```

### Index Operations
```cpp
Table::createIndex(column)
Table::dropIndex(name)
Index::rebuild()
```
