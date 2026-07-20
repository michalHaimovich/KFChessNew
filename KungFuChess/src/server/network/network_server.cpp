#include "network_server.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

NetworkServer::NetworkServer()
    : m_gameSession([this](websocketpp::connection_hdl hdl, const std::string &msg)
                    { this->sendToClient(hdl, msg); }),
      m_controller(m_gameSession.getEngine())
{
    m_db = std::make_unique<DatabaseConnection>("kungfu_chess.db");
    m_userRepo = std::make_unique<UserRepository>(*m_db);
    m_userRepo->initializeTable();

    m_server.init_asio();

    m_server.set_open_handler(bind(&NetworkServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(bind(&NetworkServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(bind(&NetworkServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void NetworkServer::sendToClient(websocketpp::connection_hdl hdl, const std::string &message)
{
    try
    {
        m_server.send(hdl, message, websocketpp::frame::opcode::text);
    }
    catch (const websocketpp::exception &e)
    {
        std::cout << "Send failed: " << e.what() << std::endl;
    }
}

void NetworkServer::run(uint16_t port)
{
    m_server.listen(port);
    m_server.start_accept();
    std::cout << "Server listening on port " << port << "..." << std::endl;
    m_server.run();
}

void NetworkServer::on_open(websocketpp::connection_hdl hdl)
{
    connectedClients[hdl] = ClientSession();
    std::cout << "[NetworkServer] New client connected. Awaiting login." << std::endl;
}

void NetworkServer::on_close(websocketpp::connection_hdl hdl)
{
    // Clean up the client session from our map
    connectedClients.erase(hdl);

    // Also remove from the game session if they were already playing
    m_gameSession.removeClient(hdl);
    std::cout << "[NetworkServer] Client disconnected." << std::endl;
}

void NetworkServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    std::string payload = msg->get_payload();
    auto &client = connectedClients[hdl];

    if (client.state == ClientState::CONNECTED)
    {
        try
        {
            auto j = json::parse(payload);

            if (j.value("action", "") == "LOGIN")
            {
                std::string user = j.value("username", "Guest");
                std::string pass = j.value("password", "");

                auto existingUser = m_userRepo->getByUsername(user);
                bool loginSuccess = false;

                if (existingUser.has_value())
                {
                    if (existingUser->password == pass)
                    {
                        loginSuccess = true;
                        client.rating = existingUser->rating;
                        std::cout << "[NetworkServer] User authenticated: " << user << std::endl;
                    }
                    else
                    {
                        std::cout << "[NetworkServer] Invalid password for: " << user << std::endl;
                        sendToClient(hdl, R"({"type": "LOGIN_ERROR", "message": "Invalid password"})");
                    }
                }
                else
                {
                    User newUser(user, pass);
                    if (m_userRepo->create(newUser))
                    {
                        loginSuccess = true;
                        client.rating = newUser.rating;
                        std::cout << "[NetworkServer] New user registered and authenticated: " << user << std::endl;
                    }
                    else
                    {
                        std::cout << "[NetworkServer] Failed to create user: " << user << std::endl;
                        sendToClient(hdl, R"({"type": "LOGIN_ERROR", "message": "Database error"})");
                    }
                }

                if (loginSuccess)
                {
                    client.state = ClientState::LOBBY;
                    client.username = user;
                    sendToClient(hdl, R"({"type": "LOGIN_SUCCESS"})");
                }
            }
            else
            {
                std::cout << "[NetworkServer] Unauthenticated client sent non-login message." << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "[NetworkServer] Invalid JSON during login: " << e.what() << std::endl;
        }

        return;
    }

    PlayerRole role = m_gameSession.getRole(hdl);
    bool success = m_controller.processCommand(payload, role);

    if (!success)
    {
        std::cout << "[NetworkServer] Invalid command or unauthorized role." << std::endl;
    }
}