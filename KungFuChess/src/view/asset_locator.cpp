#include "view/asset_locator.hpp"

std::string AssetLocator::getPiecePrefix(PieceKind kind, PieceColor color) {
    std::string prefix = "";
    switch (kind) {
        case PieceKind::Pawn:   prefix = "P"; break;
        case PieceKind::Knight: prefix = "N"; break;
        case PieceKind::Bishop: prefix = "B"; break;
        case PieceKind::Rook:   prefix = "R"; break;
        case PieceKind::Queen:  prefix = "Q"; break;
        case PieceKind::King:   prefix = "K"; break;
    }
    prefix += (color == PieceColor::White) ? "W" : "B";
    return prefix;
}

std::string AssetLocator::getAnimationPath(const std::string& basePath, const std::string& prefix, PieceState state) {
    std::string stateStr = "";
    switch (state) {
        case PieceState::Idle:      stateStr = "idle"; break;
        case PieceState::Move:      stateStr = "move"; break;
        case PieceState::Jump:      stateStr = "jump"; break;
        case PieceState::ShortRest: stateStr = "short_rest"; break;
        case PieceState::LongRest:  stateStr = "long_rest"; break;
    }
    return basePath + "/" + prefix + "/states/" + stateStr;
}