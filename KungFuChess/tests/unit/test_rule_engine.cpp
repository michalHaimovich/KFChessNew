#include "doctest.h"
#include "rules/rule_engine.hpp"
#include "model/board.hpp"

TEST_CASE("RuleEngine - General Validations") {
    RuleEngine engine;

    SUBCASE("isFatalDeath correctly identifies fatal pieces") {
        // Only the King's death should be fatal
        CHECK(engine.isFatalDeath(PieceKind::King) == true);
        
        // Other pieces should not end the game
        CHECK(engine.isFatalDeath(PieceKind::Queen) == false);
        CHECK(engine.isFatalDeath(PieceKind::Pawn) == false);
        CHECK(engine.isFatalDeath(PieceKind::Rook) == false);
    }

    SUBCASE("validateMove correctly validates piece movements") {
        Board board(8, 8);
        Piece rook(1, PieceColor::White, PieceKind::Rook, Position{4, 4});
        board.addPiece(rook);

        // A Rook moving in a straight line should be valid
        CHECK(engine.validateMove(board, rook, Position{4, 7}) == true); // Horizontal
        CHECK(engine.validateMove(board, rook, Position{0, 4}) == true); // Vertical

        // A Rook moving diagonally should be invalid
        CHECK(engine.validateMove(board, rook, Position{5, 5}) == false); 
    }

    SUBCASE("getCooldownMs returns non-negative values for rests") {
        // Verify that the engine correctly delegates and fetches cooldown times
        long shortRest = engine.getCooldownMs(PieceKind::Knight, PieceState::ShortRest);
        long longRest = engine.getCooldownMs(PieceKind::Knight, PieceState::LongRest);
        
        CHECK(shortRest >= 0);
        CHECK(longRest >= 0);
    }
}