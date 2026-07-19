#include "move_history_manager.hpp"
#include <iomanip>
#include <sstream>

std::string MoveHistoryManager::formatTime(long ms) const {
    long totalSeconds = ms / 1000;
    long minutes = totalSeconds / 60;
    long seconds = totalSeconds % 60;
    long millis = ms % 1000;
    
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << minutes << ":"
       << std::setfill('0') << std::setw(2) << seconds << "."
       << std::setfill('0') << std::setw(3) << millis;
    return ss.str();
}

std::string MoveHistoryManager::getPieceChar(PieceKind kind) const {
    switch (kind) {
        case PieceKind::Knight: return "N";
        case PieceKind::Bishop: return "B";
        case PieceKind::Rook:   return "R";
        case PieceKind::Queen:  return "Q";
        case PieceKind::King:   return "K";
        default: return "";
    }
}

std::string MoveHistoryManager::getCoord(Position pos) const {
    char file = 'a' + pos.col;      
    char rank = '8' - pos.row;    
    return std::string(1, file) + std::string(1, rank);
}

void MoveHistoryManager::onPieceCaptured(const Piece& capturedPiece) {}

void MoveHistoryManager::onMoveCompleted(const Piece& piece, Position source, Position dest, bool destinationCapture, long timeMs) {
    std::string timeStr = formatTime(timeMs);
    std::string moveStr = getPieceChar(piece.kind) + getCoord(source);

    if (destinationCapture) {
        moveStr += " x " + getCoord(dest);
    } else {
        moveStr += " -> " + getCoord(dest);
    }

    if (piece.color == PieceColor::White) {
        whiteMoves.push_back({timeStr, moveStr});
    } else {
        blackMoves.push_back({timeStr, moveStr});
    }
}