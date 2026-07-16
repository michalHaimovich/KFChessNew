#include "input/controller.hpp"
#include "input/board_mapper.hpp"
#include <cmath>
#include <algorithm>

// Constants to avoid magic numbers
constexpr long MS_PER_CELL = 1000; 

// Generic helper to compute move duration
static long calculateMoveDuration(Position start, Position end) {
    // Chebyshev distance (max difference of rows or columns)
    int distance = std::max(std::abs(start.row - end.row), std::abs(start.col - end.col));
    return distance * MS_PER_CELL;
}

Controller::Controller(GameEngine& eng, const BoardMapper& map) 
    : engine(eng), mapper(map), selectedCell(std::nullopt) {}

    
std::optional<Position> Controller::getSelectedCell() const {
    return selectedCell;
}

ControllerResult Controller::click(int x, int y) {
    auto mappedCellOpt = mapper.pixelToCell(x, y); 

    if (!mappedCellOpt.has_value()) {
        return ControllerResult::Ignored;
    }
    Position p = mappedCellOpt.value();
    
    GameSnapshot snap = engine.getSnapshot();

    // clean boundary check against the snapshot
    if (!snap.isInside(p)) {
        if (selectedCell.has_value()) {
            selectedCell = std::nullopt; 
            return ControllerResult::SelectionCleared;
        }
        return ControllerResult::Ignored;
    }

    // retrieve the piece at the clicked location
    auto clickedPiece = snap.getPieceAt(p);

    if (!selectedCell.has_value()) {
        if (clickedPiece.has_value()) {
            selectedCell = p; 
            return ControllerResult::PieceSelected;
        }
        return ControllerResult::Ignored; 
    } 
    else {
        auto selectedPiece = snap.getPieceAt(selectedCell.value());

        // switch selection to a friendly piece
        if (clickedPiece.has_value() && selectedPiece.has_value() && 
            clickedPiece->color == selectedPiece->color) {
            selectedCell = p;
            return ControllerResult::PieceSelected;
        }

        // calculate duration generically instead of using a hardcoded 1000
        long duration = calculateMoveDuration(selectedCell.value(), p);
        engine.requestMove(selectedCell.value(), p, duration); 
        
        selectedCell = std::nullopt; 
        
        return ControllerResult::MoveRequested;
    }
}

// route jump through the controller (map pixels then forward to engine)
ControllerResult Controller::jump(int x, int y) {
    
    auto mappedCellOpt = mapper.pixelToCell(x, y); 
    if (!mappedCellOpt.has_value()) {
        return ControllerResult::Ignored;
    }

    // send the request to the engine
    if (engine.requestJump(mappedCellOpt.value())) {
        return ControllerResult::JumpRequested;
    }
    
    return ControllerResult::Ignored;
}