#pragma once
#include <optional>
#include "model/position.hpp"

class BoardMapper {
private:
    int startX, startY, cellSize;
public:
    // בנאי שמקבל את המידות האמיתיות של הלוח כפי שחושבו ב-Layout
    BoardMapper(int x, int y, int size) : startX(x), startY(y), cellSize(size) {}

    std::optional<Position> pixelToCell(int x, int y) const {
        if (x < startX || y < startY) return std::nullopt;
        int col = (x - startX) / cellSize;
        int row = (y - startY) / cellSize;
        return Position{row, col};
    }
};