#pragma once
#include "position.hpp"

// Forward compatibility: These states match the graphic assets perfectly
enum class PieceState {
    Idle,
    Move,
    Jump,
    ShortRest,
    LongRest
};

enum class PieceColor { White, Black };
enum class PieceKind { Pawn, Knight, Bishop, Rook, Queen, King };

struct Piece {
    int id;
    PieceColor color;
    PieceKind kind;
    Position cell;
    PieceState state;
    
    long readyTime; 
    
    Piece(int id, PieceColor color, PieceKind kind, Position cell) 
        : id(id), color(color), kind(kind), cell(cell), state(PieceState::Idle), readyTime(0) {}
};