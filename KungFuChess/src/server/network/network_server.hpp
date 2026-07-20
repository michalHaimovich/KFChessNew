#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <map>
#include <string>

#include "game/game_session.hpp"
#include "game/server_controller.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

// NEW: Client session state to manage authentication before joining a game
struct ClientSession {
    bool isAuthenticated = false;
    std::string username = "";
};

class NetworkServer {
private:
    server m_server;
    
    GameSession m_gameSession; 
    ServerController m_controller; 

    // NEW: Map to keep track of connected clients and their authentication state
    std::map<websocketpp::connection_hdl, ClientSession, std::owner_less<websocketpp::connection_hdl>> connectedClients;

    void sendToClient(websocketpp::connection_hdl hdl, const std::string& message);

public:
    NetworkServer();
    void run(uint16_t port);

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
};