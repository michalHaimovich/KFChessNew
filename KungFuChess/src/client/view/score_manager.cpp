#include "score_manager.hpp"

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

void ScoreManager::onPieceCaptured(const Piece& capturedPiece) {
    int value = getPieceValue(capturedPiece.kind);
    
    if (capturedPiece.color == PieceColor::Black) {
        whiteScore += value; 
    } else {
        blackScore += value;
    }
}

void ScoreManager::onMoveCompleted(const Piece& piece, Position source, Position dest,  bool destinationCapture, long timeMs) {
}