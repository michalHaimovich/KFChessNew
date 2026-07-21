#include "lobby_handler.hpp"

LobbyHandler::LobbyHandler(RoomManager& rm, Matchmaker& mm) 
    : m_roomManager(rm), m_matchmaker(mm) {}

std::string LobbyHandler::handleMessage(const nlohmann::json& j, ClientSession& client, websocketpp::connection_hdl hdl) {
    std::string action = j.value("action", "");

    if (action == "CREATE_ROOM") {
        std::string roomName = j.value("roomName", "");
        if (m_roomManager.createRoom(roomName)) {
            client.roomId = roomName;
            client.state = ClientState::IN_ROOM;
            
            auto room = m_roomManager.getRoom(roomName);
            room->addPlayer(hdl, client.username);
            return R"({"type": "ROOM_SUCCESS", "message": "Room created"})";
        }
        return R"({"type": "ROOM_ERROR", "message": "Room name already exists"})";
    } 
    else if (action == "JOIN_ROOM") {
        std::string roomName = j.value("roomName", "");
        auto room = m_roomManager.getRoom(roomName);
        if (room) {
            client.roomId = roomName;
            client.state = ClientState::IN_ROOM;
            room->addPlayer(hdl, client.username);
            return R"({"type": "ROOM_SUCCESS", "message": "Joined room"})";
        }
        return R"({"type": "ROOM_ERROR", "message": "Room not found"})";
    } 
    else if (action == "FIND_MATCH") {
        client.state = ClientState::IN_QUEUE;
        client.queueStartTime = std::chrono::steady_clock::now();
        m_matchmaker.addPlayer(hdl, client.username, client.rating);
        return ""; 
    }
    
    return ""; 
}