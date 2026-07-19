#pragma once
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <websocketpp/common/connection_hdl.hpp>
#include "engine/game_engine.hpp"

enum class PlayerRole {
    White,
    Black,
    Spectator
};

class GameSession {
private:
    GameEngine engine; 
    
    std::map<websocketpp::connection_hdl, PlayerRole, std::owner_less<websocketpp::connection_hdl>> players;
    
    bool whiteTaken;
    bool blackTaken;

    std::thread timeThread;
    std::atomic<bool> isRunning;

    void gameLoop();
public:
    GameSession();
    ~GameSession();
    
    PlayerRole addClient(websocketpp::connection_hdl hdl);
    void removeClient(websocketpp::connection_hdl hdl);
    PlayerRole getRole(websocketpp::connection_hdl hdl) const;
    
    GameEngine& getEngine();
};