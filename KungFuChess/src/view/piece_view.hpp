#pragma once
#include "view/animation_player.hpp"
#include "view/asset_manager.hpp"
#include "model/piece.hpp"

class PieceView {
private:
    int pieceId;
    AnimationPlayer player;

public:
    // בנאי ברירת מחדל הנדרש עבור שימוש במילון std::map
    PieceView(int id = -1);

    // מסנכרנת את נגן האנימציה עם המצב הנוכחי של הכלי בלוגיקה
    void syncWithModel(const Piece& modelPiece, const AssetManager& assets, long dt);

    // שולפת את הפריים הסופי לציור
    const Img& getFrame() const;
};