#include "score_manager.hpp"

#include "score_manager.hpp"

ScoreManager::ScoreManager(EventBus* bus) : whiteScore(0), blackScore(0) {
    if (bus) {
        // Subscribe to the PieceCapturedEvent using a lambda function
        bus->subscribe<PieceCapturedEvent>([this](const PieceCapturedEvent& event) {
            int value = this->getPieceValue(event.capturedPiece.kind);
            
            if (event.capturedPiece.color == PieceColor::Black) {
                this->whiteScore += value; 
            } else {
                this->blackScore += value;
            }
        });
    }
}

int ScoreManager::getPieceValue(PieceKind kind) const {
    switch (kind) {
        case PieceKind::Pawn:   return 1;
        case PieceKind::Knight: return 3;
        case PieceKind::Bishop: return 3;
        case PieceKind::Rook:   return 5;
        case PieceKind::Queen:  return 9;
        case PieceKind::King:   return 0; 
        default: return 0;
    }
}
