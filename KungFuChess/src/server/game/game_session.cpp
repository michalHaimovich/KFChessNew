#include "game_session.hpp"
#include <iostream>

GameSession::GameSession() : engine(8, 8), whiteTaken(false), blackTaken(false) {
    engine.setupStandardBoard(); 
}

PlayerRole GameSession::addClient(websocketpp::connection_hdl hdl) {
    PlayerRole assignedRole;
    
    if (!whiteTaken) {
        assignedRole = PlayerRole::White;
        whiteTaken = true;
        std::cout << "[GameSession] Client joined - Assigned as White Player." << std::endl;
    } 
    else if (!blackTaken) {
        assignedRole = PlayerRole::Black;
        blackTaken = true;
        std::cout << "[GameSession] Client joined - Assigned as Black Player." << std::endl;
    } 
    else {
        assignedRole = PlayerRole::Spectator;
        std::cout << "[GameSession] Client joined - Assigned as Spectator." << std::endl;
    }
    
    players[hdl] = assignedRole;
    return assignedRole;
}

void GameSession::removeClient(websocketpp::connection_hdl hdl) {
    auto it = players.find(hdl);
    if (it != players.end()) {
        if (it->second == PlayerRole::White) {
            whiteTaken = false;
            std::cout << "[GameSession] White Player disconnected. Slot is open." << std::endl;
        } else if (it->second == PlayerRole::Black) {
            blackTaken = false;
            std::cout << "[GameSession] Black Player disconnected. Slot is open." << std::endl;
        } else {
            std::cout << "[GameSession] Spectator disconnected." << std::endl;
        }
        players.erase(it);
    }
}

PlayerRole GameSession::getRole(websocketpp::connection_hdl hdl) const {
    auto it = players.find(hdl);
    if (it != players.end()) {
        return it->second;
    }
    return PlayerRole::Spectator; //defult
}

GameEngine& GameSession::getEngine() {
    return engine;
}