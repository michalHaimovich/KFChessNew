#pragma once
#include <string>
#include <optional>
#include "../database_connection.hpp"
#include "../entities/user.hpp"

class UserRepository {
private:
    sqlite3* db;

public:
    explicit UserRepository(DatabaseConnection& connection);

    void initializeTable();

    bool create(const User& user);
    std::optional<User> getByUsername(const std::string& username);
    bool updateRating(const std::string& username, int newRating);
};