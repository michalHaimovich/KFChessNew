#pragma once
#include <string>
#include <cmath>

#include "engine/game_engine.hpp"
#include "game_session.hpp" 

class ServerController {
private:
    GameEngine& engine;

public:
    ServerController(GameEngine& eng);

    bool processCommand(const std::string& command, PlayerRole role);
};