#pragma once
#include <vector>
#include <optional>
#include "model/position.hpp"
#include "model/piece.hpp"

class Board {
private:
    int width;
    int height;

    std::vector<std::optional<Piece>> grid;

    // Converts a 2D position into a 1D vector index.
    int getIndex(Position pos) const;

public:
    // Constructs a board with the given dimensions.
    Board(int w, int h);

    // Returns the board width.
    int getWidth() const;

    // Returns the board height.
    int getHeight() const;

    // Checks whether a given cell is inside the board bounds.
    bool isInside(Position pos) const;

    // Adds a piece to the board.
    bool addPiece(const Piece& piece);

    // Removes the piece at the given position.
    bool removePiece(Position pos);

    // Retrieves the piece at the given position.
    std::optional<Piece> getPiece(Position pos) const;

    // Moves a piece between two positions.
    void movePiece(Position from, Position to);
};