#pragma once
#include <string>

struct sqlite3; 

class DatabaseConnection {
private:
    sqlite3* db;

public:
    explicit DatabaseConnection(const std::string& dbPath);
    
    ~DatabaseConnection();

    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    sqlite3* getHandle() const;
    
    bool isOpen() const;
};