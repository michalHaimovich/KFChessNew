#pragma once
#include <ostream>

struct Position {
    int row;
    int col;

    // Compares two positions for equality.
    bool operator==(const Position& other) const;

    // Compares two positions for inequality.
    bool operator!=(const Position& other) const;

    // Orders positions lexicographically.
    bool operator<(const Position& other) const {
        if (row != other.row) return row < other.row;
        return col < other.col;
    }
};

// Writes a position to an output stream.
std::ostream& operator<<(std::ostream& os, const Position& pos);

