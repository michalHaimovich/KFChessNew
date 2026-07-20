#pragma once
#include <vector>
#include <string>
#include <optional>
#include <set>
#include <mutex>

#include "model/game_state.hpp"
#include "rules/rule_engine.hpp"
#include "realtime/real_time_arbiter.hpp"
#include "model/game_observer.hpp" 
#include "model/game_snapshot.hpp"



struct MoveResult {
    bool isAccepted;
    std::string reason;
};

class GameEngine {
private:
    GameState currentState;
    RuleEngine ruleEngine;
    RealTimeArbiter arbiter;

    void processArrivals(const std::vector<Motion>& arrivals);
    void resolvePhysicsTick();

    std::vector<GameObserver*> observers;

    mutable std::mutex mtx;
    
    void notifyMoveCompleted(const Piece& piece, Position source, Position dest, bool destinationCapture, long timeMs);    
    void notifyPieceCaptured(const Piece& capturedPiece);

public:
    GameEngine(int width, int height);

    void setupStandardBoard();
    bool requestJump(Position pos);
    GameState& getGameState();
    bool requestMove(Position source, Position destination, long durationMs = 1000);
    void wait(long ms);

    // UPDATED: Now accepts an optional selected position from the controller
    GameSnapshot getSnapshot(std::optional<Position> selectedPos = std::nullopt) const;

    void addObserver(GameObserver* observer) {
        observers.push_back(observer);
    }
};