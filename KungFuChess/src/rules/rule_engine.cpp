#include "rules/rule_engine.hpp"
#include "rules/balance_config.hpp"

RuleEngine::RuleEngine() {
    GameBalance balance;

    rulesRegistry[PieceKind::Rook]   = std::make_unique<RookRule>(balance.rook);
    rulesRegistry[PieceKind::Bishop] = std::make_unique<BishopRule>(balance.bishop);
    rulesRegistry[PieceKind::Queen]  = std::make_unique<QueenRule>(balance.queen);
    rulesRegistry[PieceKind::Knight] = std::make_unique<KnightRule>(balance.knight);
    rulesRegistry[PieceKind::King]   = std::make_unique<KingRule>(balance.king);
    rulesRegistry[PieceKind::Pawn]   = std::make_unique<PawnRule>(balance.pawn);
}

bool RuleEngine::validateMove(const Board& board, const Piece& piece, Position destination) const {
    // Find the specific rule for this piece kind
    auto it = rulesRegistry.find(piece.kind);
    
    // If rule doesn't exist (e.g., piece not fully implemented), reject move safely
    if (it == rulesRegistry.end()) {
        return false;
    }
    
    // Ask the specific strategy for all its legal destinations
    auto legalDestinations = it->second->legalDestinations(board, piece);
    
    // Check if the requested destination exists in the legal set
    return legalDestinations.count(destination) > 0;
}

void RuleEngine::processArrival(const Board& board, Piece& piece) const {
    auto it = rulesRegistry.find(piece.kind);
    if (it != rulesRegistry.end()) {
        it->second->onArrival(board, piece);
    }
}

bool RuleEngine::isFatalDeath(PieceKind kind) const {
    auto it = rulesRegistry.find(kind);
    if (it != rulesRegistry.end()) {
        return it->second->isFatalDeath();
    }
    return false;
}

long RuleEngine::getCooldownMs(PieceKind kind, PieceState state) const {
    auto it = rulesRegistry.find(kind);
    if (it != rulesRegistry.end()) {
        return it->second->getCooldownMs(state);
    }
    return 1000; // defult jast incase...
}