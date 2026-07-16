#pragma once
#include "model/board.hpp"

struct GameState {
    Board board;
    bool isGameOver;

    // Initializes the board and starts the game as active.
    GameState(int width, int height)
        : board(width, height), isGameOver(false) {}
};