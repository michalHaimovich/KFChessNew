#pragma once
#include "view/animation_player.hpp"
#include "view/asset_manager.hpp"
#include "model/piece.hpp"

constexpr int INVALID_PIECE_ID = -1;

class PieceView {
private:
    int pieceId;
    AnimationPlayer player;

public:
    PieceView(int id = INVALID_PIECE_ID);

    void syncWithModel(const Piece& modelPiece, const AssetManager& assets, long dt);

    const Img& getFrame() const;
};