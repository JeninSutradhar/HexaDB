<p align="center">
  <img src="https://github.com/user-attachments/assets/75d0f6fe-07c9-4f9b-a5fc-3cafea265563" alt="Screenshot_20241228_102517" width=200>
</p>

# HexaDB 
An AI-powered in-memory relational database system that supports SQL-like operations, enhanced with AI-based natural language processing

![diagram-export-3-29-2025-12_56_06-PM](https://github.com/user-attachments/assets/f1bc336b-d20e-407a-96d4-5ee4e3076cf8)

**Version:** 1.0 (Hypothetical)
**Author:** j.sutradhar@symbola.io 
## Table of Contents

1.  [Introduction](#1-introduction)
    *   [What is HexaDB?](#11-what-is-hexadb)
    *   [Key Features](#12-key-features)
    *   [Intended Use Cases](#13-intended-use-cases)
2.  [Getting Started](#2-getting-started)
    *   [Prerequisites](#21-prerequisites)
    *   [Configuration](#22-configuration)
    *   [Compilation](#23-compilation)
    *   [Running HexaDB](#24-running-hexadb)
3.  [Architecture](#3-architecture)
    *   [Core Database Engine](#31-core-database-engine)
    *   [NLP Processor](#32-nlp-processor)
    *   [Configuration Reader](#33-configuration-reader)
    *   [Command Line Interface (CLI)](#34-command-line-interface-cli)
    *   [Data Flow (NLP Query)](#35-data-flow-nlp-query)
4.  [Command Line Interface (CLI)](#4-command-line-interface-cli)
    *   [Starting the CLI](#41-starting-the-cli)
    *   [Command Structure](#42-command-structure)
    *   [Available Commands](#43-available-commands)
        *   [SQL Data Definition Language (DDL)](#431-sql-data-definition-language-ddl)
        *   [SQL Data Manipulation Language (DML)](#432-sql-data-manipulation-language-dml)
        *   [Database Management](#433-database-management)
        *   [Natural Language Processing (NLP)](#434-natural-language-processing-nlp)
        *   [Utility Commands](#435-utility-commands)
5.  [SQL Reference](#5-sql-reference)
    *   [Overview](#51-overview)
    *   [Data Types](#52-data-types)
    *   [SQL Commands](#53-sql-commands)
        *   [CREATE TABLE](#531-create-table)
        *   [INSERT INTO](#532-insert-into)
        *   [SELECT](#533-select)
        *   [UPDATE](#534-update)
        *   [DELETE FROM](#535-delete-from)
        *   [CREATE INDEX](#536-create-index)
    *   [WHERE Clause](#54-where-clause)
    *   [Literals](#55-literals)
    *   [Identifiers](#56-identifiers)
6.  [Natural Language Processing (NLP)](#6-natural-language-processing-nlp)
    *   [Overview](#61-overview)
    *   [Usage](#62-usage)
    *   [How it Works](#63-how-it-works)
    *   [Examples](#64-examples)
    *   [Tips for Effective Queries](#65-tips-for-effective-queries)
    *   [Output Format](#66-output-format)
    *   [Reliability](#67-reliability)
7.  [Persistence](#7-persistence)
    *   [Overview](#71-overview)
    *   [SAVE DB Command](#72-save-db-command)
    *   [LOAD DB Command](#73-load-db-command)
    *   [File Format (`hexadb.data`)](#74-file-format-hexadbdata)
    *   [Considerations](#75-considerations)
8.  [Indexing](#8-indexing)
    *   [Overview](#81-overview)
    *   [CREATE INDEX Command](#82-create-index-command)
    *   [Usage and Limitations](#83-usage-and-limitations)
9.  [Configuration (`config.txt`)](#9-configuration-configtxt)
    *   [Location and Format](#91-location-and-format)
    *   [GEMINI_API_KEY](#92-gemini_api_key)
    *   [Security Considerations](#93-security-considerations)
10. [API Integration (Google Gemini)](#10-api-integration-google-gemini)
    *   [Purpose](#101-purpose)
    *   [Mechanism](#102-mechanism)
    *   [Data Sent](#103-data-sent)
    *   [Dependencies](#104-dependencies)
11. [Limitations and Considerations](#11-limitations-and-considerations)
12. [Troubleshooting](#12-troubleshooting)

---

## 1. Introduction

### 1.1 What is HexaDB?

HexaDB is a lightweight, in-memory relational database system implemented in C++. It provides basic SQL-like functionality for creating tables, inserting, querying, updating, and deleting data. Its distinguishing feature is the integration with Google's Gemini Large Language Model (LLM) to allow users to interact with the database using natural language queries, which are then translated into SQL.
![diagram-export-3-29-2025-12_37_11-PM](https://github.com/user-attachments/assets/acbdebcd-8f80-44b9-be68-10f4c5060271)

### 1.2 Key Features

*   **In-Memory Storage:** All data resides in system memory for fast access (but is lost when the application closes unless saved).
*   **SQL Support:** Implements core SQL commands (`CREATE TABLE`, `INSERT`, `SELECT`, `UPDATE`, `DELETE`).
*   **Natural Language Processing (NLP):** Converts natural language queries into executable SQL using the Gemini API.
*   **Persistence:** Ability to save the current database state to a file and load it back later.
*   **Basic Data Types:** Supports `INT`, `TEXT`, and `REAL` data types.

### 1.3 Intended Use Cases

*   Educational purposes for learning about database internals and C++ implementation.
*   Demonstrating NLP integration with database systems.
*   Prototyping simple applications where persistence and complex queries are not primary requirements.
*   Personal projects requiring a very simple, temporary data store.

---

## 2. Getting Started

### 2.1 Prerequisites

*   **C++ Compiler:** A modern C++ compiler supporting C++17 features (like `std::variant`). GCC (g++) or Clang is recommended.
*   **libcurl:** The cURL library for making HTTP requests to the Gemini API.
    *   On Debian/Ubuntu: `sudo apt-get update && sudo apt-get install libcurl4-openssl-dev`
    *   On Fedora: `sudo dnf install libcurl-devel`
    *   On macOS (with Homebrew): `brew install curl`
*   **nlohmann/json:** A JSON library for C++. You need to download the `json.hpp` header file and place it where your compiler can find it (e.g., in the same directory or an include path). Get it from [https://github.com/nlohmann/json](https://github.com/nlohmann/json).
*   **Google Gemini API Key:** Required for the NLP feature. Obtain one from Google AI Studio or Google Cloud Console.

### 2.2 Configuration

1.  Create a file named `config.txt` in the same directory where you will run the HexaDB executable.
2.  Add the following line to `config.txt`, replacing `your_api_key_here` with your actual Gemini API Key:
    ```
    GEMINI_API_KEY=your_api_key_here
    ```
3.  **Important:** Protect this file appropriately, as it contains your sensitive API key. Do not commit it to public repositories.

### 2.3 Compilation

Compile the source files using your C++ compiler, linking against `libcurl`. Assuming all `.cpp` files (`hexadb.cpp`, `nlp_processor.cpp`), header files (`.h`), and `json.hpp` are in the same directory:

```bash
g++ -std=c++17 hexadb.cpp nlp_processor.cpp -o hexadb -lcurl
```

*   `-o hexadb`: Specifies the output executable name.
*   `-std=c++17`: Enables C++17 features.
*   `-lcurl`: Links the cURL library.
*   `-I.`: Tells the compiler to look for headers (like `json.hpp` and the project's `.h` files) in the current directory.

### 2.4 Running HexaDB

Execute the compiled program from your terminal:

```bash
./hexadb
```

You should see the welcome message and the `HexaDB>` prompt.

---

## 3. Architecture
![architecture](https://github.com/user-attachments/assets/4af3d5b2-532f-4edd-a84d-507c153bb954)

HexaDB consists of several key components working together:

### 3.1 Core Database Engine (`Database`, `Table` classes)

*   Manages the collection of tables (`std::map<std::string, Table>`).
*   The `Table` class stores column definitions (`std::vector<ColumnDefinition>`) and row data (`std::vector<std::vector<Value>>`). `Value` is a `std::variant<int, std::string, double>`.
*   Handles parsing and execution of basic SQL commands.
*   Manages data persistence (saving/loading to file).
*   Maintains indexes (`std::map<std::string, std::map<Value, std::vector<int>>>`).

### 3.2 NLP Processor (`NLPProcessor` class)

*   Interfaces with the Google Gemini API.
*   Constructs prompts for the LLM, including database schema and command history context.
*   Uses `libcurl` to send requests to the Gemini API.
*   Parses the JSON response using `nlohmann/json` to extract the generated SQL query and reasoning.
*   Coordinates the execution of the generated SQL via the `Database` engine.

### 3.3 Configuration Reader (`ConfigReader` class)

*   A simple utility class responsible solely for reading the `GEMINI_API_KEY` from the `config.txt` file.
*   Performs basic validation (file exists, key exists, key not empty or placeholder).

### 3.4 Command Line Interface (CLI) (`main` function)

*   Provides the interactive user interface.
*   Reads user input line by line.
*   Parses the initial command word to determine the action (SQL, NLP, management command).
*   Routes the command to the appropriate handler (`Database::executeQuery` or `NLPProcessor::executeNLQuery`).
*   Handles basic errors and displays results or messages.

### 3.5 Data Flow (NLP Query)

1.  User enters `NLP <natural language query>` at the CLI.
2.  CLI identifies the `NLP` command and passes the query string to `NLPProcessor`.
3.  `NLPProcessor` retrieves the current database schema (table names, column names, types) and recent command history from the `Database`.
4.  `NLPProcessor` constructs a detailed prompt for the Gemini API, including the schema, history, and the user's query, asking for SQL and reasoning in JSON format.
5.  `NLPProcessor` sends the prompt to the Gemini API endpoint using `libcurl`.
6.  Gemini API processes the prompt and returns a JSON response containing the generated SQL and reasoning.
7.  `NLPProcessor` parses the JSON response.
8.  `NLPProcessor` prints the formatted NL query, generated SQL, and reasoning to the user.
9.  `NLPProcessor` passes the generated SQL string to `Database::executeQuery`.
10. `Database` parses and executes the SQL query, modifying the in-memory data structures.
11. Results (if any, e.g., from `SELECT`) or confirmation messages are printed to the CLI by the `Database` execution logic.
12. The executed NL query and generated SQL are added to the `NLPProcessor`'s command history.
![instruction_flow](https://github.com/user-attachments/assets/6e332339-4201-4adb-beeb-dcc855242b5a)

---

## 4. Command Line Interface (CLI)

### 4.1 Starting the CLI

Run the compiled executable (`./hexadb`). You will be greeted with:

```
Welcome to HexaDB Terminal
Type 'help' for commands, 'exit' to quit, or 'NLP <query>' for natural language queries.
HexaDB>
```

### 4.2 Command Structure

Enter commands directly at the `HexaDB>` prompt and press Enter. Commands are generally case-insensitive (e.g., `SELECT` is the same as `select`), but table and column names might be treated case-insensitively during lookup, although they are stored as entered. String literals within SQL commands are case-sensitive.

### 4.3 Available Commands

#### 4.3.1 SQL Data Definition Language (DDL)

*   `CREATE TABLE table_name (col1_name TYPE, col2_name TYPE, ...)`
    *   Creates a new table with the specified columns and data types.
    *   Example: `CREATE TABLE Users (UserID INT, Name TEXT, SignupDate REAL)`
*   `CREATE INDEX index_name ON table_name (column_name)`
    *   Creates an index on a single specified column of an existing table.
    *   Example: `CREATE INDEX idx_user_name ON Users (Name)`

#### 4.3.2 SQL Data Manipulation Language (DML)

*   `INSERT INTO table_name (col1, col2, ...) VALUES (val1, val2, ...)`
    *   Inserts a new row into the specified table. Column names must match the order of values.
    *   Example: `INSERT INTO Users (UserID, Name, SignupDate) VALUES (101, 'Alice', 2023.1026)`
*   `SELECT col1, col2 | * FROM table_name [WHERE condition]`
    *   Retrieves rows from a table. Select specific columns or all columns (`*`). Optionally filter rows with a `WHERE` clause.
    *   Example: `SELECT Name, UserID FROM Users WHERE UserID > 100`
    *   Example: `SELECT * FROM Products WHERE Price < 50.0`
*   `UPDATE table_name SET col1 = val1, col2 = val2, ... [WHERE condition]`
    *   Modifies existing rows in a table. Optionally update only rows matching the `WHERE` condition.
    *   Example: `UPDATE Products SET Price = 45.0 WHERE Name = 'Gadget'`
*   `DELETE FROM table_name [WHERE condition]`
    *   Removes rows from a table. Optionally delete only rows matching the `WHERE` condition. If no `WHERE` clause is provided, **all rows will be deleted**.
    *   Example: `DELETE FROM Logs WHERE Timestamp < 2023.0101`

#### 4.3.3 Database Management

*   `PRINT TABLE table_name`
    *   Displays the entire contents of the specified table in a formatted manner, including column names and types.
    *   Example: `PRINT TABLE Users`
*   `SAVE DB [filename]`
    *   Saves the current state of the entire database (all tables and data) to a file. If `filename` is omitted, it defaults to `hexadb.data`.
    *   Example: `SAVE DB my_database_backup.dat`
    *   Example: `SAVE DB`
*   `LOAD DB [filename]`
    *   Loads the database state from the specified file, **replacing the current in-memory database**. If `filename` is omitted, it defaults to `hexadb.data`.
    *   Example: `LOAD DB my_database_backup.dat`
    *   Example: `LOAD DB`

#### 4.3.4 Natural Language Processing (NLP)

*   `NLP <natural language query>`
    *   Sends the natural language query to the Gemini API to be translated into SQL, which is then executed.
    *   Example: `NLP show all users with id greater than 50`
    *   Example: `NLP create a table named Orders with columns order_id INT and product_name TEXT`

#### 4.3.5 Utility Commands

*   `help`
    *   Displays a summary of available commands and syntax.
*   `exit`
    *   Exits the HexaDB CLI. Does **not** automatically save the database. Use `SAVE DB` first if needed.

---

## 5. SQL Reference

### 5.1 Overview

HexaDB supports a limited subset of standard SQL syntax. The parser is basic and relies on specific keyword ordering and spacing. Complex queries involving JOINs, subqueries, functions, aliases, or complex boolean logic (`AND`/`OR`) in `WHERE` clauses are **not** supported.

### 5.2 Data Types

*   `INT`: Used for whole numbers (integers). Stored internally as `int`.
*   `TEXT`: Used for character strings. Stored internally as `std::string`.
*   `REAL`: Used for floating-point numbers (numbers with decimals). Stored internally as `double`.

### 5.3 SQL Commands

#### 5.3.1 `CREATE TABLE`

```sql
CREATE TABLE table_name (
    column_name1 DATA_TYPE,
    column_name2 DATA_TYPE,
    ...
);
-- Note: The trailing semicolon is optional in the CLI but good practice.
-- Parentheses are mandatory.
-- At least one column definition is required by the parser.
```

#### 5.3.2 `INSERT INTO`

```sql
INSERT INTO table_name (column1, column2, ...)
VALUES (value1, value2, ...);
-- Parentheses for column names and values are mandatory.
-- The number and order of columns must match the number and order of values.
-- Data types of values must be compatible with the corresponding column types.
-- Use single quotes for TEXT literals (e.g., 'hello world').
```

#### 5.3.3 `SELECT`

```sql
SELECT column1, column2, ... | *
FROM table_name
[WHERE column_name OPERATOR value];
-- Select specific columns by name, or all columns using *.
-- The WHERE clause is optional.
```

#### 5.3.4 `UPDATE`

```sql
UPDATE table_name
SET column1 = value1, column2 = value2, ...
[WHERE column_name OPERATOR value];
-- Updates one or more columns for rows matching the WHERE condition.
-- If WHERE is omitted, ALL rows in the table are updated.
-- Comma-separate multiple assignments in the SET clause.
```

#### 5.3.5 `DELETE FROM`

```sql
DELETE FROM table_name
[WHERE column_name OPERATOR value];
-- Deletes rows matching the WHERE condition.
-- If WHERE is omitted, ALL rows in the table are deleted.
```

#### 5.3.6 `CREATE INDEX`

```sql
CREATE INDEX index_name ON table_name (column_name);
-- Creates an index named 'index_name' on a single column 'column_name' of 'table_name'.
-- Parentheses around the column name are mandatory.
```

### 5.4 WHERE Clause

The `WHERE` clause is used in `SELECT`, `UPDATE`, and `DELETE` statements to filter rows. HexaDB supports only very simple conditions:

*   **Format:** `WHERE column_name OPERATOR value`
*   **Supported Operators:**
    *   `=` or `==`: Equal to (for `INT`, `REAL`, `TEXT`)
    *   `!=`: Not equal to (for `INT`, `REAL`, `TEXT`)
    *   `<`: Less than (primarily for `INT`, `REAL`)
    *   `>`: Greater than (primarily for `INT`, `REAL`)
*   **Limitations:**
    *   Only one condition is supported per `WHERE` clause. `AND` / `OR` are not supported.
    *   Comparisons (`<`, `>`) are only reliably implemented for numeric types (`INT`, `REAL`). String comparison using `<` or `>` is not supported.
    *   No complex expressions or function calls.

### 5.5 Literals

*   **INT:** Specify as whole numbers (e.g., `123`, `-45`).
*   **REAL:** Specify using decimal points (e.g., `99.95`, `-0.5`, `100.0`). Scientific notation is not explicitly supported by the parser.
*   **TEXT:** Enclose in single quotes (e.g., `'John Doe'`, `'Example Text'`). The parser might handle unquoted text for single words in some contexts (like `WHERE name = Alice`), but using quotes is strongly recommended for clarity and correctness, especially with spaces or special characters.

### 5.6 Identifiers

*   Table names, column names, and index names.
*   Should ideally start with a letter and contain letters, numbers, or underscores.
*   Lookup is generally case-insensitive (e.g., `Users` is treated the same as `users`), but the original casing is preserved for storage and display. Avoid creating tables/columns whose names differ only by case.

---

## 6. Natural Language Processing (NLP)

### 6.1 Overview

The NLP feature allows users to execute database operations by typing queries in plain English instead of strict SQL. HexaDB uses the Google Gemini API to translate these natural language requests into SQL queries.

### 6.2 Usage

Prefix your natural language query with the `NLP` keyword:

```
HexaDB> NLP <your query in English>
```

### 6.3 How it Works

1.  You provide a query like `NLP show me all products cheaper than $50`.
2.  HexaDB sends this query, along with the current database schema (tables and columns) and a short history of recent NL/SQL commands, to the Google Gemini API.
3.  Gemini analyzes the request in the context of your database structure and attempts to generate the corresponding SQL query (e.g., `SELECT * FROM Products WHERE Price < 50.0`). It also provides a brief explanation (reasoning) for how it arrived at that SQL.
4.  HexaDB receives the SQL and reasoning from Gemini.
5.  It displays the original query, the generated SQL, and the reasoning to you for review.
6.  It then automatically executes the generated SQL query against the database.

### 6.4 Examples

*   `NLP create table Customers (id INT, name TEXT, city TEXT)`
*   `NLP insert a customer with id 1, name 'Bob', city 'New York'`
*   `NLP show names of customers in 'London'`
*   `NLP update the city to 'Paris' for the customer named 'Bob'`
*   `NLP how many customers are there?` (May generate `SELECT COUNT(*) FROM Customers`, although `COUNT(*)` might not be fully supported by the simple HexaDB `SELECT` parser/executor).
*   `NLP delete the customer with id 5`

### 6.5 Tips for Effective Queries

*   **Be Specific:** Clearly mention table names, column names, and values if known.
*   **Keep it Simple:** Avoid overly complex sentences or ambiguous requests. Remember the underlying SQL capabilities are limited.
*   **Use Keywords:** Employ terms commonly used in database operations like "show", "find", "list", "create", "add", "insert", "update", "change", "delete", "remove", "where", "with", etc.
*   **Check the Schema:** If unsure about table/column names, use `PRINT TABLE` first.

### 6.6 Output Format

When you run an `NLP` command, HexaDB will typically output:

1.  A formatted box showing:
    *   Your original Natural Language Query.
    *   The SQL query generated by Gemini.
    *   The Reasoning provided by Gemini explaining the translation.
2.  The standard output from executing the generated SQL command (e.g., selected rows, confirmation messages).

### 6.7 Reliability

The accuracy of the NLP feature depends entirely on the Gemini LLM's ability to understand your query and the provided schema/context, and to generate correct, HexaDB-compatible SQL.
*   **Potential for Errors:** The LLM might misinterpret your query or generate SQL that is syntactically incorrect or not supported by HexaDB's limited SQL parser.
*   **No Validation:** HexaDB executes the generated SQL directly without an intermediate validation step. Review the generated SQL carefully before relying on the results.

---

## 7. Persistence

### 7.1 Overview

HexaDB operates primarily in memory. To prevent data loss when the application closes, you can save the current state of the database to a file and load it back later.

### 7.2 `SAVE DB` Command

```
SAVE DB [filename]
```

*   Saves all tables (schema and data) to the specified file.
*   If `filename` is omitted, it defaults to `hexadb.data` in the current working directory.
*   Overwrites the file if it already exists.
*   Reports success or failure to the console.

### 7.3 `LOAD DB` Command

```
LOAD DB [filename]
```

*   Loads the database state from the specified file.
*   **Important:** This completely replaces the database currently in memory. Any unsaved changes will be lost.
*   If `filename` is omitted, it attempts to load from `hexadb.data`.
*   Reports success or failure. If the file is not found or has an invalid format, an error is reported, and the in-memory database state might be left empty or in an inconsistent state.

### 7.4 File Format (`hexadb.data`)

HexaDB uses a custom, human-readable text format for saving data:

```
DATABASE_NAME <db_name>
TABLE_COUNT <number_of_tables>
TABLE_NAME <table_name_1>
COLUMN_COUNT <number_of_columns_in_table_1>
COLUMN <col1_name> <col1_type_enum_int>
COLUMN <col2_name> <col2_type_enum_int>
...
ROW_COUNT <number_of_rows_in_table_1>
ROW <type_prefix_1> <value_1> <type_prefix_2> <value_2> ...
ROW <type_prefix_1> <value_1> <type_prefix_2> <value_2> ...
...
TABLE_NAME <table_name_2>
... (columns and rows for table 2) ...
```

*   `DATA_TYPE` enums: `INT`=0, `TEXT`=1, `REAL`=2.
*   `ROW` data type prefixes: `I` for INT, `T` for TEXT, `R` for REAL.
*   `TEXT` values are enclosed in double quotes (`"`) within the ROW data line.

### 7.5 Considerations

*   The text format is simple but potentially brittle. Manual editing is risky and can easily corrupt the file, making it unreadable by `LOAD DB`.
*   Saving/loading large databases can be slow compared to binary formats.
*   No transactional guarantees during save/load.

---

## 8. Indexing

### 8.1 Overview

HexaDB provides basic support for single-column indexes. Indexes are data structures that can potentially speed up data retrieval operations (specifically `SELECT`, `UPDATE`, `DELETE` with `WHERE` clauses) by avoiding full table scans for certain types of queries.

### 8.2 `CREATE INDEX` Command

```sql
CREATE INDEX index_name ON table_name (column_name);
```

*   Creates an index named `index_name` on the specified `column_name` within `table_name`.
*   The index internally maps distinct values in the column to the list of row indices (positions) containing that value.

### 8.3 Usage and Limitations

*   **Current Implementation:** The current HexaDB implementation **may not fully utilize indexes** to optimize `SELECT`, `UPDATE`, or `DELETE` operations involving `WHERE` clauses, especially for range queries (`<`, `>`). The index structure is primarily suited for speeding up exact match lookups (`=`). Index creation itself populates the structure.
*   **Update/Delete Overhead:** When rows are updated or deleted, the indexes need to be maintained. The current implementation might rebuild indexes inefficiently after modifications, potentially slowing down `UPDATE` and `DELETE` operations, especially on large tables.
*   **Single Column Only:** Only single-column indexes are supported.
*   **Persistence:** Index structures themselves are not explicitly saved in the `hexadb.data` file. They are rebuilt when data is loaded or modified if necessary (though the `CREATE INDEX` command itself isn't saved, so indexes aren't automatically recreated on load unless `CREATE INDEX` is run again).

---

## 9. Configuration (`config.txt`)

### 9.1 Location and Format

*   HexaDB expects a configuration file named `config.txt` in the same directory where the executable is run.
*   The format is simple key-value pairs, one per line: `KEY=value`.
*   Lines starting with `#` are treated as comments and ignored.
*   Whitespace around the key and value is trimmed.

### 9.2 `GEMINI_API_KEY`

*   This is the only configuration key currently used.
*   It must contain your valid Google Gemini API key for the NLP functionality to work.
    ```
    GEMINI_API_KEY=AIza...................
    ```

### 9.3 Security Considerations

*   **Plain Text Storage:** Storing API keys in plain text files like `config.txt` is a security risk. Ensure the file has restrictive permissions (readable only by the user running HexaDB).
*   **Never commit `config.txt`** or files containing API keys to version control systems (like Git), especially public repositories. Consider using environment variables as a more secure alternative in real applications.

---

## 10. API Integration (Google Gemini)

### 10.1 Purpose

HexaDB integrates with the Google Gemini API to provide its Natural Language Processing (NLP) feature, translating user queries in English into executable SQL.

### 10.2 Mechanism

*   Uses the `libcurl` C library to make HTTPS POST requests to the Gemini API endpoint (`https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent`).
*   Sends a JSON payload containing the prompt (user query, schema, history).
*   Receives a JSON response containing the generated SQL and reasoning.
*   Requires a valid API key passed in the request URL.

### 10.3 Data Sent

When using the `NLP` command, the following information is sent to Google's servers:

*   Your natural language query.
*   Your database schema: Table names, column names, and their data types (`INT`, `TEXT`, `REAL`).
*   A recent history (up to 10) of previous `NLP` queries and the SQL generated for them.
*   **No actual row data** from your tables is sent.

Be mindful of privacy implications if your schema contains sensitive information.

### 10.4 Dependencies

*   Requires a working internet connection to reach the Gemini API.
*   Requires a valid, non-expired Gemini API key with the necessary permissions.
*   Dependent on the availability and terms of service of the Google Gemini API.

---

## 11. Limitations and Considerations

*   **SQL Parser:** Very basic, cannot handle complex syntax (JOINs, subqueries, `AND`/`OR`, functions, aliases). Sensitive to formatting.
*   **Performance:** In-memory, but operations like `SELECT`, `UPDATE`, `DELETE` often involve full table scans. Index utilization is limited and updates can be inefficient. Not suitable for large datasets or high-performance needs.
*   **Concurrency:** Strictly single-threaded. No support for concurrent access.
*   **NLP Reliability:** Accuracy depends entirely on the external LLM. Generated SQL can be incorrect or incompatible. No validation before execution.
*   **Persistence Format:** Custom text format is simple but potentially fragile and less efficient than binary formats.
*   **Error Handling:** Primarily uses basic `std::runtime_error` exceptions. Error reporting could be more granular. Database loading error recovery is limited.
*   **Security:** API key stored in plain text. Schema information sent to Google for NLP.
*   **Data Integrity:** No support for transactions, constraints (foreign keys, unique, etc.), or complex data validation beyond basic type checks during insertion/update.
*   **Feature Set:** Lacks many standard database features (views, stored procedures, user management, permissions, advanced data types, etc.).

---

## 12. Troubleshooting

*   **Error: "Could not open config file..."**: Ensure `config.txt` exists in the same directory as the `hexadb` executable and has read permissions.
*   **Error: "GEMINI_API_KEY not found..." or "...is empty"**: Check `config.txt` to ensure the line `GEMINI_API_KEY=your_key` exists and `your_key` is not empty or the placeholder.
*   **Error: "Please replace the default API key..."**: You need to replace `your_api_key_here` in `config.txt` with your actual key.
*   **NLP commands fail (CURL error, JSON parsing error, etc.)**:
    *   Check your internet connection.
    *   Verify your Gemini API key is correct and active in `config.txt`.
    *   Ensure Google's Gemini API service is operational.
    *   The API response format might have changed, breaking the JSON parsing in `nlp_processor.cpp`.
*   **Error: "Table '...' not found" / "Column '...' not found"**: Check spelling and case (though lookup is often case-insensitive, consistency helps). Use `PRINT TABLE` to verify existence.
*   **Error: "Invalid value..." / "Error converting value..."**: Ensure the data you provide in `INSERT` or `UPDATE` matches the column's data type (e.g., don't put text in an INT column). Use single quotes for TEXT literals.
*   **Error: "Database file not found..." / Load errors**: Ensure the `.data` file exists and is in the correct location. The file might be corrupted if manually edited. Try saving again or starting fresh.
*   **SQL Syntax Errors (often generic "Error: ...")**: HexaDB's parser is strict. Check spacing, keyword order, parentheses, and commas carefully against the documented syntax. Use `help` for reminders. Remember complex SQL is not supported.

---
