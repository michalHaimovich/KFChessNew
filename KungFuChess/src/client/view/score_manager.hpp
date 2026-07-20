#pragma once
#include "model/event_bus.hpp" 

class ScoreManager { 
private:
    int whiteScore;
    int blackScore;
    int getPieceValue(PieceKind kind) const;

public:
    // Constructor now takes the EventBus
    ScoreManager(EventBus* bus);

    int getWhiteScore() const { return whiteScore; }
    int getBlackScore() const { return blackScore; }
};