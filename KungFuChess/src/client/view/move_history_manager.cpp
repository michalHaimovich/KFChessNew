#include "move_history_manager.hpp"
#include <iomanip>
#include <sstream>

MoveHistoryManager::MoveHistoryManager(EventBus* bus) : pendingCapture(false) {
    if (bus) {
        bus->subscribe<MoveCompletedEvent>([this](const MoveCompletedEvent& event) {
            std::string timeStr = this->formatTime(event.timeMs);
            std::string moveStr = this->getPieceChar(event.piece.kind) + this->getCoord(event.source);

            if (event.destinationCapture) {
                moveStr += " x " + this->getCoord(event.dest);
            } else {
                moveStr += " -> " + this->getCoord(event.dest);
            }

            if (event.piece.color == PieceColor::White) {
                this->whiteMoves.push_back({timeStr, moveStr});
            } else {
                this->blackMoves.push_back({timeStr, moveStr});
            }
        });
    }
}
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
