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
    // מילון ששומר את כל הקלטות בזיכרון. המפתח יהיה סטרינג שמשלב סוג, צבע ומצב (למשל "PW_0")
    std::map<std::string, std::shared_ptr<AnimationData>> library;

public:
    // טוען הכל בתחילת המשחק
    void loadAllAssets(const std::string& basePath, int targetSize);
    
    // שולף קלטת ספציפית ללא עותק זיכרון (O(1) או קרוב לזה)
    std::shared_ptr<AnimationData> getAnimData(PieceKind kind, PieceColor color, PieceState state) const;
};