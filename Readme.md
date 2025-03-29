
<p align="center">
  <img src="https://github.com/user-attachments/assets/75d0f6fe-07c9-4f9b-a5fc-3cafea265563" alt="logo" width=300 margin=0>
</p>

# HexaDB

An AI-powered in-memory relational database system that supports SQL-like operations, enhanced with AI-based natural language processing.

![High-Level Overview Diagram](https://github.com/user-attachments/assets/f1bc336b-d20e-407a-96d4-5ee4e3076cf8)

**Version:** 1.0 | **Author:** j.sutradhar@symbola.io

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
13. ![Contributing](#13-contributing)
---
## 1. Introduction

### 1.1 What is HexaDB?

HexaDB is a lightweight, in-memory relational database implemented in C++. It provides basic SQL-like functionality (CRUD operations) and integrates with Google's Gemini API, allowing users to interact via natural language queries translated into SQL.

![NLP to Database Query Flow Diagram](https://github.com/user-attachments/assets/acbdebcd-8f80-44b9-be68-10f4c5060271)

### 1.2 Key Features

*   **In-Memory:** Fast data access; data lost on exit unless saved.
*   **SQL Support:** Core commands (`CREATE TABLE`, `INSERT`, `SELECT`, `UPDATE`, `DELETE`).
*   **Natural Language Processing (NLP):** Execute queries using plain English via the Gemini API.
*   **Persistence:** Save/load database state to/from a file.
*   **Basic Data Types:** Supports `INT`, `TEXT`, `REAL`.
*   **Simple Indexing:** Basic single-column indexing support.

### 1.3 Intended Use Cases

*   Learning database internals and C++.
*   Demonstrating NLP-database integration.
*   Prototyping simple applications.
*   Temporary personal data storage.

---

## 2. Getting Started

### 2.1 Prerequisites

*   **C++17 Compiler:** GCC (g++) or Clang recommended.
*   **libcurl:** Development library (e.g., `libcurl4-openssl-dev` on Debian/Ubuntu, `libcurl-devel` on Fedora).
*   **nlohmann/json:** Single header file (`json.hpp`) from [https://github.com/nlohmann/json](https://github.com/nlohmann/json).
*   **Google Gemini API Key:** For NLP functionality.

### 2.2 Configuration

1.  Create `config.txt` in the same directory as the executable.
2.  Add the line: `GEMINI_API_KEY=your_actual_gemini_api_key`
3.  **Important:** Secure this file, as it contains your API key.

### 2.3 Compilation

Compile the source files (`.cpp`, `.h`), ensuring `json.hpp` is accessible:

```bash
g++ -std=c++17 hexadb.cpp nlp_processor.cpp -o hexadb -lcurl
```

*   `-std=c++17`: Enables required C++ features.
*   `-lcurl`: Links the cURL library.

### 2.4 Running HexaDB

```bash
./hexadb
```

You'll see the welcome message and the `HexaDB>` prompt.

---

## 3. Architecture

![HexaDB Architecture Overview Diagram](https://github.com/user-attachments/assets/4af3d5b2-532f-4edd-a84d-507c153bb954)

HexaDB comprises several interacting components:

*   **Core Database Engine:** Manages tables, rows (`std::variant` based `Value`), columns, indexes, SQL execution, and persistence.
*   **NLP Processor:** Interfaces with Gemini API (via `libcurl`) using schema/history context, parses JSON responses (`nlohmann/json`), and triggers SQL execution.
*   **Configuration Reader:** Reads the `GEMINI_API_KEY` from `config.txt`.
*   **Command Line Interface (CLI):** Provides the interactive terminal, reads input, routes commands to the appropriate processor (DB Engine or NLP), and displays output.

The **NLP Query Data Flow** is visualized below:

![System Interaction Flow Diagram](https://github.com/user-attachments/assets/6e332339-4201-4adb-beeb-dcc855242b5a)

---

## 4. Command Line Interface (CLI)

### 4.1 Starting the CLI

Run `./hexadb`. The prompt `HexaDB>` indicates readiness.

### 4.2 Command Structure

Enter commands at the prompt. SQL keywords are case-insensitive; lookups often ignore case, but string literals are case-sensitive.

### 4.3 Available Commands

#### 4.3.1 SQL DDL
*   `CREATE TABLE name (col1 TYPE, ...)`: Creates a table.
    *   Ex: `CREATE TABLE Users (UserID INT, Name TEXT)`
*   `CREATE INDEX name ON table (column)`: Creates a single-column index.
    *   Ex: `CREATE INDEX idx_uname ON Users (Name)`

#### 4.3.2 SQL DML
*   `INSERT INTO table (cols) VALUES (vals)`: Inserts a row.
    *   Ex: `INSERT INTO Users (UserID, Name) VALUES (101, 'Alice')`
*   `SELECT cols | * FROM table [WHERE condition]`: Retrieves rows.
    *   Ex: `SELECT Name FROM Users WHERE UserID > 100`
*   `UPDATE table SET col=val, ... [WHERE condition]`: Modifies rows.
    *   Ex: `UPDATE Users SET Name = 'Bob' WHERE UserID = 101`
*   `DELETE FROM table [WHERE condition]`: Removes rows (all if no `WHERE`).
    *   Ex: `DELETE FROM Users WHERE UserID = 101`

#### 4.3.3 Database Management
*   `PRINT TABLE name`: Displays table contents formatted.
    *   Ex: `PRINT TABLE Users`
*   `SAVE DB [filename]`: Saves database state (default: `hexadb.data`).
    *   Ex: `SAVE DB backup.hdb`
*   `LOAD DB [filename]`: Loads state, **replacing current data** (default: `hexadb.data`).
    *   Ex: `LOAD DB backup.hdb`

#### 4.3.4 Natural Language Processing (NLP)
*   `NLP <natural language query>`: Translates query to SQL via Gemini API and executes.
    *   Ex: `NLP show all users`
    *   Ex: `NLP create table products (id int, price real)`

#### 4.3.5 Utility Commands
*   `help`: Displays command summary.
*   `exit`: Quits HexaDB (does not auto-save).

---

## 5. SQL Reference

### 5.1 Overview

Supports a limited SQL subset. Parser is basic; complex queries (JOINs, subqueries, `AND`/`OR`, functions) are **not** supported.

### 5.2 Data Types

*   `INT`: Whole numbers.
*   `TEXT`: Character strings.
*   `REAL`: Floating-point numbers.

### 5.3 SQL Commands

*(Syntax blocks remain. Explanations slightly shortened.)*

#### 5.3.1 `CREATE TABLE`
```sql
CREATE TABLE table_name (col1 TYPE, col2 TYPE, ...);
-- Parentheses required; at least one column needed.
```

#### 5.3.2 `INSERT INTO`
```sql
INSERT INTO table_name (col1, col2, ...) VALUES (val1, val2, ...);
-- Column/value counts must match; types must be compatible.
-- Use single quotes for TEXT literals ('example').
```

#### 5.3.3 `SELECT`
```sql
SELECT col1, col2 | * FROM table_name [WHERE condition];
-- Select specific columns or all (*). WHERE is optional.
```

#### 5.3.4 `UPDATE`
```sql
UPDATE table_name SET col1 = val1, ... [WHERE condition];
-- Modifies rows matching WHERE (or all rows if omitted).
```

#### 5.3.5 `DELETE FROM`
```sql
DELETE FROM table_name [WHERE condition];
-- Removes rows matching WHERE (or all rows if omitted).
```

#### 5.3.6 `CREATE INDEX`
```sql
CREATE INDEX index_name ON table_name (column_name);
-- Creates index on a single specified column.
```

### 5.4 WHERE Clause

Filters rows in `SELECT`, `UPDATE`, `DELETE`. Simple format: `WHERE column OPERATOR value`.

*   **Operators:** `=`, `==`, `!=`, `<`, `>`
*   **Limitations:** Only one condition; no `AND`/`OR`. `<`/`>` primarily for `INT`/`REAL`.

### 5.5 Literals

*   **INT:** `123`, `-45`
*   **REAL:** `99.95`, `100.0`
*   **TEXT:** `'John Doe'`, `'Hello'` (use single quotes)

### 5.6 Identifiers

Table/column/index names. Should ideally be alphanumeric + underscore. Lookup is often case-insensitive.

---

## 6. Natural Language Processing (NLP)

### 6.1 Overview

Execute database operations using plain English queries, translated to SQL via the Google Gemini API.

![Example NLP Terminal Output](https://github.com/user-attachments/assets/377f6346-94ff-4cfc-9bbe-571ac97ffc0a)

### 6.2 Usage

Prefix your query with `NLP`:
```
HexaDB> NLP <your query in English>
```

### 6.3 How it Works

HexaDB sends your query, current database schema, and recent command history to the Gemini API. Gemini returns a generated SQL query and reasoning. HexaDB displays this information, then executes the SQL.

### 6.4 Examples

*   `NLP create table Customers (id INT, name TEXT)`
*   `NLP insert customer id 1, name 'Bob'`
*   `NLP show names of customers`
*   `NLP update customer name to 'Alice' where id = 1`
*   `NLP delete customer with id 5`

### 6.5 Tips for Effective Queries

*   Be specific (table/column names, values).
*   Keep queries simple.
*   Use common keywords (show, list, create, add, update, delete, where).
*   Use `PRINT TABLE` to check schema if unsure.

### 6.6 Output Format

Typically shows a box with your NL query, the generated SQL, and Gemini's reasoning, followed by the result of the SQL execution.

### 6.7 Reliability

Accuracy depends on the Gemini LLM understanding the query and schema.
*   Incorrect or incompatible SQL may be generated.
*   There is no validation before execution; review the generated SQL.

---

## 7. Persistence

### 7.1 Overview

Save/load the in-memory database state to prevent data loss.

### 7.2 `SAVE DB` Command
```
SAVE DB [filename]
```
Saves current state (default: `hexadb.data`). Overwrites existing file.

### 7.3 `LOAD DB` Command
```
LOAD DB [filename]
```
Loads state (default: `hexadb.data`), **replacing** current database.

### 7.4 File Format (`hexadb.data`)

Custom human-readable text format.
```
DATABASE_NAME ...
TABLE_COUNT ...
TABLE_NAME ...
COLUMN_COUNT ...
COLUMN name type_enum
...
ROW_COUNT ...
ROW type_prefix value ...
...
```
*   Type Enums: `INT=0`, `TEXT=1`, `REAL=2`
*   Row Prefixes: `I`(INT), `T`(TEXT), `R`(REAL). TEXT values are quoted (`"`).

### 7.5 Considerations

*   Format is simple but brittle; manual editing is risky.
*   Potentially slow for very large databases.
*   No transactional guarantees.

---

## 8. Indexing

### 8.1 Overview

Basic single-column indexes to potentially speed up exact match lookups.

### 8.2 `CREATE INDEX` Command
```sql
CREATE INDEX index_name ON table_name (column_name);
```
Creates an index mapping column values to row indices.

### 8.3 Usage and Limitations

*   **Limited Optimization:** May not significantly speed up `WHERE` clauses, especially range queries (`<`, `>`). Primarily for exact matches (`=`).
*   **Update/Delete Overhead:** Maintaining indexes can slow down modifications. Rebuilds may be inefficient.
*   **Single Column Only.**
*   **Not Persistent:** Index structures are not saved in the `.data` file and need `CREATE INDEX` to be run again after loading.

---

## 9. Configuration (`config.txt`)

### 9.1 Location and Format

Expected in the runtime directory. Simple `KEY=value` format. `#` denotes comments.

### 9.2 `GEMINI_API_KEY`

The only key used; requires your valid Google Gemini API key.
```
GEMINI_API_KEY=AIza...................
```

### 9.3 Security Considerations

*   **Plain text API keys are a risk.** Restrict file permissions.
*   **Do not commit `config.txt` to Git.** Consider environment variables for better security.

---

## 10. API Integration (Google Gemini)

### 10.1 Purpose

Enables the NLP feature by translating English queries to SQL.

### 10.2 Mechanism

Uses `libcurl` to send HTTPS POST requests (with schema/history/query in JSON) to the Gemini API endpoint, requiring the API key. Receives JSON response with SQL/reasoning.

### 10.3 Data Sent

*   Your natural language query.
*   Database schema (table/column names, types).
*   Recent command history (NL/SQL pairs).
*   **No actual row data is sent.** Be mindful of sensitive schema information.

### 10.4 Dependencies

*   Internet connection.
*   Valid Gemini API key.
*   Availability of the Google Gemini API service.

---

## 11. Limitations and Considerations

*   **SQL Parser:** Basic, limited syntax, sensitive to format.
*   **Performance:** In-memory but scans common; index use limited; not for large data/high load.
*   **Concurrency:** Single-threaded only.
*   **NLP Reliability:** Depends on external LLM; potential for errors; no validation.
*   **Persistence:** Simple text format, potentially brittle/slow.
*   **Error Handling:** Basic runtime errors.
*   **Security:** Plain text API key; schema sent externally.
*   **Data Integrity:** No transactions or constraints.
*   **Feature Set:** Lacks advanced database features.

---

## 12. Troubleshooting

*   **"Could not open config file..."**: Check `config.txt` exists, is readable.
*   **"GEMINI_API_KEY not found/empty/default"**: Verify `config.txt` content and key value.
*   **NLP fails (CURL/JSON error)**: Check internet, API key validity/status, Gemini service status.
*   **"Table/Column not found"**: Check spelling/case; use `PRINT TABLE`.
*   **"Invalid value/Error converting"**: Ensure data matches column type; use single quotes for `TEXT`.
*   **Load errors**: Check `.data` file existence/location; avoid manual edits.
*   **SQL Syntax Errors**: Check spacing, keywords, parentheses, commas; refer to SQL Reference; remember limitations.

---

## 13. Contributing

We welcome contributions to HexaDB! As an educational project and prototype, there's plenty of room for improvement and new features. Whether it's fixing bugs, enhancing the SQL parser, improving the NLP integration, adding more robust error handling, writing tests, or improving this documentation, your help is appreciated. Please feel free to fork the repository, create a feature branch, and submit a pull request. If you find an issue or have a feature suggestion, please open an issue in the project's issue tracker first to discuss it. Thank you for considering contributing!
