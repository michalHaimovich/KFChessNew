#include "view/asset_manager.hpp"

void AssetManager::loadAllAssets(const std::string& basePath, int targetSize) {
    std::vector<PieceKind> kinds = {PieceKind::Pawn, PieceKind::Knight, PieceKind::Bishop, PieceKind::Rook, PieceKind::Queen, PieceKind::King};
    std::vector<PieceColor> colors = {PieceColor::White, PieceColor::Black};
    std::vector<PieceState> states = {PieceState::Idle, PieceState::Move, PieceState::Jump, PieceState::ShortRest, PieceState::LongRest};

    for (auto color : colors) {
        for (auto kind : kinds) {
            std::string prefix = AssetLocator::getPiecePrefix(kind, color);
            for (auto state : states) {
                std::string path = AssetLocator::getAnimationPath(basePath, prefix, state);
                
                std::string key = prefix + "_" + std::to_string(static_cast<int>(state));
                
                library[key] = std::make_shared<AnimationData>(path, targetSize);
            }
        }
    }
}

std::shared_ptr<AnimationData> AssetManager::getAnimData(PieceKind kind, PieceColor color, PieceState state) const {
    std::string prefix = AssetLocator::getPiecePrefix(kind, color);
    std::string key = prefix + "_" + std::to_string(static_cast<int>(state));
    
    auto it = library.find(key);
    if (it != library.end()) {
        return it->second;
    }
    return nullptr; 
}