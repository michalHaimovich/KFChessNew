#pragma once
#include "piece.hpp"
#include "position.hpp"
#include <cmath>

enum class MotionType { Normal, Jump };

struct Motion {
    Piece piece;
    Position source;
    Position destination;
    long startTime;
    long arrivalTime;
    MotionType type = MotionType::Normal;

    // Marks whether this motion has been cancelled mid-air.
    bool isDead = false;

    // Returns the piece's current cell for a given time.
    Position getCurrentCell(long currentTime) const {
        if (type == MotionType::Jump) return source;
        if (currentTime >= arrivalTime) return destination;
        if (currentTime <= startTime) return source;

        double progress = static_cast<double>(currentTime - startTime) / (arrivalTime - startTime);
        int cur_r = source.row + std::round((destination.row - source.row) * progress);
        int cur_c = source.col + std::round((destination.col - source.col) * progress);

        return Position{cur_r, cur_c};
    }

    // Orders motions by arrival time.
    bool operator>(const Motion& other) const {
        return arrivalTime > other.arrivalTime;
    }
};