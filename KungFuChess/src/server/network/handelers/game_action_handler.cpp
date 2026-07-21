#include "game_action_handler.hpp"
#include <sstream>

namespace {
    constexpr int TIME_PER_SQUARE_MS = 1000;
}

GameActionHandler::GameActionHandler(GameEngine& eng) : engine(eng) {}

bool GameActionHandler::isPlayerAllowed(PlayerRole role, PieceColor pieceColor) const {
    return (role == PlayerRole::White && pieceColor == PieceColor::White) ||
           (role == PlayerRole::Black && pieceColor == PieceColor::Black);
}

bool GameActionHandler::processCommand(const std::string& command, PlayerRole role) {
    if (role == PlayerRole::Spectator) {
        return false; 
    }

    std::stringstream ss(command);
    std::string action;
    ss >> action;

    if (action == "MOVE") {
        int r1, c1, r2, c2;
        if (ss >> r1 >> c1 >> r2 >> c2) {
            auto snapshot = engine.getSnapshot(); 
            auto pieceOpt = snapshot.getPieceAt({r1, c1});
            
            if (pieceOpt.has_value() && isPlayerAllowed(role, pieceOpt.value().color)) {
                
                int rowDiff = std::abs(r1 - r2);
                int colDiff = std::abs(c1 - c2);
                int distance = std::max(rowDiff, colDiff);
                
                long durationMs = std::max(1, distance) * TIME_PER_SQUARE_MS;
          
                return engine.requestMove({r1, c1}, {r2, c2}, durationMs);
            }
        }
    }
    else if (action == "JUMP") {
        int r, c;
        if (ss >> r >> c) {
            auto snapshot = engine.getSnapshot(); 
            auto pieceOpt = snapshot.getPieceAt({r, c});
            
            if (pieceOpt.has_value() && isPlayerAllowed(role, pieceOpt.value().color)) {
                
                return engine.requestJump({r, c});
            }
        }
    }

    return false; 
}