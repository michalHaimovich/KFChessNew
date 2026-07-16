#include "doctest.h"
#include "rules/rule_engine.hpp"
#include "model/board.hpp"
#include "model/piece.hpp"

TEST_CASE("RuleEngine - Movement Validation Routing") {
    Board board(8, 8);
    RuleEngine engine;

    SUBCASE("Validates legal moves using the correct piece strategy") {
        Piece rook(1, PieceColor::White, PieceKind::Rook, Position{0, 0});
        board.addPiece(rook);

        // Rook moving straight - should be validated as true
        CHECK(engine.validateMove(board, rook, Position{0, 5}) == true);
        CHECK(engine.validateMove(board, rook, Position{7, 0}) == true);
        
        // Rook moving diagonally - should be validated as false
        CHECK(engine.validateMove(board, rook, Position{1, 1}) == false);
    }

    SUBCASE("Rejects moves for unregistered piece kinds gracefully") {
        // We simulate a piece kind that doesn't have a rule registered yet
        // (Assuming Pawn isn't registered in the engine yet based on our previous step)
        Piece pawn(2, PieceColor::Black, PieceKind::Pawn, Position{1, 1});
        board.addPiece(pawn);

        // Should return false rather than crashing
        CHECK(engine.validateMove(board, pawn, Position{1, 2}) == false);
    }
}