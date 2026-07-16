#pragma once
#include "model/piece.hpp"
#include "model/position.hpp"

class GameObserver {
public:
    virtual ~GameObserver() = default;

    virtual void onMoveCompleted(const Piece& piece, Position source, Position dest, long timeMs) = 0;

    virtual void onPieceCaptured(const Piece& capturedPiece) = 0;
};