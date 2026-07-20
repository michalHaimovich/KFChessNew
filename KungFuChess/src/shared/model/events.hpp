#pragma once
#include "model/piece.hpp"
#include "model/position.hpp"

// Base class for all events
struct Event {
    virtual ~Event() = default;
};

// Event fired when a piece is captured
struct PieceCapturedEvent : public Event {
    Piece capturedPiece;
};

// Event fired when a piece successfully lands on a destination
struct MoveCompletedEvent : public Event {
    Piece piece;
    Position source;
    Position dest;
    bool destinationCapture;
    long timeMs;
};