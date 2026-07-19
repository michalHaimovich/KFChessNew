#pragma once
#include <optional>
#include "model/position.hpp"
#include "model/game_snapshot.hpp" 
#include "network/network_client.hpp"
#include "input/board_mapper.hpp"

enum class ControllerResult {
    Ignored,
    PieceSelected,
    SelectionCleared,
    MoveRequested,
    JumpRequested
};

class Controller {
private:
    NetworkClient& network;
    const GameSnapshot& localSnapshot;
    std::optional<Position> selectedCell;
    BoardMapper mapper;

public:
    Controller(NetworkClient& net, const GameSnapshot& snapshot, const BoardMapper& map);
    
    ControllerResult click(int x, int y);
    ControllerResult jump(int x, int y);
    std::optional<Position> getSelectedCell() const;
};