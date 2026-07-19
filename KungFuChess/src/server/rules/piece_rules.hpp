#pragma once
#include <set>
#include "model/board.hpp"
#include "model/piece.hpp"
#include "model/position.hpp"
#include "rules/balance_config.hpp"


class PieceRule {
protected:
    CooldownConfig cooldowns;

public:
    PieceRule(CooldownConfig config) : cooldowns(config) {}

    // Destroys the rule object.
    virtual ~PieceRule() = default;

    // Returns the legal destinations for a piece.
    virtual std::set<Position> legalDestinations(const Board& board, const Piece& piece) const = 0;

    // Handles the effect of a piece landing.
    virtual void onArrival(const Board& board, Piece& piece) const {}

    // Indicates whether this piece's death is fatal.
    virtual bool isFatalDeath() const { return false; }

    virtual long getCooldownMs(PieceState state) const {
        if (state == PieceState::ShortRest) return cooldowns.shortRestMs;
        if (state == PieceState::LongRest) return cooldowns.longRestMs;
        return 0;
    }
};

class RookRule : public PieceRule {
public:
    // Returns the legal destinations for a rook.
    RookRule(CooldownConfig config) : PieceRule(config) {}
    std::set<Position> legalDestinations(const Board& board, const Piece& piece) const override;
};

class BishopRule : public PieceRule {
public:
    BishopRule(CooldownConfig config) : PieceRule(config) {}
    // Returns the legal destinations for a bishop.
    std::set<Position> legalDestinations(const Board& board, const Piece& piece) const override;
};

class QueenRule : public PieceRule {
public:
    QueenRule(CooldownConfig config) : PieceRule(config) {}
    // Returns the legal destinations for a queen.
    std::set<Position> legalDestinations(const Board& board, const Piece& piece) const override;
};

class KnightRule : public PieceRule {
public:
    KnightRule(CooldownConfig config) : PieceRule(config) {}
    // Returns the legal destinations for a knight.
    std::set<Position> legalDestinations(const Board& board, const Piece& piece) const override;
};

class KingRule : public PieceRule {
public:
    KingRule(CooldownConfig config) : PieceRule(config) {}

    // Returns the legal destinations for a king.
    std::set<Position> legalDestinations(const Board& board, const Piece& piece) const override;

    // Indicates whether a king's death is fatal.
    bool isFatalDeath() const override;
};

class PawnRule : public PieceRule {
public:

    PawnRule(CooldownConfig config) : PieceRule(config) {}

    // Returns the legal destinations for a pawn.
    std::set<Position> legalDestinations(const Board& board, const Piece& piece) const override;

    // Handles the pawn arrival behavior.
    void onArrival(const Board& board, Piece& piece) const override;
};