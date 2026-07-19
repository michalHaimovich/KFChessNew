#pragma once
#include <string>
#include "model/game_state.hpp"

class BoardPrinter {
public:
    // Generates a text representation of the current board state.
    static std::string print(const GameState& state);
};