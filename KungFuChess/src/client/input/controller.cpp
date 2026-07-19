#include "input/controller.hpp"
#include <cmath>
#include <algorithm>

Controller::Controller(NetworkClient& net, const GameSnapshot& snapshot, const BoardMapper& map) 
    : network(net), localSnapshot(snapshot), mapper(map), selectedCell(std::nullopt) {}
    
std::optional<Position> Controller::getSelectedCell() const {
    return selectedCell;
}

ControllerResult Controller::click(int x, int y) {
    auto mappedCellOpt = mapper.pixelToCell(x, y); 

    if (!mappedCellOpt.has_value()) {
        return ControllerResult::Ignored;
    }
    Position p = mappedCellOpt.value();
    
    // בדיקה מול הלוח המקומי שיש ללקוח באותו רגע
    if (!localSnapshot.isInside(p)) {
        if (selectedCell.has_value()) {
            selectedCell = std::nullopt; 
            return ControllerResult::SelectionCleared;
        }
        return ControllerResult::Ignored;
    }

    auto clickedPiece = localSnapshot.getPieceAt(p);

    if (!selectedCell.has_value()) {
        if (clickedPiece.has_value() && clickedPiece->state == PieceState::Idle) {
            selectedCell = p; 
            return ControllerResult::PieceSelected;
        }
        return ControllerResult::Ignored; 
    } 
    else {
        auto selectedPiece = localSnapshot.getPieceAt(selectedCell.value());

        if (clickedPiece.has_value() && selectedPiece.has_value() && 
            clickedPiece->color == selectedPiece->color) {
            
            if (clickedPiece->state == PieceState::Idle) {
                selectedCell = p;
                return ControllerResult::PieceSelected;
            } else {
                return ControllerResult::Ignored;
            }
        }

        // שינוי קריטי: במקום מנוע, הלקוח פשוט שולח בקשה לשרת!
        network.sendMove(selectedCell.value().col, selectedCell.value().row, p.col, p.row);
        
        selectedCell = std::nullopt; 
        return ControllerResult::MoveRequested;
    }
}

ControllerResult Controller::jump(int x, int y) {
    auto mappedCellOpt = mapper.pixelToCell(x, y); 
    if (!mappedCellOpt.has_value()) {
        return ControllerResult::Ignored;
    }

    // שליחת בקשת הקפיצה לשרת
    network.sendJump(mappedCellOpt.value().col, mappedCellOpt.value().row);
    return ControllerResult::JumpRequested;
}