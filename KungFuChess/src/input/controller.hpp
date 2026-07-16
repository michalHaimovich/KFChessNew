#pragma once
#include <optional>
#include "model/position.hpp"
#include "engine/game_engine.hpp"
#include "board_mapper.hpp"

enum class ControllerResult {
    Ignored,
    PieceSelected,
    SelectionCleared,
    MoveRequested,
    JumpRequested
};

class Controller {
private:
    GameEngine& engine;
    std::optional<Position> selectedCell;
    BoardMapper mapper;

public:
    // Stores the engine dependency.
    Controller(GameEngine& eng, const BoardMapper& map);
    // Handles a user click and returns the outcome.
    ControllerResult click(int x, int y);

    // Handles a jump request from the UI.
    ControllerResult jump(int x, int y);

    // Returns the currently selected cell, if any.
    std::optional<Position> getSelectedCell() const;
};