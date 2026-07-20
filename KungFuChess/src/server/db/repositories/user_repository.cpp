#include "user_repository.hpp"
#include <sqlite3.h>
#include <iostream>

UserRepository::UserRepository(DatabaseConnection& connection) {
    db = connection.getHandle();
}

void UserRepository::initializeTable() {
    if (!db) return;

    const char* sql = 
        "CREATE TABLE IF NOT EXISTS Users ("
        "username TEXT PRIMARY KEY, "
        "password TEXT NOT NULL, "
        "rating INTEGER NOT NULL"
        ");";
    
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creating Users table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool UserRepository::create(const User& user) {
    if (!db) return false;

    const char* sql = "INSERT INTO Users (username, password, rating) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false; 
    }
    
    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, user.rating);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

std::optional<User> UserRepository::getByUsername(const std::string& username) {
    if (!db) return std::nullopt;

    const char* sql = "SELECT username, password, rating FROM Users WHERE username = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    
    std::optional<User> result = std::nullopt;
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        u.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.rating = sqlite3_column_int(stmt, 2);
        result = u;
    }
    
    sqlite3_finalize(stmt);
    return result;
}

bool UserRepository::updateRating(const std::string& username, int newRating) {
    if (!db) return false;

    const char* sql = "UPDATE Users SET rating = ? WHERE username = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, newRating);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}