#pragma once
#include "model/game_observer.hpp"

class ScoreManager : public GameObserver {
private:
    int whiteScore;
    int blackScore;

    int getPieceValue(PieceKind kind) const;

public:
    ScoreManager() : whiteScore(0), blackScore(0) {}

    void onPieceCaptured(const Piece& capturedPiece) override;
    void onMoveCompleted(const Piece& piece, Position source, Position dest, bool destinationCapture, long timeMs) override;

    int getWhiteScore() const { return whiteScore; }
    int getBlackScore() const { return blackScore; }
};