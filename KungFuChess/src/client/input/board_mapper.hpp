#pragma once
#include <optional>
#include "model/position.hpp"

class BoardMapper {
private:
    int startX, startY, cellSize;
public:
    BoardMapper(int x, int y, int size) : startX(x), startY(y), cellSize(size) {}

    std::optional<Position> pixelToCell(int x, int y) const {
        if (x < startX || y < startY) return std::nullopt;
        int col = (x - startX) / cellSize;
        int row = (y - startY) / cellSize;
        return Position{row, col};
    }

    void updateLayout(int x, int y, int size) {
    startX = x;
    startY = y;
    cellSize = size;
    }
};