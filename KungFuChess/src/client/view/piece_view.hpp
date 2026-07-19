#pragma once
#include "view/animation_player.hpp"
#include "view/asset_manager.hpp"
#include "model/piece.hpp"

class PieceView {
private:
    int pieceId;
    AnimationPlayer player;

public:
    PieceView(int id = -1);

    void syncWithModel(const Piece& modelPiece, const AssetManager& assets, long dt);

    const Img& getFrame() const;
};