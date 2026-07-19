#pragma once
#include <vector>
#include <string>
#include "model/game_observer.hpp"

struct MoveRecord {
    std::string timeStr;
    std::string moveStr;
};

class MoveHistoryManager : public GameObserver {
private:
    std::vector<MoveRecord> whiteMoves;
    std::vector<MoveRecord> blackMoves;
    bool pendingCapture;

    std::string formatTime(long ms) const;
    std::string getPieceChar(PieceKind kind) const;
    std::string getCoord(Position pos) const;

public:
    MoveHistoryManager() : pendingCapture(false) {}

    void onPieceCaptured(const Piece& capturedPiece) override;
    void onMoveCompleted(const Piece& piece, Position source, Position dest, bool destinationCapture, long timeMs) override;
    const std::vector<MoveRecord>& getWhiteMoves() const { return whiteMoves; }
    const std::vector<MoveRecord>& getBlackMoves() const { return blackMoves; }
};