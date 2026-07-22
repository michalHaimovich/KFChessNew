#pragma once
#include <memory>
#include <vector>
#include <set>
#include <optional>
#include <cmath>

#include "engine/game_engine.hpp"
#include "rules/rule_engine.hpp"
#include "bot_config.hpp"

class KungFuBot {
private:
    PieceColor m_color;
    BotConfig m_config;
    
    // References to the game components
    GameEngine& m_engine;
    RuleEngine& m_ruleEngine;

    // State tracking
    long m_lastActionTimeMs;

    // --- Internal Evaluation Helpers ---
    
    // Calculates Manhattan distance between two positions
    int getDistance(Position a, Position b) const;

    // Checks if a position is under immediate threat from active motions
    bool isUnderThreat(const GameSnapshot& snap, Position pos, long currentTimeMs, long& outThreatArrivalTime) const;

    // Evaluates a destination position and returns a score
    int evaluateMove(const GameSnapshot& snap, const Piece& movingPiece, Position dest) const;

public:
    KungFuBot(PieceColor color, int elo, GameEngine& engine, RuleEngine& ruleEngine);

    // The main function to be called from the game loop
    void update(long currentTimeMs);
};