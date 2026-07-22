#include "input/controller.hpp"
#include <cmath>
#include <algorithm>

Controller::Controller(NetworkClient &net, const GameSnapshot &snapshot, BoardMapper &map)
    : network(net), localSnapshot(snapshot), mapper(map), selectedCell(std::nullopt) {}

std::optional<Position> Controller::getSelectedCell() const
{
    return selectedCell;
}

ControllerResult Controller::click(int x, int y)
{

    if (localSnapshot.isGameOver)
    {
        int boardW = mapper.getCellSize() * Theme::BoardCells;
        int boardH = boardW;

        int btnX = mapper.getStartX() + (boardW - Theme::BtnWidth) / 2;
        int btnY = mapper.getStartY() + (boardH) / 2 + Theme::BtnOffsetY;

        if (x >= btnX && x <= btnX + Theme::BtnWidth &&
            y >= btnY && y <= btnY + Theme::BtnHeight)
        {

            wantsReturn = true;
            return ControllerResult::ReturnToLobby;
        }
        return ControllerResult::Ignored;
    }

    auto mappedCellOpt = mapper.pixelToCell(x, y);

    if (!mappedCellOpt.has_value())
    {
        return ControllerResult::Ignored;
    }
    Position p = mappedCellOpt.value();

    if (!localSnapshot.isInside(p))
    {
        if (selectedCell.has_value())
        {
            selectedCell = std::nullopt;
            return ControllerResult::SelectionCleared;
        }
        return ControllerResult::Ignored;
    }

    auto clickedPiece = localSnapshot.getPieceAt(p);

    if (!selectedCell.has_value())
    {
        if (clickedPiece.has_value() && clickedPiece->state == PieceState::Idle)
        {
            selectedCell = p;
            return ControllerResult::PieceSelected;
        }
        return ControllerResult::Ignored;
    }
    else
    {
        auto selectedPiece = localSnapshot.getPieceAt(selectedCell.value());

        if (clickedPiece.has_value() && selectedPiece.has_value() &&
            clickedPiece->color == selectedPiece->color)
        {

            if (clickedPiece->state == PieceState::Idle)
            {
                selectedCell = p;
                return ControllerResult::PieceSelected;
            }
            else
            {
                return ControllerResult::Ignored;
            }
        }

        network.sendMove(selectedCell.value().col, selectedCell.value().row, p.col, p.row);

        selectedCell = std::nullopt;
        return ControllerResult::MoveRequested;
    }
}

ControllerResult Controller::jump(int x, int y)
{
    auto mappedCellOpt = mapper.pixelToCell(x, y);
    if (!mappedCellOpt.has_value())
    {
        return ControllerResult::Ignored;
    }

    network.sendJump(mappedCellOpt.value().col, mappedCellOpt.value().row);
    return ControllerResult::JumpRequested;
}
