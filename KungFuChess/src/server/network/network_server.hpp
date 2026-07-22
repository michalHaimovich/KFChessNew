#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <map>
#include <string>
#include <memory>
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>


#include "handelers/auth_handler.hpp"
#include "handelers/lobby_handler.hpp"
#include "handelers/game_action_handler.hpp"
#include "matchmaker.hpp"
#include "db/database_connection.hpp"       
#include "db/repositories/user_repository.hpp"
#include "game/room_manager.hpp"
#include "client_session.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

class NetworkServer {
private:
    server m_server;
    
    std::unique_ptr<RoomManager> m_roomManager;

    std::map<websocketpp::connection_hdl, ClientSession, std::owner_less<websocketpp::connection_hdl>> connectedClients;

    std::unique_ptr<DatabaseConnection> m_db;
    std::unique_ptr<UserRepository> m_userRepo;

    std::unique_ptr<Matchmaker> m_matchmaker;
    std::unique_ptr<AuthHandler> m_authHandler;
    std::unique_ptr<LobbyHandler> m_lobbyHandler;

    void start_timer();

    void sendToClient(websocketpp::connection_hdl hdl, const std::string& message);

    std::optional<json> parseJsonSafe(const std::string& payload);

    void onMatchFound(websocketpp::connection_hdl hdl1, websocketpp::connection_hdl hdl2);
    void onMatchTimeout(websocketpp::connection_hdl hdl);

public:
    NetworkServer();
    void run(uint16_t port);

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
};