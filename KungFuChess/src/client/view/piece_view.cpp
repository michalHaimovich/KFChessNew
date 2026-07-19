#include "view/piece_view.hpp"

PieceView::PieceView(int id) : pieceId(id) {}

void PieceView::syncWithModel(const Piece& modelPiece, const AssetManager& assets, long dt) {
    auto animData = assets.getAnimData(modelPiece.kind, modelPiece.color, modelPiece.state);
    
    player.play(animData);
    
    player.update(dt);
}

const Img& PieceView::getFrame() const {
    return player.getCurrentFrame();
}