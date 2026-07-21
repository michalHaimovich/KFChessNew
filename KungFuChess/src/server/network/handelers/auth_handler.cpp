#include "auth_handler.hpp"

AuthHandler::AuthHandler(UserRepository& repo) : m_userRepo(repo) {}

bool AuthHandler::handleLogin(const nlohmann::json& j, ClientSession& client, std::string& errorMessage) {
    std::string user = j.value("username", "Guest");
    std::string pass = j.value("password", "");

    auto existingUser = m_userRepo.getByUsername(user);

    if (existingUser.has_value()) {
        if (existingUser->password == pass) {
            client.rating = existingUser->rating;
            client.username = user;
            client.state = ClientState::LOBBY;
            return true;
        } else {
            errorMessage = "Invalid password";
            return false;
        }
    } else {
        User newUser(user, pass);
        if (m_userRepo.create(newUser)) {
            client.rating = newUser.rating;
            client.username = user;
            client.state = ClientState::LOBBY;
            return true;
        } else {
            errorMessage = "Database error";
            return false;
        }
    }
}