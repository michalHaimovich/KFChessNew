#include "network_server.hpp"
#include <iostream>
#include <nlohmann/json.hpp> // NEW: Required to parse the login message

using json = nlohmann::json;

NetworkServer::NetworkServer() 
    : m_gameSession([this](websocketpp::connection_hdl hdl, const std::string& msg) { 
          this->sendToClient(hdl, msg); 
      }), 
      m_controller(m_gameSession.getEngine()) 
{    
    m_server.init_asio();

    m_server.set_open_handler(bind(&NetworkServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(bind(&NetworkServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(bind(&NetworkServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void NetworkServer::sendToClient(websocketpp::connection_hdl hdl, const std::string& message) {
    try {
        m_server.send(hdl, message, websocketpp::frame::opcode::text);
    } catch (const websocketpp::exception& e) {
        std::cout << "Send failed: " << e.what() << std::endl;
    }
}

void NetworkServer::run(uint16_t port) {
    m_server.listen(port);
    m_server.start_accept();
    std::cout << "Server listening on port " << port << "..." << std::endl;
    m_server.run(); 
}

void NetworkServer::on_open(websocketpp::connection_hdl hdl) {
    // NEW: Just create a new unauthenticated session. 
    // Do NOT add to GameSession yet!
    connectedClients[hdl] = ClientSession(); 
    std::cout << "[NetworkServer] New client connected. Awaiting login." << std::endl;
}

void NetworkServer::on_close(websocketpp::connection_hdl hdl) {
    // NEW: Clean up the client session from our map
    connectedClients.erase(hdl);
    
    // Also remove from the game session if they were already playing
    m_gameSession.removeClient(hdl);
    std::cout << "[NetworkServer] Client disconnected." << std::endl;
}

void NetworkServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::string payload = msg->get_payload(); 
    auto& client = connectedClients[hdl];

    // NEW: Gatekeeper - Check if the client is authenticated
    if (!client.isAuthenticated) {
        try {
            auto j = json::parse(payload);
            
            if (j.value("action", "") == "LOGIN") {
                std::string user = j.value("username", "Guest");
                std::string pass = j.value("password", "");
                
                // For now, accept any credentials (presentation only). 
                // Future: This is exactly where you will query the Database!
                std::cout << "[NetworkServer] User authenticated: " << user << std::endl;
                
                client.isAuthenticated = true;
                client.username = user;
                
                // Now that they are authenticated, assign them to the game.
                // The first will get White, the second Black, others Spectator.
                m_gameSession.addClient(hdl);
                
                // Send a success acknowledgment back to the client
                sendToClient(hdl, R"({"type": "LOGIN_SUCCESS"})");
            } else {
                std::cout << "[NetworkServer] Unauthenticated client sent non-login message." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "[NetworkServer] Invalid JSON during login: " << e.what() << std::endl;
        }
        
        return; // Stop processing further until authenticated
    }

    // If we reach here, the client is fully authenticated.
    // Route the gameplay messages (e.g., MOVE) to the game controller.
    PlayerRole role = m_gameSession.getRole(hdl);
    bool success = m_controller.processCommand(payload, role);
    
    if (!success) {
        std::cout << "[NetworkServer] Invalid command or unauthorized role." << std::endl;
    }
}