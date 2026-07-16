#pragma once
#include <map>
#include <memory>
#include "model/board.hpp"
#include "model/piece.hpp"
#include "model/position.hpp"
#include "rules/piece_rules.hpp"

class RuleEngine {
private:
    std::map<PieceKind, std::unique_ptr<PieceRule>> rulesRegistry;

public:
    // Constructs the rule engine with the default registry.
    RuleEngine();

    // Applies arrival rules such as promotion handling.
    void processArrival(const Board& board, Piece& piece) const;

    // Returns whether death of a piece kind is fatal.
    bool isFatalDeath(PieceKind kind) const;

    // Validates whether a move is legal.
    bool validateMove(const Board& board, const Piece& piece, Position destination) const;

    long getCooldownMs(PieceKind kind, PieceState state) const;
};