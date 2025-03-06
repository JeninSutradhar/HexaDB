# HexaDB

A very simple in-memory database implemented in C++.
![large-removebg-preview](https://github.com/user-attachments/assets/1c1e56ac-e524-4c19-af68-847736e6e80d)

## Description

HexaDB is a basic command-line database system that supports creating tables, inserting, selecting, updating, and deleting rows. It also supports indexing and saving/loading databases to/from files.  This is a learning project and is not intended for production use.

## Compilation and Running

To compile and run the code, you'll need a C++ compiler that supports C++17 (or later).  g++ is recommended.

```bash
g++ -o hexadb main.cpp
./hexadb
```

# Architecture
![Architecture of HexaDB - visual selection](https://github.com/user-attachments/assets/7e755d14-cc22-4858-829c-700caee1e08d)


## Features

*   **Tables:** Create and manage tables with columns of type INT, TEXT, or REAL.
*   **CRUD Operations:**  Supports INSERT, SELECT, UPDATE, and DELETE operations with basic WHERE clause filtering.
*   **Indexing:**  Create indexes on columns to potentially speed up queries.
*   **Persistence:** Save and load databases to/from files.
*   **Command-Line Interface:** Interact with the database using SQL-like commands.

## Example Usage

```sql
CREATE TABLE users (id INT, name TEXT, age INT);
INSERT INTO users (id, name, age) VALUES (1, 'Alice', 30);
INSERT INTO users (id, name, age) VALUES (2, 'Bob', 25);
SELECT * FROM users;
SELECT name, age FROM users WHERE age > 28;
UPDATE users SET age = 31 WHERE name = 'Alice';
DELETE FROM users WHERE id = 2;
PRINT TABLE users;
SAVE DB my_database.data
LOAD DB my_database.data
```

Type `help` in the HexaDB terminal for a list of commands. Type `exit` to quit.
```
