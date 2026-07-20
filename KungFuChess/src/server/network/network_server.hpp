#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <map>
#include <string>

#include "db/database_connection.hpp"       
#include "db/repositories/user_repository.hpp"
#include "game/game_session.hpp"
#include "game/server_controller.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

// NEW: Client session state to manage authentication before joining a game
enum class ClientState {
    CONNECTED,   
    LOBBY,       
    IN_QUEUE,     
    IN_ROOM       
};

struct ClientSession {
    ClientState state = ClientState::CONNECTED; 
    std::string username = "";
    int rating = 1200;      
    std::string roomId = ""; 
};

class NetworkServer {
private:
    server m_server;
    
    GameSession m_gameSession; 
    ServerController m_controller; 

    std::map<websocketpp::connection_hdl, ClientSession, std::owner_less<websocketpp::connection_hdl>> connectedClients;

    std::unique_ptr<DatabaseConnection> m_db;
    std::unique_ptr<UserRepository> m_userRepo;

    void sendToClient(websocketpp::connection_hdl hdl, const std::string& message);

public:
    NetworkServer();
    void run(uint16_t port);

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
};