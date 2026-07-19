#include "model/position.hpp"

bool Position::operator==(const Position& other) const {
    // Two positions are equal if both their row and col match exactly
    return row == other.row && col == other.col;
}

bool Position::operator!=(const Position& other) const {
    // Reuse the equality operator to determine inequality
    return !(*this == other);
}



std::ostream& operator<<(std::ostream& os, const Position& pos) {
    // Format the position for readable test output and debugging
    os << "Position(row: " << pos.row << ", col: " << pos.col << ")";
    return os;
}