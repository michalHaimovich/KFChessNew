#pragma once
#include <string>

struct User {
    std::string username;
    std::string password;
    int rating;

    User(const std::string& u = "", const std::string& p = "", int r = 1200) 
        : username(u), password(p), rating(r) {}
};