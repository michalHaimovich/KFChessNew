#pragma once
#include <string>
#include <cmath>

#include "../../engine/game_engine.hpp"
#include "../../game/game_session.hpp" 

class GameActionHandler {
private:
    GameEngine& engine;
    
    bool isPlayerAllowed(PlayerRole role, PieceColor pieceColor) const;

public:
    GameActionHandler(GameEngine& eng);

    bool processCommand(const std::string& command, PlayerRole role);
};