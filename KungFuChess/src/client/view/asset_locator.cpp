#include "view/asset_locator.hpp"
#include <map>

std::string AssetLocator::getPiecePrefix(PieceKind kind, PieceColor color) {
    static const std::map<PieceKind, std::string> kindPrefixes = {
        {PieceKind::Pawn, "P"}, {PieceKind::Knight, "N"}, 
        {PieceKind::Bishop, "B"}, {PieceKind::Rook, "R"}, 
        {PieceKind::Queen, "Q"}, {PieceKind::King, "K"}
    };

    std::string prefix = kindPrefixes.at(kind);
    prefix += (color == PieceColor::White) ? "W" : "B";
    return prefix;
}

std::string AssetLocator::getAnimationPath(const std::string& basePath, const std::string& prefix, PieceState state) {
    static const std::map<PieceState, std::string> stateNames = {
        {PieceState::Idle, "idle"}, {PieceState::Move, "move"}, 
        {PieceState::Jump, "jump"}, {PieceState::ShortRest, "short_rest"}, 
        {PieceState::LongRest, "long_rest"}
    };

    return basePath + "/" + prefix + "/states/" + stateNames.at(state);
}