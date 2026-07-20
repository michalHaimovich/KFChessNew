#pragma once
#include <vector>
#include <string>
#include "model/event_bus.hpp"

struct MoveRecord {
    std::string timeStr;
    std::string moveStr;
};

class MoveHistoryManager { 
private:
    std::vector<MoveRecord> whiteMoves;
    std::vector<MoveRecord> blackMoves;
    bool pendingCapture;

    std::string formatTime(long ms) const;
    std::string getPieceChar(PieceKind kind) const;
    std::string getCoord(Position pos) const;

public:
    MoveHistoryManager(EventBus* bus);
    const std::vector<MoveRecord>& getWhiteMoves() const { return whiteMoves; }
    const std::vector<MoveRecord>& getBlackMoves() const { return blackMoves; }
};