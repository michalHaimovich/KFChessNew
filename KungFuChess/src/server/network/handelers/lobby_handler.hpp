#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "game/room_manager.hpp"
#include "network/matchmaker.hpp"
#include "network/client_session.hpp"

class LobbyHandler {
private:
    RoomManager& m_roomManager;
    Matchmaker& m_matchmaker;

public:
    LobbyHandler(RoomManager& rm, Matchmaker& mm);
    
    std::string handleMessage(const nlohmann::json& j, ClientSession& client, websocketpp::connection_hdl hdl);
};