#pragma once
#include "model/piece.hpp"
#include "model/position.hpp"

struct Event {
    virtual ~Event() = default;
};

struct PieceCapturedEvent : public Event {
    
    Piece capturedPiece;

    PieceCapturedEvent(const Piece& p) : capturedPiece(p) {}
};

struct MoveCompletedEvent : public Event {
    Piece piece;
    Position source;
    Position dest;
    bool destinationCapture;
    long timeMs;

    MoveCompletedEvent(const Piece& p, Position src, Position dst, bool capture, long time) 
        : piece(p), source(src), dest(dst), destinationCapture(capture), timeMs(time) {}
};