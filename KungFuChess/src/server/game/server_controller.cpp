#include "server_controller.hpp"
#include <sstream>

int time_per_sqere = 1000;

ServerController::ServerController(GameEngine& eng) : engine(eng) {}

bool ServerController::processCommand(const std::string& command, PlayerRole role) {
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
            
            if (pieceOpt.has_value()) {
                PieceColor pieceColor = pieceOpt.value().color;
                
                if ((role == PlayerRole::White && pieceColor == PieceColor::White) ||
                    (role == PlayerRole::Black && pieceColor == PieceColor::Black)) {
                    
                    int rowDiff = std::abs(r1 - r2);
                    int colDiff = std::abs(c1 - c2);
                    int distance = std::max(rowDiff, colDiff);
                    
                    long durationMs = std::max(1, distance) * time_per_sqere;
              
                    return engine.requestMove({r1, c1}, {r2, c2}, durationMs);
                }
            }
        }
    }
    else if (action == "JUMP") {
        int r, c;
        if (ss >> r >> c) {
            auto snapshot = engine.getSnapshot(); 
            auto pieceOpt = snapshot.getPieceAt({r, c});
            
            if (pieceOpt.has_value()) {
                PieceColor pieceColor = pieceOpt.value().color;
                
                if ((role == PlayerRole::White && pieceColor == PieceColor::White) ||
                    (role == PlayerRole::Black && pieceColor == PieceColor::Black)) {
                    
                    return engine.requestJump({r, c});
                }
            }
        }
    }

    return false; 
}