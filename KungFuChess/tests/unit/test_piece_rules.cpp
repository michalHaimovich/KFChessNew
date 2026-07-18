#include "doctest.h"
#include "rules/piece_rules.hpp"
#include "rules/balance_config.hpp" 
#include "model/board.hpp"

TEST_CASE("PieceRules - Specific Behaviors") {
    GameBalance balance; 
    
    SUBCASE("KingRule identifies death as fatal") {
        KingRule kingRule(balance.king);
        CHECK(kingRule.isFatalDeath() == true);
    }
    
    SUBCASE("RookRule identifies death as non-fatal") {
        RookRule rookRule(balance.rook);
        CHECK(rookRule.isFatalDeath() == false); 
    }

    SUBCASE("PawnRule handles promotion upon arrival at the last rank") {
        PawnRule pawnRule(balance.pawn);
        Board board(8, 8);
        
        Piece whitePawn(1, PieceColor::White, PieceKind::Pawn, Position{0, 4});
        
        pawnRule.onArrival(board, whitePawn);
        
        CHECK(whitePawn.kind == PieceKind::Queen);
    }

    SUBCASE("PieceRule base class correctly maps states to cooldowns") {
        RookRule rookRule(balance.rook);
        
        CHECK(rookRule.getCooldownMs(PieceState::Idle) == 0);
        CHECK(rookRule.getCooldownMs(PieceState::Move) == 0);
    }
}