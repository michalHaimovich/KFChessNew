#include "database_connection.hpp"
#include <sqlite3.h>
#include <iostream>

DatabaseConnection::DatabaseConnection(const std::string& dbPath) : db(nullptr) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        // במערכת אמיתית נהוג לזרוק חריגה (Exception) כאן אם ה-DB חיוני לעליית השרת
    }
}

DatabaseConnection::~DatabaseConnection() {
    if (db) {
        sqlite3_close(db);
    }
}

sqlite3* DatabaseConnection::getHandle() const {
    return db;
}

bool DatabaseConnection::isOpen() const {
    return db != nullptr;
}