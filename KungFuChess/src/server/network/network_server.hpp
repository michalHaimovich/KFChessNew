#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <map>
#include <string>
#include <memory>
#include <chrono>

#include "db/database_connection.hpp"       
#include "db/repositories/user_repository.hpp"
#include "game/room_manager.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

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
    std::chrono::steady_clock::time_point queueStartTime;
};

class NetworkServer {
private:
    server m_server;
    
    std::unique_ptr<RoomManager> m_roomManager;

    std::map<websocketpp::connection_hdl, ClientSession, std::owner_less<websocketpp::connection_hdl>> connectedClients;

    std::unique_ptr<DatabaseConnection> m_db;
    std::unique_ptr<UserRepository> m_userRepo;

    void start_timer();

    void sendToClient(websocketpp::connection_hdl hdl, const std::string& message);

public:
    NetworkServer();
    void run(uint16_t port);

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
};