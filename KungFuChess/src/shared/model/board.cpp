#include "model/board.hpp"

Board::Board(int w, int h) : width(w), height(h) {
    // Initialize the grid with empty optionals
    grid.resize(width * height, std::nullopt);
}

int Board::getIndex(Position pos) const {
    return pos.row * width + pos.col;
}

int Board::getWidth() const { 
    return width; 
}

int Board::getHeight() const { 
    return height; 
}

bool Board::isInside(Position pos) const {
    return pos.row >= 0 && pos.row < height &&
           pos.col >= 0 && pos.col < width;
}

bool Board::addPiece(const Piece& piece) {
    if (!isInside(piece.cell)) {
        return false;
    }
    
    int idx = getIndex(piece.cell);
    if (grid[idx].has_value()) {
        // Reject duplicate occupancy
        return false; 
    }
    
    grid[idx] = piece;
    return true;
}

bool Board::removePiece(Position pos) {
    if (!isInside(pos)) {
        return false;
    }
    
    int idx = getIndex(pos);
    if (!grid[idx].has_value()) {
        return false;
    }
    
    grid[idx] = std::nullopt;
    return true;
}

std::optional<Piece> Board::getPiece(Position pos) const {
    if (!isInside(pos)) {
        return std::nullopt;
    }
    return grid[getIndex(pos)];
}

void Board::movePiece(Position from, Position to) {
    if (!isInside(from) || !isInside(to)) {
        return;
    }
    
    int fromIdx = getIndex(from);
    int toIdx = getIndex(to);
    
    if (grid[fromIdx].has_value()) {
        // Copy the piece, update its internal cell, and move it
        Piece movingPiece = grid[fromIdx].value();
        movingPiece.cell = to; 
        
        grid[toIdx] = movingPiece;
        grid[fromIdx] = std::nullopt;
    }
}