#pragma once
#include <vector>
#include <string>
#include <optional>
#include <set>
#include "model/game_state.hpp"
#include "rules/rule_engine.hpp"
#include "realtime/real_time_arbiter.hpp"
#include "model/game_observer.hpp" 

struct GameSnapshot {
    int boardWidth;
    int boardHeight;
    std::vector<Piece> stationaryPieces;
    std::vector<Motion> activeMotions;
    bool isGameOver;
    std::optional<Piece> selectedPiece; 
    std::set<Position> highlightedCells; 
    std::optional<PieceColor> winner;

    // Checks whether a coordinate is inside the board bounds.
    bool isInside(Position p) const {
        return (p.row >= 0 && p.row < boardHeight &&
                p.col >= 0 && p.col < boardWidth);
    }

    // Retrieves the piece occupying a cell, if any.
    std::optional<Piece> getPieceAt(Position p) const {
        for (const auto& piece : stationaryPieces) {
            if (piece.cell.row == p.row && piece.cell.col == p.col) {
                return piece;
            }
        }
        return std::nullopt;
    }
};

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