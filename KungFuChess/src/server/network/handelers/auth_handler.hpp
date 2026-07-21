#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "db/repositories/user_repository.hpp"
#include "network/client_session.hpp" 

class AuthHandler {
private:
    UserRepository& m_userRepo;

public:
    AuthHandler(UserRepository& repo);
    
    bool handleLogin(const nlohmann::json& j, ClientSession& client, std::string& errorMessage);
};