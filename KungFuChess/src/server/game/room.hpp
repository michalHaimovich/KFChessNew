#pragma once
#include <string>
#include <memory>
#include <chrono>
#include <websocketpp/common/connection_hdl.hpp>

#include "game_session.hpp" 

class Room {
private:
    std::string m_roomName;

    PlayerRole m_disconnectedRole;
    
    std::shared_ptr<GameSession> m_gameSession;

    bool m_isGracePeriodActive;
    std::chrono::steady_clock::time_point m_disconnectTime;
    websocketpp::connection_hdl m_disconnectedPlayer;

public:
    Room(const std::string& name, std::function<void(websocketpp::connection_hdl, const std::string&)> sendCb);
    ~Room() = default;

    std::string getName() const;
    std::shared_ptr<GameSession> getSession();
    PlayerRole addPlayer(websocketpp::connection_hdl hdl,  const std::string& username);
    void removePlayer(websocketpp::connection_hdl hdl);

    void startGracePeriod(websocketpp::connection_hdl hdl, PlayerRole role); 
    void handleAutoResign();
    void cancelGracePeriod();
    
    bool checkAutoResign(); 
};