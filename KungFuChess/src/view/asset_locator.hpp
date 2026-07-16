#pragma once
#include <string>
#include "model/piece.hpp"

class AssetLocator {
public:
    static std::string getPiecePrefix(PieceKind kind, PieceColor color);

    static std::string getAnimationPath(const std::string& basePath, 
                                        const std::string& prefix, 
                                        PieceState state);
};