#pragma once

#include "piece.hpp"
#include <optional>
#include <set>
#include <vector>
#include "motion.hpp"

struct GameSnapshot {

    long serverTime = 0;

    int boardWidth;
    int boardHeight;
    
    std::string whitePlayerName = "Waiting...";
    std::string blackPlayerName = "Waiting...";

    std::vector<Piece> stationaryPieces;
    std::vector<Motion> activeMotions;
    
    bool isGameOver;
    
    std::optional<Piece> selectedPiece; 
    std::set<Position> highlightedCells; 
    std::optional<PieceColor> winner;

    // Checks whether a coordinate is inside the board bounds.
    bool isInside(Position p) const {
        return (p.row >= 0 && p.row < boardHeight &&
                p.col >= 0 && p.col < boardWidth);
    }

    // Retrieves the piece occupying a cell, if any.
    std::optional<Piece> getPieceAt(Position p) const {
        for (const auto& piece : stationaryPieces) {
            if (piece.cell.row == p.row && piece.cell.col == p.col) {
                return piece;
            }
        }
        return std::nullopt;
    }
};