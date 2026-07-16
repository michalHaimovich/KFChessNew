#pragma once
#include <map>
#include <string>
#include <memory>
#include <vector>
#include "view/animation_data.hpp"
#include "view/asset_locator.hpp"
#include "model/piece.hpp"

class AssetManager {
private:
    std::map<std::string, std::shared_ptr<AnimationData>> library;

public:
    void loadAllAssets(const std::string& basePath, int targetSize);
    
    std::shared_ptr<AnimationData> getAnimData(PieceKind kind, PieceColor color, PieceState state) const;
};